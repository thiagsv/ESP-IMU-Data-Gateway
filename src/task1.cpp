#include "../include/tasks.h"

void Task1(void *pvParameters) {
    bool fileOpen = false;
    bool removeFileData = false;
    File file;

    while (true) {
        if (runCollect) {
            if (!fileOpen) {
                file = SPIFFS.open(fileName, FILE_APPEND);  // Abre o arquivo no modo append
                if (!file) {
                    Serial.println("Falha ao abrir o arquivo no SPIFFS.");
                    vTaskDelay(pdMS_TO_TICKS(500));  // Ajuste conforme a frequência de coleta
                    continue;  // Tenta novamente na próxima iteração
                }
                fileOpen = true;
            }

            IMUData imuData;
            if (xQueueReceive(imuDataQueue, &imuData, 0) == pdPASS) {
                if (fileOpen) {
                    file.printf("%d,%f,%f,%f,%f,%f,%f,%f;\n",
                                imuData.Id, imuData.AcX, imuData.AcY, imuData.AcZ,
                                imuData.GyX, imuData.GyY, imuData.GyZ, imuData.Timestamp);
                    Serial.println("Dados escritos no SPIFFS.");
                    removeFileData = true;
                }
            }
        } else {
            if (fileOpen) {
                file.close();  // Fecha o arquivo quando a coleta for interrompida
                fileOpen = false;
                Serial.println("Arquivo fechado.");
            }

            if (removeFileData) {
                // Esvazia o arquivo
                File emptyFile = SPIFFS.open(fileName, "w");
                if (emptyFile) {
                    emptyFile.close();
                    Serial.println("Arquivo esvaziado.");
                } else {
                    Serial.println("Falha ao abrir o arquivo para esvaziar.");
                }
                removeFileData = false;
            }

            // Descartar dados da fila
            IMUData discardData;
            while (xQueueReceive(imuDataQueue, &discardData, 0) == pdPASS) {
                // Dados descartados
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));  // Ajuste conforme a frequência de coleta
    }
}
