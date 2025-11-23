#include <Arduino.h>
#include "global.h"
#define FAN_PIN 10
#define PUMP_PIN 8
void task_fan(void *pvParameters);
void task_pump(void *pvParameters);

