#include "temp_humi_monitor.h"
#include <Arduino.h>

DHT20 dht20;
// LiquidCrystal_I2C lcd(33, 16, 2);
// LiquidCrystal_I2C lcd(0x21, 16, 2);

void temp_humi_monitor(void *pvParameters)
{
    // Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();
    while (1)
    {
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity = -1;
            // return;
        }

        if (xSemaphoreTake(xSensorMutex, portMAX_DELAY))
        {
            data.temperature = temperature;
            data.humidity = humidity;
            xSemaphoreGive(xSensorMutex);
        }

        // xQueueSend(xQueueSensor, &data, portMAX_DELAY);
        // xSemaphoreGive(xSemaphoreLCD);
        // xSemaphoreGive(xSemaphoreLed);
        // xSemaphoreGive(xSemaphoreNeoLed);

        Serial.printf("[Sensor] Sent to Queue: T=%.1f H=%.1f\n", data.temperature, data.humidity);

        vTaskDelay(1000);
    }
}