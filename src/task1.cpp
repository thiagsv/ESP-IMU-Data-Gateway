#include "include/tasks.h"

void Task1(void *pvParameters) {
    const size_t bufferLimit = 5120;
    String dataBuffer = "";  // Buffer para armazenar os dados

    while (true) {
        if (runCollect) {
            File file = SPIFFS.open(fileName, FILE_APPEND);  // Mantém o arquivo aberto
            if (!file) {
                Serial.println("Falha ao abrir o arquivo no SPIFFS.");
                vTaskDelay(pdMS_TO_TICKS(5));
                continue;  // Sai da task se não conseguir abrir o arquivo
            }

            IMUData imuData;

            // Enquanto houver dados na fila, retire-os e adicione ao buffer
            while (xQueueReceive(imuDataQueue, &imuData, 0) == pdPASS) {
                dataBuffer += String(imuData.Id) + "," + String(imuData.AcX, 6) + "," + 
                              String(imuData.AcY, 6) + "," + String(imuData.AcZ, 6) + "," +
                              String(imuData.GyX, 6) + "," + String(imuData.GyY, 6) + "," +
                              String(imuData.GyZ, 6) + "," + String(imuData.Timestamp, 3) + ";";

                // Se o buffer atingir o limite, grava no arquivo
                if (dataBuffer.length() >= bufferLimit && runCollect) {
                    file.print(dataBuffer);  // Grava os dados do buffer no arquivo
                    dataBuffer = "";  // Limpa o buffer
                }
            }

            // Grava qualquer dado restante no buffer
            if (dataBuffer.length() > 0 && runCollect) {
                file.print(dataBuffer);
                dataBuffer = "";
            }

            file.close();
        }

        vTaskDelay(pdMS_TO_TICKS(1));  // Ajuste conforme a frequência de coleta
    }
}
