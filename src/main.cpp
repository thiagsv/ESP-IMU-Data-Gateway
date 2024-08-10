#include <Arduino.h>
#include "tasks.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;

int runCollect = 0; 
QueueHandle_t imuDataQueue;

void setup() {
  Serial.begin(115200);

  imuDataQueue = xQueueCreate(UINT32_MAX, sizeof(String));

  xTaskCreatePinnedToCore(
    Task1,
    "Task1",
    8192,
    NULL,
    1,  
    &task1Handle,
    0
  );

  xTaskCreatePinnedToCore(
    Task2,
    "Task2",
    16384,
    NULL,
    2,
    &task2Handle,
    1
  );
}

void loop() {

}
