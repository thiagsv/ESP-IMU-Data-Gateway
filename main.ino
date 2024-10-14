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
              request->send(file, "text/plain", true);
              file.close();  // Fecha o arquivo após o envio
          } else {
              request->send(500, "text/plain", "Falha ao abrir o arquivo no SPIFFS");
          }
      } else {
          request->send(403, "text/plain", "Coleta de dados em andamento. Tente mais tarde.");
      }
  });

  server.begin();

  imuDataQueue = xQueueCreate(300, sizeof(IMUData)); 

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
    1,
    &task2Handle,
    1
  );
}

void loop() {}
