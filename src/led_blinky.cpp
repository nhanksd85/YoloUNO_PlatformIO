#include <Arduino.h>
#include "global.h"
#include "led_blinky.h"

void led_blinky(void *pvParameters)
{
  pinMode(LED_GPIO, OUTPUT);
  SensorData recv;
  while (1)
  {
    if (xSemaphoreTake(xSensorMutex, portMAX_DELAY))
    {
      recv = data;
      xSemaphoreGive(xSensorMutex);
      int delayTime;

      if (recv.temperature < 25)
        delayTime = 1000;
      else if (recv.temperature < 35)
        delayTime = 500;
      else
        delayTime = 200;

      digitalWrite(LED_GPIO, HIGH);
      vTaskDelay(pdMS_TO_TICKS(delayTime));
      digitalWrite(LED_GPIO, LOW);
      vTaskDelay(pdMS_TO_TICKS(delayTime));

      Serial.printf("[LED] Blinked @T=%.1f°C\n", recv.temperature);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
void setFanSpeed(int percent)
{
  // 0-100%
  percent = constrain(percent, 0, 100);
  int duty = map(percent, 0, 100, 0, 1023);
  ledcWrite(0, duty);
}
