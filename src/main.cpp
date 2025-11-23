#include "global.h"
#include <Arduino.h>

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"
#include "lcd_display.h"
#include "control.h"
void setup()
{
  Serial.begin(115200);
  Wire.begin(11, 12);

  xQueueSensor = xQueueCreate(5, sizeof(SensorData));
  xSensorMutex = xSemaphoreCreateMutex();

  xBinarySemaphoreInternet = xSemaphoreCreateBinary();
  xSemaphoreLed = xSemaphoreCreateBinary();
  xSemaphoreNeoLed = xSemaphoreCreateBinary();
  xSemaphoreLCD = xSemaphoreCreateBinary();

  // Tạo task LCD trước
  // xTaskCreate(wifi_connect_task, "WiFi Connect", 4096, NULL, 1, NULL);

  // Tạo task sensor sau
  xTaskCreate(temp_humi_monitor, "Sensor", 4096, NULL, 1, NULL);

  // LED task
  // xTaskCreate(led_blinky, "LED", 4096, NULL, 1, NULL);
  // xTaskCreate(neo_blinky, "NeoPixel", 4096, NULL, 1, NULL);
  // xTaskCreate(lcd_display, "LCD", 4096, NULL, 1, NULL);
  // xTaskCreate(task_fan, "Fan", 4096, NULL, 1, NULL);
  // xTaskCreate(task_fan, "Fan", 4096, NULL, 1, NULL);
  // xTaskCreate(task_pump, "Pump", 4096, NULL, 1, NULL);
  xTaskCreate(main_server_task, "MainServer", 8192, NULL, 1, NULL);
  xTaskCreate(coreiot_task, "CoreIOT", 8192, NULL, 1, NULL);
  // xTaskCreate(tiny_ml_task, "TinyML", 8192, NULL, 1, NULL);
}

void loop()
{
}
