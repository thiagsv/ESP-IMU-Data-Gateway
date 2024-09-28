#include "include/tasks.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
QueueHandle_t imuDataQueue;
AsyncWebServer server(80);

int runCollect = 0; 
const char *ssid = "ESP32_AP";
const char *password = "123456789";
String fileName = "/imuData.txt";

void setup() {
  Serial.begin(115200);
  delay(1000);
  // Montar SPIFFS
  if (!SPIFFS.begin(true)) {
      Serial.println("Falha ao montar o sistema de arquivos SPIFFS.");
  } else {
      Serial.println("SPIFFS montado com sucesso.");
  }

  // Criar dados mock (ou continuar a escrever no arquivo existente)
  createMockIMUData();

  // Criar ponto de acesso Wi-Fi
  WiFi.softAP(ssid, password);
  Serial.println("Ponto de acesso criado");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Endpoint para inverter o valor de runCollect
  server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request) {
      runCollect = !runCollect;
      Serial.println("Valor de runCollect invertido para: " + String(runCollect));
      request->send(200, "text/plain", String(runCollect));
  });

  // Endpoint para obter dados do arquivo
  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (runCollect == 0) {
          File file = SPIFFS.open(fileName, "r");
  
          if (file) {
              request->send(file, fileName, "text/plain", true); 
          } else {
              request->send(500, "text/plain", "Falha ao abrir o arquivo no SPIFFS");
          }
      } else {
          request->send(403, "text/plain", "Coleta de dados em andamento. Tente mais tarde.");
      }
  });

  server.begin();

  imuDataQueue = xQueueCreate(10, sizeof(String)); 

  xTaskCreatePinnedToCore(
    Task1,
    "Task1",
    4096,
    NULL,
    1,
    &task1Handle,
    0
  );

  xTaskCreatePinnedToCore(
    Task2,
    "Task2",
    8192,
    NULL,
    2,
    &task2Handle,
    1
  );
}

void loop() {}

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
      file.print(";");     // Adiciona ";" no final da linha e nova linha
  }

  // Fecha o arquivo
  file.close();
  Serial.println("Dados mock sobrescritos no arquivo imuData.txt.");

}
