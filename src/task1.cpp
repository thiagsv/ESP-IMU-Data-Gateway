#include "../include/tasks.h"

void Task1(void *pvParameters) {
    while (true) {
        if (runCollect) {
            IMUData imuData;
            if (xQueueReceive(imuDataQueue, &imuData, 0) == pdPASS) {
                File file = SD.open(fileName, FILE_APPEND);
                if (file) {
                    file.print(imuData.AcX); file.print(",");
                    file.print(imuData.AcY); file.print(",");
                    file.print(imuData.AcZ); file.print(",");
                    file.print(imuData.GyX); file.print(",");
                    file.print(imuData.GyY); file.print(",");
                    file.print(imuData.GyZ); file.print(",");
                    file.println(imuData.Timestamp); file.print(";");
                    file.close();
                    Serial.println("Dados escritos no SD.");
                } else {
                    Serial.println("Falha ao abrir o arquivo no SD.");
                }
            }
        } else {
            IMUData discardData;
            while (xQueueReceive(imuDataQueue, &discardData, 0) == pdPASS) {
                // Discard data
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
