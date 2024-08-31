#include "include/tasks.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
QueueHandle_t imuDataQueue;
AsyncWebServer server(80);

int runCollect = 0; 
const char *ssid = "ESP32_AP";
const char *password = "123456789";
String fileName = "imuData.txt";

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
      Serial.println("Falha ao montar o sistema de arquivos SPIFFS.");
  } else {
      Serial.println("SPIFFS montado com sucesso.");
  }

  WiFi.softAP(ssid, password);
  Serial.println("Ponto de acesso criado");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
      runCollect = !runCollect;
      if (runCollect) {
        initialTime = millis();
        fileName = "/" + String(initialTime) + ".txt";
      }
      request->send(200, "text/plain", String(runCollect));
      Serial.println("Valor de runCollect invertido para: " + String(runCollect));
  });

  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (runCollect == 0) {
          File file = SPIFFS.open("/" + fileName, "r");
  
          if (file) {
              // Enviar o arquivo como um stream assíncrono
              request->send(file, fileName, "text/plain", true); 
              // 'true' habilita o envio em chunks assíncronos
          } else {
              request->send(500, "text/plain", "Failed to open file on SPIFFS");
          }
      } else {
          request->send(403, "text/plain", "Data collection is in progress. Try again later.");
      }
  });


  server.begin();

  imuDataQueue = xQueueCreate(10, sizeof(String)); 

  xTaskCreatePinnedToCore(
    Task1,
    "Task1",
    8192,
    NULL,
    1,
    &task1Handle,
    0
  );

  xTaskCreatePinnedToCore(
    Task2,
    "Task2",
    16384,
    NULL,
    2,
    &task2Handle,
    1
  );
}

void loop() {}
