#include "../include/tasks.h"

const int MPU_ADDR = 0x69;
const uint8_t n = 5;
const uint8_t AD0_MPU[] = {17, 16, 4, 12, 15};
unsigned long initialTime = 0;

void setupMPU() {
    // Reset do MPU para garantir que ele inicie corretamente
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);  // Registrador de gerenciamento de energia
    Wire.write(0x80);  // Comando de reset
    Wire.endTransmission(true);

    // vTaskDelay(pdMS_TO_TICKS(100));  // Aguarde o reset completar

    // Sair do modo sleep e usar o clock PLL
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);

    // Adicione um pequeno delay para permitir que o MPU estabilize
    // vTaskDelay(pdMS_TO_TICKS(100));

    // Verifica se o sensor acordou corretamente
    // Wire.beginTransmission(MPU_ADDR);
    // Wire.write(0x6B);  // Registrador de gerenciamento de energia
    // Wire.endTransmission(false);
    // Wire.requestFrom(MPU_ADDR, 1, true);  // Solicita 1 byte de dado
    // byte powerManagement = Wire.read();
    // Serial.print("Power Management Register: ");
    // Serial.println(powerManagement, HEX);  // Deve retornar 0x00

    // --- Taxa de amostragem no máximo ---
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x19);
    Wire.write(0x00);
    Wire.endTransmission(false);

    // --- Configura a sensibilidade do acelerômetro ---
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);  // Define a faixa de ±2g para o acelerômetro
    Wire.endTransmission(false);

    // --- Configura a sensibilidade do giroscópio ---
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B);
    Wire.write(0x08);  // Define a faixa de ±500°/s para o giroscópio
    Wire.endTransmission(false);

    // --- Configura o filtro passa-baixa ---
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1A);
    Wire.write(0x05);
    Wire.endTransmission();
}

void getIMUData(uint8_t mpu) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);  // Endereço do registrador do acelerômetro

    int transmissionStatus = Wire.endTransmission(false);  // Envia comando sem finalizar I2C

    if (transmissionStatus != 0) {
        Serial.print("Erro na transmissão com MPU ");
        Serial.print(mpu);
        Serial.print(". Código de erro: ");
        Serial.println(transmissionStatus);
        return;
    }

    if (Wire.requestFrom(MPU_ADDR, 14, true) != 14) {
        Serial.print("Erro: não foi possível ler os dados do MPU: ");
        Serial.println(AD0_MPU[mpu]);
        return;
    }

    int16_t AcX = Wire.read() << 8 | Wire.read();
    int16_t AcY = Wire.read() << 8 | Wire.read();
    int16_t AcZ = Wire.read() << 8 | Wire.read();
    int16_t Tmp = Wire.read() << 8 | Wire.read();
    int16_t GyX = Wire.read() << 8 | Wire.read();
    int16_t GyY = Wire.read() << 8 | Wire.read();
    int16_t GyZ = Wire.read() << 8 | Wire.read();

    IMUData imuData;
    imuData.Id = mpu;
    imuData.AcX = double(AcX) / 16384;
    imuData.AcY = double(AcY) / 16384;
    imuData.AcZ = double(AcZ) / 16384;
    imuData.GyX = double(GyX) / 65.5;
    imuData.GyY = double(GyY) / 65.5;
    imuData.GyZ = double(GyZ) / 65.5;
    imuData.Timestamp = double(millis() - initialTime) / 1000;

    // Serial.print("Id: "); Serial.print(mpu);
    // Serial.print(", AcX: "); Serial.print(imuData.AcX, 6);
    // Serial.print(", AcY: "); Serial.print(imuData.AcY, 6);
    // Serial.print(", AcZ: "); Serial.print(imuData.AcZ, 6);
    // Serial.print(", GyX: "); Serial.print(imuData.GyX, 6);
    // Serial.print(", GyY: "); Serial.print(imuData.GyY, 6);
    // Serial.print(", GyZ: "); Serial.println(imuData.GyZ, 6);
    if (xQueueSend(imuDataQueue, &imuData, 0) != pdPASS) {
        Serial.println("Falha ao enviar dados para a fila.");
    }
}

void selectMPU(uint8_t mpu) {
    uint8_t highMPU = mpu == 0 ? (n - 1) : (mpu - 1);
    digitalWrite(AD0_MPU[highMPU], LOW);
    digitalWrite(AD0_MPU[mpu], HIGH);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void deselectMPUs() {
    for (uint8_t i = 0; i < n; i++) {
        digitalWrite(AD0_MPU[i], LOW);
    }
}

void initMPUs() {
    Serial.println("Inicializando MPUs...");
    for (uint8_t i = 0; i < n; i++) {
        pinMode(AD0_MPU[i], OUTPUT);
        digitalWrite(AD0_MPU[i], LOW);
    }

    for (uint8_t i = 0; i < n; i++) {
        selectMPU(i);
        setupMPU();
    }
    deselectMPUs();
    Serial.println("Inicialização dos MPUs completa.");
}

void Task2(void *pvParameters) {
    Wire.begin();
    Wire.setClock(400000);

    initMPUs();

    while (true) {
        if (changeState && dataSent) {
            File fileToErase = SPIFFS.open(fileName, "w");
            if (fileToErase) {
                fileToErase.close();
                Serial.println("Arquivo esvaziado.");
            } else {
                Serial.println("Falha ao abrir o arquivo para esvaziar.");
            }

            IMUData discardData;
            while (xQueueReceive(imuDataQueue, &discardData, 0) == pdPASS) {
                // Dados descartados
            }

            changeState = false;
        }

        if (runCollect) {
            for (uint8_t i = 0; i < n; i++) {
                selectMPU(i);
                getIMUData(i);

                int dynamicDelay = 10;
                if (uxQueueSpacesAvailable(imuDataQueue) < 3) {
                    dynamicDelay = 20;
                    Serial.println("Fila com pouco espaço");
                }

                vTaskDelay(pdMS_TO_TICKS(dynamicDelay));
            }
            deselectMPUs();
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
