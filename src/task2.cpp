#include "../include/tasks.h"

const int MPU_ADDR = 0x69;
const uint8_t n = 13;
const uint8_t AD0_MPU[] = {15, 2, 4, 16, 17, 3, 1, 13, 32, 33, 25, 26, 27};
unsigned long initialTime = 0;

void setupMPU() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B); 
    Wire.write(0x00);
    Wire.write(0x80);
    Wire.endTransmission(false);

    delay(100);

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
        Serial.println("Erro: não foi possível ler os dados do MPU.");
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

    Serial.print("Id: "); Serial.print(mpu);
    Serial.print(", AcX: "); Serial.print(imuData.AcX);
    Serial.print(", AcY: "); Serial.print(imuData.AcY);
    Serial.print(", AcZ: "); Serial.print(imuData.AcZ);
    Serial.print(", GyX: "); Serial.print(imuData.GyX);
    Serial.print(", GyY: "); Serial.print(imuData.GyY);
    Serial.print(", GyZ: "); Serial.print(imuData.GyZ);

    if (xQueueSend(imuDataQueue, &imuData, 0) != pdPASS) {
        Serial.println("Falha ao enviar dados para a fila.");
    }
}

void selectMPU(uint8_t mpu) {
    uint8_t highMPU = mpu == 0 ? (n - 1) : (mpu - 1);
    digitalWrite(AD0_MPU[highMPU], LOW);
    digitalWrite(AD0_MPU[mpu], HIGH);
    delay(10);
}

void deselectMPUs() {
    for (uint8_t i = 0; i < n; i++) {
        digitalWrite(AD0_MPU[i], LOW);
    }
}

// Função para criar/atualizar o arquivo imuData.txt com dados mock
void createMockIMUData() {
  // Gera um arquivo mock com dados do IMU no SPIFFS no modo WRITE (sobrescrevendo o arquivo existente)
  File file = SPIFFS.open(fileName, FILE_WRITE);  // Usando FILE_WRITE para sobrescrever

  if (!file) {
      Serial.println("Falha ao abrir o arquivo imuData.txt para escrita.");
      return;
  }

  // Dados mock (AcX, AcY, AcZ, GyX, GyY, GyZ, Timestamp)
  for (int i = 0; i < 10; i++) {
      file.print(i); file.print(","); // AcX
      file.print(i); file.print(","); // AcY
      file.print(i); file.print(","); // AcZ
      file.print(i); file.print(","); // GyX
      file.print(i); file.print(","); // GyY
      file.print(i); file.print(","); // GyZ
      file.print(millis());  // Timestamp 
      file.print(";\n");     // Adiciona ";" no final da linha e nova linha
  }

  // Fecha o arquivo
  file.close();
  Serial.println("Dados mock sobrescritos no arquivo imuData.txt.");
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
        setupMPU();
    }
    deselectMPUs();

    while (true) {
        if (runCollect) {
            for (uint8_t i = 0; i < n; i++) {
                selectMPU(i);
                readIMUData(i);
                // Criar dados mock (ou continuar a escrever no arquivo existente)
                // createMockIMUData();
                vTaskDelay(pdMS_TO_TICKS(500));  // Delay to avoid overwhelming the system
            }
            deselectMPUs();
        } else {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}
