#ifndef __COREIOT_H__
#define __COREIOT_H__

#include <Arduino.h>
#include <WiFi.h>
#include "global.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

void coreiot_task(void *pvParameters);
void wifi_connect_task(void *pvParameters);
void reportStateToCoreIOT(const char *key, bool state);

extern WiFiClient espClient;
extern PubSubClient client;
#endif