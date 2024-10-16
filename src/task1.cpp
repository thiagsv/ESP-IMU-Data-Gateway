#include "include/tasks.h"

void Task1(void *pvParameters) {
    while (true) {
        if (runCollect) {
            File file = SPIFFS.open(fileName, FILE_APPEND);  // Abre o arquivo no modo append
            if (!file) {
                Serial.println("Falha ao abrir o arquivo no SPIFFS.");
                vTaskDelay(pdMS_TO_TICKS(20));  // Ajuste conforme a frequência de coleta
                continue;  // Tenta novamente na próxima iteração
            }

            IMUData imuData;
            if (xQueueReceive(imuDataQueue, &imuData, 0) == pdPASS) {
                // Grava os dados no arquivo
                // Serial.print("Id: "); Serial.print(imuData.Id);
                // Serial.print(", AcX: "); Serial.print(imuData.AcX, 6);
                // Serial.print(", AcY: "); Serial.print(imuData.AcY, 6);
                // Serial.print(", AcZ: "); Serial.print(imuData.AcZ, 6);
                // Serial.print(", GyX: "); Serial.print(imuData.GyX, 6);
                // Serial.print(", GyY: "); Serial.print(imuData.GyY, 6);
                // Serial.print(", GyZ: "); Serial.println(imuData.GyZ, 6);
                file.printf("%d,%f,%f,%f,%f,%f,%f,%f;\n",
                            imuData.Id, imuData.AcX, imuData.AcY, imuData.AcZ,
                            imuData.GyX, imuData.GyY, imuData.GyZ, imuData.Timestamp);
                Serial.println("Dados escritos no SPIFFS.");
            }

            // Fecha o arquivo após cada escrita
            file.close();
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // Ajuste conforme a frequência de coleta
    }
}
