#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern float glob_temperature;
extern float glob_humidity;
typedef struct
{
    float temperature;
    float humidity;
} SensorData;

extern SensorData data;
extern QueueHandle_t xQueueSensor;
extern SemaphoreHandle_t xSemaphoreLed;
extern SemaphoreHandle_t xSemaphoreNeoLed;
extern SemaphoreHandle_t xSemaphoreLCD;
extern SemaphoreHandle_t xSensorMutex;

extern String ssid;
extern String password;
extern String wifi_ssid;
extern String wifi_password;
extern boolean isWifiConnected;

extern SemaphoreHandle_t xBinarySemaphoreInternet;

extern bool pump_state;
extern bool fan_state;
extern bool led1_state;
extern bool led2_state;

#endif