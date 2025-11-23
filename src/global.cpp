#include "global.h"
float glob_temperature = 30;
float glob_humidity = 70;

String ssid = "ESP32-MH";
String password = "12345678";
String wifi_ssid = "ACLAB";
String wifi_password = "ACLAB2023";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet;

SensorData data;
SemaphoreHandle_t xSensorMutex;

QueueHandle_t xQueueSensor;
SemaphoreHandle_t xSemaphoreLed;
SemaphoreHandle_t xSemaphoreNeoLed;
SemaphoreHandle_t xSemaphoreLCD;

bool led1_state = false;
bool led2_state = false;
bool pump_state = false;
bool fan_state = false;