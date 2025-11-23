#include "coreiot.h"
#include "control.h"
// ----------- CONFIGURE THESE! -----------
const char *coreIOT_Server = "app.coreiot.io";
const char *coreIOT_Token = "oghoslDWPIEMWW0gg5PT"; // Device Access Token
const int mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    if (client.connect("ESP32Client", coreIOT_Token, NULL))
    {
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("RPC arrived [");
  Serial.print(topic);
  Serial.println("] ");

  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';

  Serial.print("Payload: ");
  Serial.println(msg);

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, msg))
  {
    Serial.println("JSON parse error");
    return;
  }

  String method = doc["method"];
  String requestID = String(topic).substring(String(topic).lastIndexOf('/') + 1);

  // ------------------ GET STATE ------------------
  if (method == "getPumpState")
  {
    StaticJsonDocument<64> resp;
    resp["state"] = pump_state;

    char buf[64];
    serializeJson(resp, buf);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), buf);
    Serial.println("Responded getState()");
    return;
  }
  if (method == "getFanState")
  {
    StaticJsonDocument<64> resp;
    resp["state"] = fan_state;

    char buf[64];
    serializeJson(resp, buf);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), buf);
    Serial.println("Responded getState()");
    return;
  }

  // ------------------ SET STATE -------------------
  if (method == "setPumpState")
  {
    bool new_state = doc["params"];

    pump_state = new_state;

    digitalWrite(PUMP_PIN, pump_state);

    reportStateToCoreIOT("pump_state", pump_state);
    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Pump → %s\n", pump_state ? "ON" : "OFF");
    return;
  }
  if (method == "setFanState")
  {
    fan_state = doc["params"];
    digitalWrite(FAN_PIN, fan_state);
    reportStateToCoreIOT("fan_state", fan_state);
    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Fan → %s\n", fan_state ? "ON" : "OFF");
    return;
  }
}
void reportStateToCoreIOT(const char *key, bool state)
{
  if (!client.connected())
    return;

  StaticJsonDocument<64> doc;
  doc[key] = state; // Ví dụ: {"pump_state": true}

  char buffer[64];
  serializeJson(doc, buffer);
  client.publish("v1/devices/me/telemetry", buffer);
  Serial.printf("[CoreIOT] Telemetry: %s -> %s\n", key, state ? "ON" : "OFF");
}
void setup_coreiot()
{

  // Serial.print("Connecting to WiFi...");
  // WiFi.begin(wifi_ssid, wifi_password);
  // while (WiFi.status() != WL_CONNECTED) {

  // while (isWifiConnected == false) {
  //   delay(500);
  //   Serial.print(".");
  // }
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  WiFi.begin(wifi_ssid, wifi_password);

  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(callback);
}
void wifi_connect_task(void *pvParameters)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  // Signal các task khác WiFi ready
  xSemaphoreGive(xBinarySemaphoreInternet);

  // Task xong → tự xóa
  vTaskDelete(NULL);
}

void coreiot_task(void *pvParameters)
{
  // Chờ WiFi
  // xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY);
  // client.setServer(coreIOT_Server, mqttPort);
  // client.setCallback(callback);
  // String clientId = "ESP32Client-" + String(random(0xffff), HEX);
  // if (!client.connect(clientId.c_str(), coreIOT_Token, NULL))
  // {
  //   Serial.println("MQTT connect failed");
  //   vTaskDelay(pdMS_TO_TICKS(5000));
  // }
  Serial.println("CoreIOT: Waiting for Internet connection...");
  xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY); // Task bị chặn ở đây

  // Khi Semaphore được Give (tức là WiFi.status() == WL_CONNECTED):
  Serial.println("CoreIOT: Internet connected! Starting cloud loop...");

  // Đặt Semaphore lại
  xSemaphoreTake(xBinarySemaphoreInternet, 0);
  SensorData recv;
  StaticJsonDocument<128> doc;
  char buffer[128];
  setup_coreiot();
  while (1)
  {
    if (!client.connected())
      reconnect();

    client.loop();

    if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE)
    {
      recv = data;
      xSemaphoreGive(xSensorMutex);
      doc.clear();
      doc["temperature"] = recv.temperature;
      doc["humidity"] = recv.humidity;
      Serial.printf("[CoreIOT] T=%.1f H=%.1f \n", recv.temperature, recv.humidity);
      serializeJson(doc, buffer);

      if (client.publish("v1/devices/me/telemetry", buffer))
        Serial.println("Published: " + String(buffer));
      else
        Serial.println("Failed to publish");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  // if (!client.connected())
  //   reconnect();

  // client.loop();
}
