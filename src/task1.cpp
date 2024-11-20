#include "include/tasks.h"

void Task1(void *pvParameters) {
    const int bufferSize = 4096;         // Tamanho do buffer fixo em bytes
    char dataBuffer[bufferSize];         // Buffer de caracteres para acumular dados
    int bufferIndex = 0;                 // Índice atual no buffer
    unsigned long lastWriteTime = millis();

    while (true) {  // Loop infinito para manter a tarefa ativa
        if (runCollect) {
            File file = LittleFS.open(fileName, FILE_APPEND);  // Abre o arquivo ao iniciar a coleta
            if (!file) {
                Serial.println("Erro: Falha ao abrir o arquivo no LittleFS.");
                vTaskDelay(pdMS_TO_TICKS(100));  // Delay antes de tentar novamente
                continue;  // Tenta novamente na próxima iteração do loop
            }

            IMUData imuData;

            // Enquanto houver dados na fila e a coleta estiver ativa
            while (runCollect) {
                // Verifica se há dados disponíveis na fila
                if (xQueueReceive(imuDataQueue, &imuData, pdMS_TO_TICKS(5)) == pdPASS) {
                    // Formata os dados para texto e adiciona ao buffer
                    int bytesWritten = snprintf(&dataBuffer[bufferIndex], bufferSize - bufferIndex,
                                                "%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.3f;",
                                                imuData.Id, imuData.AcX, imuData.AcY, imuData.AcZ,
                                                imuData.GyX, imuData.GyY, imuData.GyZ, imuData.Timestamp);

                    bufferIndex += bytesWritten;

                    // Se o buffer estiver cheio, grava no arquivo e limpa o buffer
                    if (bufferIndex >= bufferSize - 100) {  // Deixe algum espaço de sobra para segurança
                        file.write((uint8_t *)dataBuffer, bufferIndex);  // Grava o conteúdo do buffer no arquivo
                        bufferIndex = 0;  // Reseta o índice do buffer
                        // Serial.println("Dados gravados no arquivo.");
                    }
                }

                // Grava periodicamente se o intervalo for atingido, mesmo que o buffer não esteja cheio
                if (millis() - lastWriteTime >= 1500) {
                    if (bufferIndex > 0) {
                        file.write((uint8_t *)dataBuffer, bufferIndex);
                        bufferIndex = 0;
                        // Serial.println("Dados gravados no arquivo no intervalo.");
                    }
                    lastWriteTime = millis();  // Atualiza o tempo do último fechamento
                }

                vTaskDelay(pdMS_TO_TICKS(1));
            }

            // Grava qualquer dado restante no buffer ao finalizar a coleta
            if (bufferIndex > 0) {
                file.write((uint8_t *)dataBuffer, bufferIndex);
                // Serial.println("Dados finais gravados no arquivo.");
                bufferIndex = 0;
            }

            file.close();
            // Serial.println("Coleta interrompida, arquivo fechado.");
        }

        // Delay para evitar que a tarefa consuma CPU quando runCollect estiver desativado
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
