#include "../include/tasks.h"

void Task1(void *pvParameters) {
    while (true) {
        if (runCollect) {
            String imuData;
            if (xQueueReceive(imuDataQueue, &imuData, 0) == pdPASS) {
                File file = SD.open("/data.txt", FILE_APPEND);
                if (file) {
                    file.println(imuData);
                    file.close();
                    Serial.println("Dados escritos no SD: " + imuData);
                } else {
                    Serial.println("Falha ao abrir o arquivo no SD.");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
