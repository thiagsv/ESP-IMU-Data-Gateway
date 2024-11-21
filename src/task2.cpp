#include "../include/tasks.h"

const int MPU_ADDR = 0x69;
const uint8_t n = 5;
const uint8_t AD0_MPU[] = {17, 16, 4, 2, 15};

void setupMPU() {
    // Reset do MPU para garantir que ele inicie corretamente
    // Wire.beginTransmission(MPU_ADDR);
    // Wire.write(0x6B);  // Registrador de gerenciamento de energia
    // Wire.write(0x80);  // Comando de reset
    // Wire.endTransmission(false);

    // vTaskDelay(pdMS_TO_TICKS(100));  // Aguarde o reset completar

    // Sair do modo sleep e usar o clock PLL
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x01);
    Wire.endTransmission(true);

    // Adicione um pequeno delay para permitir que o MPU estabilize
    vTaskDelay(pdMS_TO_TICKS(10));

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
    Wire.endTransmission(true);

    // --- Configura a sensibilidade do acelerômetro ---
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);  // Define a faixa de ±2g para o acelerômetro
    Wire.endTransmission(true);

    // --- Configura a sensibilidade do giroscópio ---
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B);
    Wire.write(0x08);  // Define a faixa de ±500°/s para o giroscópio
    Wire.endTransmission(true);

    // --- Configura o filtro passa-baixa ---
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1A);
    Wire.write(0x05);
    Wire.endTransmission(true);
}

void deselectMPUs() {
    for (uint8_t i = 0; i < n; i++) {
        digitalWrite(AD0_MPU[i], LOW);
    }
}

void selectMPU(uint8_t mpu) {
    digitalWrite(AD0_MPU[mpu], HIGH);
}

void deselectMPU(uint8_t mpu) {
    digitalWrite(AD0_MPU[mpu], LOW);
}

bool getIMUData(uint8_t mpu, IMUData &imuData) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);  // Endereço do registrador do acelerômetro

    int transmissionStatus = Wire.endTransmission(false);  // Envia comando sem finalizar I2C

    if (transmissionStatus != 0) {
        Serial.print("Erro na transmissão com MPU ");
        Serial.print(mpu);
        Serial.print(". Código de erro: ");
        Serial.println(transmissionStatus);
        return false;
    }

    if (Wire.requestFrom(MPU_ADDR, 14, true) != 14) {
        Serial.print("Erro: não foi possível ler os dados (14 bits) do MPU: ");
        Serial.println(AD0_MPU[mpu]);
        return false;
    }

    int16_t AcX = Wire.read() << 8 | Wire.read();
    int16_t AcY = Wire.read() << 8 | Wire.read();
    int16_t AcZ = Wire.read() << 8 | Wire.read();
    int16_t Tmp = Wire.read() << 8 | Wire.read();
    int16_t GyX = Wire.read() << 8 | Wire.read();
    int16_t GyY = Wire.read() << 8 | Wire.read();
    int16_t GyZ = Wire.read() << 8 | Wire.read();

    imuData.Id = mpu;
    imuData.AcX = float(AcX) / 16384;
    imuData.AcY = float(AcY) / 16384;
    imuData.AcZ = float(AcZ) / 16384;
    imuData.GyX = float(GyX) / 65.5;
    imuData.GyY = float(GyY) / 65.5;
    imuData.GyZ = float(GyZ) / 65.5;
    imuData.Timestamp = float(micros() - initialTime) / 1000000;

    // Serial.print("Id: "); Serial.print(mpu);
    // Serial.print(", AcX: "); Serial.print(imuData.AcX, 6);
    // Serial.print(", AcY: "); Serial.print(imuData.AcY, 6);
    // Serial.print(", AcZ: "); Serial.print(imuData.AcZ, 6);
    // Serial.print(", GyX: "); Serial.print(imuData.GyX, 6);
    // Serial.print(", GyY: "); Serial.print(imuData.GyY, 6);
    // Serial.print(", GyZ: "); Serial.println(imuData.GyZ, 6);

    return true;
}

bool collectAllIMUData(IMUData imuDataArray[]) {
    bool allDataValid = true;  // Marca se todos os dados são válidos

    for (uint8_t i = 0; i < n; i++) {
        selectMPU(i);
        if (!getIMUData(i, imuDataArray[i])) {  // Chama getIMUData e valida retorno
            allDataValid = false;
            Serial.print("Erro ao coletar dados do MPU ");
            Serial.println(AD0_MPU[i]);
            break;  // Para se algum MPU falhar
        }
        deselectMPU(i);
    }

    deselectMPUs();

    return allDataValid;  // Retorna true se todos os dados forem válidos
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
        deselectMPU(i);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    deselectMPUs();
    Serial.println("Inicialização dos MPUs completa.");
}

void Task2(void *pvParameters) {
    Wire.begin();
    Wire.setClock(400000);

    initMPUs();
    int queueCapacity = 2000;  // Capacidade máxima da fila
    IMUData imuDataArray[n];

    while (true) {
        if (runCollect) {
            int dynamicDelay = 5;  // Atraso inicial
            int availableSpaces = uxQueueSpacesAvailable(imuDataQueue);

            // Calcula o ajuste proporcional com base no espaço disponível
            float fillLevel = (float)(queueCapacity - availableSpaces) / queueCapacity;
            if (fillLevel >= 0.5) {
                dynamicDelay = (int)(100 * fillLevel);  // Ajusta o delay de forma proporcional (escala 1 a 100)
            }

            // Coleta os dados de todos os MPUs
            if (collectAllIMUData(imuDataArray)) {
                // Se todos os dados forem válidos, escreve na fila
                for (uint8_t i = 0; i < n; i++) {
                    if (xQueueSend(imuDataQueue, &imuDataArray[i], 0) != pdPASS) {
                        Serial.print("Falha ao enviar dados do MPU ");
                        Serial.println(i);
                    }
                }
            }

            vTaskDelay(pdMS_TO_TICKS(dynamicDelay));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}
