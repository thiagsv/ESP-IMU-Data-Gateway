#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <SD.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>

extern int runCollect;
extern QueueHandle_t imuDataQueue;
extern const uint8_t AD0_MPU[];
extern unsigned long initialTime;

void Task1(void *pvParameters);
void Task2(void *pvParameters);

#endif
