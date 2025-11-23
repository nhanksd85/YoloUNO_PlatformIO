#include "neo_blinky.h"

void neo_blinky(void *pvParameters)
{
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();
    SensorData recv;
    while (1)
    {
        if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE)
        {
            recv = data;
            xSemaphoreGive(xSensorMutex);
            if (recv.humidity < 40)
                strip.setPixelColor(0, strip.Color(255, 0, 0));
            else if (recv.humidity < 70)
                strip.setPixelColor(0, strip.Color(0, 255, 0));
            else
                strip.setPixelColor(0, strip.Color(0, 0, 255));

            strip.show();
            Serial.printf("[NeoPixel] Color updated @H=%.1f%%\n", recv.humidity);
        }
        vTaskDelay(1000);
    }
}
