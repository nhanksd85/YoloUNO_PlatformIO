#include "control.h"

void task_fan(void *pvParameters)
{
    pinMode(FAN_PIN, OUTPUT);
    bool fanOn = false;
    SensorData s;

    while (1)
    {
        // ---- Cách đúng để đọc data ----
        if (xSemaphoreTake(xSensorMutex, portMAX_DELAY))
        {
            s = data;
            xSemaphoreGive(xSensorMutex);
        }

        // ---- Điều khiển quạt ----
        if (s.temperature > 29 && !fanOn)
        {
            digitalWrite(FAN_PIN, HIGH);
            fanOn = true;
            Serial.println("[FAN] Turned ON");
        }
        else if (s.temperature <= 27 && fanOn)
        {
            digitalWrite(FAN_PIN, LOW);
            fanOn = false;
            Serial.println("[FAN] Turned OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_pump(void *pvParameters)
{
    pinMode(PUMP_PIN, OUTPUT);
    bool pumpOn = false;
    SensorData s;

    while (1)
    {
        // ---- Cách đúng để đọc data ----
        if (xSemaphoreTake(xSensorMutex, portMAX_DELAY))
        {
            s = data;
            xSemaphoreGive(xSensorMutex);
        }

        // ---- Điều khiển bơm ----
        if (s.humidity < 40 && !pumpOn)
        {
            digitalWrite(PUMP_PIN, HIGH);
            pumpOn = true;
            Serial.println("[PUMP] Turned ON");
        }
        else if (s.humidity >= 45 && pumpOn)
        {
            digitalWrite(PUMP_PIN, LOW);
            pumpOn = false;
            Serial.println("[PUMP] Turned OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}