#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <esp_log.h>
#include <OTA_Firmware_Update.h>
#include <ThingsBoard.h>
#include "PubSubClient.h"
#include <Espressif_Updater.h>
#include <Server_Side_RPC.h>

#include "DHT20.h"

#define PUMP_PIN 0 // Replace 0 with the actual pin number for D3
#define LED_PIN 6
DHT20 dht20;

constexpr char CURRENT_FIRMWARE_TITLE[] = "BLINKY"; // Title firmware
constexpr char CURRENT_FIRMWARE_VERSION[] = "1.1"; // Version firmware

// Maximum amount of retries we attempt to download each firmware chunck over MQTT
constexpr uint8_t FIRMWARE_FAILURE_RETRIES = 12U;

// Size of each firmware chunck downloaded over MQTT,
// increased packet size, might increase download speed
constexpr uint16_t FIRMWARE_PACKET_SIZE = 4096U;

constexpr char WIFI_SSID[] = "LEDAT WIFI";
constexpr char WIFI_PASSWORD[] = "Dat@#)(99";

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
constexpr char TOKEN[] = "blabla"; // Your deivce Access Token
// Thingsboard we want to establish a connection too
constexpr char THINGSBOARD_SERVER[] = "app.coreiot.io";

constexpr char TEMPERATURE_KEY[] = "temperature";
constexpr char HUMIDITY_KEY[] = "humidity";

constexpr const char FW_TAG_KEY[] = "fw_tag";

constexpr uint16_t THINGSBOARD_PORT = 1883U;

constexpr uint16_t MAX_MESSAGE_SEND_SIZE = 512U;
constexpr uint16_t MAX_MESSAGE_RECEIVE_SIZE = 512U;

constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;
constexpr int16_t TELEMETRY_SEND_INTERVAL = 5000U;
constexpr int16_t BLINKY_SEND_INTERVAL = 1000U;
constexpr size_t MAX_ATTRIBUTES = 6U;

uint32_t previousTelemetrySend;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);

OTA_Firmware_Update<> ota;
Server_Side_RPC<3U, 5U> rpc;
Shared_Attribute_Update<1U, MAX_ATTRIBUTES> shared;
const std::array<IAPI_Implementation *, 3U> apis = {&ota, &rpc, &shared};

Shared_Attribute_Update<1U, MAX_ATTRIBUTES> shared_update;

ThingsBoard tb(mqttClient, MAX_MESSAGE_RECEIVE_SIZE, MAX_MESSAGE_SEND_SIZE, Default_Max_Stack_Size, apis);

Espressif_Updater<> updater;

// Current led state, on or off
volatile bool ledState = false;
volatile bool pumpState = false;
// Handle led state changes
volatile bool ledStateChanged = false;
volatile bool pumpStateChanged = false;
constexpr const char LED_STATE_ATTR[] = "led_state";
constexpr const char PUMP_STATE_ATTR[] = "pump_state";

// Statuses for updating
bool currentFWSent = false;
bool updateRequestSent = false;

bool subscribed = false;

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

bool reconnect()
{
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED)
  {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void processSetLedState(const JsonVariantConst &data, JsonDocument &response)
{
  // Process data (Comment and Uncomment to test OTA)

  ledState = data;
  Serial.print("Received set led state RPC. New state: ");
  Serial.println(ledState);

  StaticJsonDocument<1> response_doc;
  // Returning current state as response
  response_doc["newState"] = (int)ledState;
  response.set(response_doc);

  ledStateChanged = true;
}



void processSetPumpState(const JsonVariantConst &data, JsonDocument &response)
{
  // Process data
  pumpState = data;
  Serial.print("Received set pump state RPC. New state: ");
  Serial.println(ledState);

  StaticJsonDocument<1> response_doc;
  // Returning current state as response
  response_doc["newState"] = (int)pumpState;
  response.set(response_doc);

  pumpStateChanged = true;
}

// Server-side RPC callback
const std::array<RPC_Callback, 2U> rpcCallbacks = {
    RPC_Callback{"setStateLED", processSetLedState},
    RPC_Callback{"setStatePUMP", processSetPumpState}};

void update_starting_callback()
{
  // Nothing to do
}

void finished_callback(const bool &success)
{
  if (success)
  {
    Serial.println("Done, Reboot now");
    esp_restart();
    return;
  }
  Serial.println("Downloading firmware failed");
}

void progress_callback(const size_t &current, const size_t &total)
{
  Serial.printf("Progress %.2f%%\n", static_cast<float>(current * 100U) / total);
}

void setup()
{
  // Initalize serial connection for debugging
  Wire.begin(GPIO_NUM_11, GPIO_NUM_12);
  dht20.begin();
  Serial.begin(SERIAL_DEBUG_BAUD);
  pinMode(LED_PIN,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  InitWiFi();
}

void loop()
{
  delay(1000);

  if (!reconnect())
  {
    return;
  }

  if (!tb.connected())
  {
    Serial.printf("Connecting to: (%s) with token (%s)\n", THINGSBOARD_SERVER, TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT, "  ", nullptr))
    {
      Serial.println("Failed to connect");
      return;
    }

    // Shared attribute conflig with OTA don't uncomment
    // if (!subscribed)
    // {
    //   // Shared attributes we want to request from the server
    //   constexpr std::array<const char *, MAX_ATTRIBUTES> SUBSCRIBED_SHARED_ATTRIBUTES = {FW_CHKS_KEY, FW_CHKS_ALGO_KEY, FW_SIZE_KEY, FW_TAG_KEY, FW_TITLE_KEY, FW_VER_KEY};
    //   const Shared_Attribute_Callback<MAX_ATTRIBUTES> callback(&processSharedAttributeUpdate, SUBSCRIBED_SHARED_ATTRIBUTES);
    //   subscribed = shared_update.Shared_Attributes_Subscribe(callback);
    //   Serial.print("Subscribed for shared attributes: ");
    //   Serial.println(subscribed);
    // }

    Serial.println("Subscribing for RPC...");

    if (!rpc.RPC_Subscribe(rpcCallbacks.cbegin(), rpcCallbacks.cend()))
    {
      Serial.println("Failed to subscribe for RPC");
      return;
    }
  }

  // Sending telemetry by time interval
  if (millis() - previousTelemetrySend > TELEMETRY_SEND_INTERVAL)
  {

    // Use virtual random sensor
    float temperature = 0;
    float humidity = 0;

    // Uncomment if using DHT20

    dht20.read();
    temperature = dht20.getTemperature();
    humidity = dht20.getHumidity();

    // Uncomment if using DHT11/22
    /*
    float temperature = 0;
    float humidity = 0;
    dht.read2(&temperature, &humidity, NULL);
    */

    Serial.println("Sending telemetry. Temperature: " + String(temperature, 1) + " humidity: " + String(humidity, 1));

    tb.sendTelemetryData(TEMPERATURE_KEY, temperature);
    tb.sendTelemetryData(HUMIDITY_KEY, humidity);
    tb.sendAttributeData("rssi", WiFi.RSSI()); // also update wifi signal strength
    previousTelemetrySend = millis();
  }

  if (ledStateChanged)
  {
    ledStateChanged = false;
    digitalWrite(LED_PIN, ledState);
    Serial.print("LED state is set to: ");
    Serial.println(ledState);
    tb.sendAttributeData(LED_STATE_ATTR, ledState);
  }

  if (pumpStateChanged)
  {
    pumpStateChanged = false;

    Serial.print("PUMP state is set to: ");
    Serial.println(pumpState);

    // TODO
    digitalWrite(PUMP_PIN, pumpState);
    tb.sendAttributeData(PUMP_STATE_ATTR, pumpState);
  }

  if (!currentFWSent)
  {
    currentFWSent = ota.Firmware_Send_Info(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION);
  }

  if (!updateRequestSent)
  {
    Serial.println("Firwmare Update...");
    const OTA_Update_Callback callback(CURRENT_FIRMWARE_TITLE,CURRENT_FIRMWARE_VERSION, 
    &updater, &finished_callback, &progress_callback, &update_starting_callback, FIRMWARE_FAILURE_RETRIES, FIRMWARE_PACKET_SIZE);

    updateRequestSent = ota.Subscribe_Firmware_Update(callback);
  }

  tb.loop();
}
