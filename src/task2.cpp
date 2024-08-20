#include "../include/tasks.h"

const int MPU_ADDR = 0x69;
const uint8_t n = 5;
volatile uint8_t index_data = 0;

const uint8_t AD0_MPU[] = {15, 2, 4, 16, 17, 3, 1, 13, 32, 33, 25, 26, 27};
unsigned long initialTime = 0;

void setupMPU(uint8_t mpu) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B); 
    Wire.write(0x01);
    Wire.endTransmission(false);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x19);
    Wire.write(0x00);
    Wire.endTransmission(false);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);
    Wire.endTransmission(false);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B);
    Wire.write(0x08);
    Wire.endTransmission(false);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1A);
    Wire.write(0x05);
    Wire.endTransmission(false);
}

void readIMUData(uint8_t mpu) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true);

    int16_t AcX = Wire.read() << 8 | Wire.read();
    int16_t AcY = Wire.read() << 8 | Wire.read();
    int16_t AcZ = Wire.read() << 8 | Wire.read();
    int16_t Tmp = Wire.read() << 8 | Wire.read();
    int16_t GyX = Wire.read() << 8 | Wire.read();
    int16_t GyY = Wire.read() << 8 | Wire.read();
    int16_t GyZ = Wire.read() << 8 | Wire.read();

    IMUData imuData;
    imuData.AcX = double(AcX) / 16384;
    imuData.AcY = double(AcY) / 16384;
    imuData.AcZ = double(AcZ) / 16384;
    imuData.GyX = double(GyX) / 65.5;
    imuData.GyY = double(GyY) / 65.5;
    imuData.GyZ = double(GyZ) / 65.5;
    imuData.Timestamp = double(millis() - initialTime) / 1000;

    if (xQueueSend(imuDataQueue, &imuData, 0) != pdPASS) {
        Serial.println("Falha ao enviar dados para a fila.");
    }
}

void selectMPU(uint8_t mpu) {
    uint8_t highMPU = mpu == 0 ? (n - 1) : (mpu - 1);
    digitalWrite(AD0_MPU[highMPU], LOW);
    digitalWrite(AD0_MPU[mpu], HIGH);
}

void deselectMPUs() {
    for (uint8_t i = 0; i < n; i++) {
        digitalWrite(AD0_MPU[i], LOW);
    }
}

void Task2(void *pvParameters) {
    Wire.begin();
    Wire.setClock(400000);

    for (uint8_t i = 0; i < n; i++) {
        pinMode(AD0_MPU[i], OUTPUT);
        digitalWrite(AD0_MPU[i], LOW);
    }

    for (uint8_t i = 0; i < n; i++) {
        selectMPU(i);
        setupMPU(i);
    }
    deselectMPUs();

    while (true) {
        if (runCollect) {
            for (uint8_t i = 0; i < n; i++) {
                selectMPU(i);
                readIMUData(i);
                vTaskDelay(pdMS_TO_TICKS(500));  // Delay to avoid overwhelming the system
            }
            deselectMPUs();
        } else {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}
