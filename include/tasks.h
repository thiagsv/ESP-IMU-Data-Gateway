#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <SD.h>
#include <WiFi.h>
#include <Wire.h>

#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>

#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>

extern int runCollect;
extern QueueHandle_t imuDataQueue;
extern const uint8_t AD0_MPU[];
extern unsigned long initialTime;
extern String fileName;

void Task1(void *pvParameters);
void Task2(void *pvParameters);

#endif
