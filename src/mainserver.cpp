#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>
#include "led_blinky.h"
#include "neo_blinky.h"
#include "control.h"
#include "coreiot.h"

bool isAPMode = true;
Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

WebServer server(80);

unsigned long connect_start_ms = 0;
bool connecting = false;
void handleState()
{
  StaticJsonDocument<128> doc;
  doc["led1"] = led1_state ? "ON" : "OFF";
  doc["led2"] = led2_state ? "ON" : "OFF";
  doc["fan"] = fan_state ? "ON" : "OFF";
  doc["pump"] = pump_state ? "ON" : "OFF";

  char buffer[128];
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

void handleSet()
{
  String ledStr = server.arg("led");
  String state = server.arg("state");
  state.toLowerCase();

  bool value = (state == "on");

  if (ledStr == "1")
  {
    led1_state = value;
    digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);
    Serial.print("LED1 -> ");
    Serial.println(led1_state ? "ON" : "OFF");
  }
  else if (ledStr == "2")
  {
    led2_state = value;
    digitalWrite(LED2_PIN, led2_state ? HIGH : LOW);
    Serial.print("LED2 -> ");
    Serial.println(led2_state ? "ON" : "OFF");
  }
  else if (ledStr == "pump")
  {
    pump_state = value;
    digitalWrite(PUMP_PIN, pump_state ? HIGH : LOW);
    Serial.print("PUMP -> ");
    Serial.println(pump_state ? "ON" : "OFF");
  }
  else if (ledStr == "fan")
  {
    fan_state = value;
    digitalWrite(FAN_PIN, fan_state ? HIGH : LOW);
    Serial.print("FAN -> ");
    Serial.println(fan_state ? "ON" : "OFF");
  }

  // --- 4. Trả về JSON (cập nhật tất cả trạng thái) ---
  String json = "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") +
                "\",\"pump\":\"" + String(pump_state ? "ON" : "OFF") +
                "\",\"fan\":\"" + String(fan_state ? "ON" : "OFF") + "\"}";
  server.send(200, "application/json", json);
}
void handleSetAll()
{
  String state = server.arg("state"); // "on" hoặc "off"
  state.toLowerCase();
  bool value = (state == "on");

  led1_state = value;
  led2_state = value;
  fan_state = value;  // Thêm Fan
  pump_state = value; // Thêm Pump

  // Điều khiển các chân pin thực tế
  digitalWrite(LED1_PIN, value ? HIGH : LOW);
  digitalWrite(LED2_PIN, value ? HIGH : LOW);
  digitalWrite(FAN_PIN, value ? HIGH : LOW);
  digitalWrite(PUMP_PIN, value ? HIGH : LOW);
  // ---------------------------------
  if (!value)
  {
    strip.clear();
    strip.show();
  }
  Serial.print("ALL DEVICES -> ");
  Serial.println(value ? "ON" : "OFF");

  // Trả về JSON chứa status của tất cả thiết bị
  String json = "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") +
                "\",\"pump\":\"" + String(pump_state ? "ON" : "OFF") +
                "\",\"fan\":\"" + String(fan_state ? "ON" : "OFF") + "\"}";
  server.send(200, "application/json", json);
}
uint32_t hexToUint32(String hex)
{
  if (hex.startsWith("#"))
  {
    hex.remove(0, 1); // Loại bỏ '#'
  }
  long number = (long)strtol(hex.c_str(), NULL, 16);

  uint8_t r = number >> 16;
  uint8_t g = (number >> 8) & 0xFF;
  uint8_t b = number & 0xFF;

  return strip.Color(r, g, b); // Trả về màu theo định dạng NeoPixel (GRB)
}

void handleNeopixel()
{
  String hex = server.arg("hex"); // dạng "#RRGGBB"
  Serial.print("NEOPIXEL color: ");
  Serial.println(hex);

  if (hex.length() == 7 && hex.startsWith("#"))
  {
    uint32_t color = hexToUint32(hex);

    // Đặt màu cho LED đầu tiên (LED 0)
    strip.setPixelColor(0, color);
    strip.show(); // Hiển thị màu

    // Nếu bạn có một semaphore để báo hiệu cho task neo_blinky dừng lại, hãy sử dụng nó ở đây
    // Ví dụ: xSemaphoreTake(xSemaphoreNeoLed, 0);
  }
  // ======================================

  server.send(200, "text/plain", "OK");
}
void handleToggle()
{
  int led = server.arg("led").toInt();
  if (led == 1)
  {
    led1_state = !led1_state;
    Serial.println("YOUR CODE TO CONTROL LED1");
  }
  else if (led == 2)
  {
    led2_state = !led2_state;
    Serial.println("YOUR CODE TO CONTROL LED2");
  }
  server.send(200, "application/json",
              "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                  "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}");
}

void handleSensors()
{
  float t = data.temperature;
  float h = data.humidity;
  String json = "{\"temp\":" + String(t) + ",\"hum\":" + String(h) + "}";
  server.send(200, "application/json", json);
}

void handleWifiStatus()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    server.send(200, "text/plain", "connected");
  }
  else if (connecting)
  {
    server.send(200, "text/plain", "connecting");
  }
  else
  {
    server.send(200, "text/plain", "failed");
  }
}

void handleConnect()
{
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");
  server.send(200, "text/plain", "Connecting....");
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  connectToWiFi();
}

String getContentType(const String &filename)
{
  if (filename.endsWith(".html"))
    return "text/html; charset=utf-8";
  if (filename.endsWith(".css"))
    return "text/css";
  if (filename.endsWith(".js"))
    return "application/javascript";
  if (filename.endsWith(".json"))
    return "application/json";
  if (filename.endsWith(".png"))
    return "image/png";
  if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
    return "image/jpeg";
  if (filename.endsWith(".ico"))
    return "image/x-icon";
  return "text/plain";
}

bool serveFile(const String &path)
{
  if (!SPIFFS.exists(path))
  {
    Serial.print("File not found: ");
    Serial.println(path);
    return false;
  }
  File file = SPIFFS.open(path, "r");
  String contentType = getContentType(path);
  server.streamFile(file, contentType);
  file.close();
  return true;
}

void handleRootPage()
{ // GET "/"
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!serveFile("/index.html"))
    {
      server.send(404, "text/plain", "index.html not found");
    }
  }
  else
  {
    if (!serveFile("/wifi.html"))
    {
      server.send(404, "text/plain", "wifi.html not found");
    }
  }
}

void setupServer()
{
  // ==== Trang chính & các subpage ====
  server.on("/", HTTP_GET, handleRootPage);

  server.on("/dashboard.html", HTTP_GET, []()
            { serveFile("/dashboard.html"); });
  server.on("/control.html", HTTP_GET, []()
            { serveFile("/control.html"); });
  server.on("/cloud.html", HTTP_GET, []()
            { serveFile("/cloud.html"); });
  server.on("/system_log.html", HTTP_GET, []()
            { serveFile("/system_log.html"); });
  server.on("/wifi.html", HTTP_GET, []()
            { serveFile("/wifi.html"); });
  server.on("/login.html", HTTP_GET, []()
            { serveFile("/login.html"); });

  // ==== Static assets: CSS / JS / JSON ====
  server.on("/style.css", HTTP_GET, []()
            { serveFile("/style.css"); });
  server.on("/script.js", HTTP_GET, []()
            { serveFile("/script.js"); });
  server.on("/humidity.json", HTTP_GET, []()
            { serveFile("/humidity.json"); });
  server.on("/Thermometer.json", HTTP_GET, []()
            { serveFile("/Thermometer.json"); });

  // ==== API động bạn đã có sẵn ====
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/set_all", HTTP_GET, handleSetAll);
  server.on("/neopixel", HTTP_GET, handleNeopixel);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/wifi_status", HTTP_GET, handleWifiStatus);
  server.on("/connect", HTTP_GET, handleConnect);
  server.on("/state", HTTP_GET, handleState);

  // fallback: nếu path nào chưa khai báo mà trùng tên file trong SPIFFS thì vẫn serve được
  server.onNotFound([]()
                    {
      String uri = server.uri();
      if (!uri.startsWith("/")) uri = "/" + uri;
      if (!serveFile(uri)) {
        server.send(404, "text/plain", "404: Not found");
      } });

  server.begin();
}

// ========== WiFi ==========
void startAP()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi()
{
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.print("Connecting to: ");
  Serial.print(wifi_ssid.c_str());

  Serial.print(" Password: ");
  Serial.print(wifi_password.c_str());
}

// ========== Main task ==========
void main_server_task(void *pvParameters)
{
  pinMode(BOOT_PIN, INPUT_PULLUP);

  if (!SPIFFS.begin(true))
  { // true = format nếu mount fail (tùy bạn)
    Serial.println("SPIFFS mount failed");
  }
  else
  {
    Serial.println("SPIFFS mounted OK");
  }
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  strip.begin(); // Khởi tạo NeoPixel
  strip.show();
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);
  startAP();
  setupServer();

  while (1)
  {
    server.handleClient();
    // BOOT Button to switch to AP Mode
    if (digitalRead(BOOT_PIN) == LOW)
    {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW)
      {
        if (!isAPMode)
        {
          startAP();
          setupServer();
        }
      }
    }

    // STA Mode
    if (connecting)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true; // Internet access

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
        connecting = false;
        setupServer();
      }
      else if (millis() - connect_start_ms > 10000)
      { // timeout 10s
        Serial.println("WiFi connect failed! Back to AP.");
        startAP();
        setupServer();
        connecting = false;
        isWifiConnected = false;
      }
    }

    vTaskDelay(1); // avoid watchdog reset
  }
}