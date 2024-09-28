#include "../include/tasks.h"

void Task1(void *pvParameters) {
    while (true) {
        if (runCollect) {
            IMUData imuData;
            if (xQueueReceive(imuDataQueue, &imuData, 0) == pdPASS) {
                File file = SPIFFS.open("/" + fileName, FILE_APPEND);
                if (file) {
                    file.print(imuData.AcX); file.print(",");
                    file.print(imuData.AcY); file.print(",");
                    file.print(imuData.AcZ); file.print(",");
                    file.print(imuData.GyX); file.print(",");
                    file.print(imuData.GyY); file.print(",");
                    file.print(imuData.GyZ); file.print(",");
                    file.print(imuData.Timestamp); file.print(";\n");
                    file.close();
                    Serial.println("Dados escritos no SPIFFS.");
                } else {
                    Serial.println("Falha ao abrir o arquivo no SPIFFS.");
                }
            }
        } else {
            // Esvazia o arquivo
            File file = SPIFFS.open("/" + fileName, "w");
            if (file) {
                file.close(); // Fecha imediatamente após abrir para garantir que está vazio
                Serial.println("Arquivo esvaziado.");
            } else {
                Serial.println("Falha ao abrir o arquivo para esvaziar.");
            }

            IMUData discardData;
            while (xQueueReceive(imuDataQueue, &discardData, 0) == pdPASS) {
                // Descartar dados
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
