#include "tasks.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
QueueHandle_t imuDataQueue;
AsyncWebServer server(80);

int runCollect = 0; 
const char *ssid = "ESP32_AP";
const char *password = "123456789";

void setup() {
  Serial.begin(115200);

  if (!SD.begin()) {
    Serial.println("Failed to initialize SD card.");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  WiFi.softAP(ssid, password);
  Serial.println("Ponto de acesso criado");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
      runCollect = !runCollect;
      if (runCollect) {
        initialTime = millis();
      }
      request->send(200, "text/plain", String(runCollect));
      Serial.println("Valor de runCollect invertido para: " + String(runCollect));
  });

  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (runCollect == 0) {
          File file = SD.open("/data.txt");
          if (file) {
              request->send(file, "/data.txt", "text/plain");
              file.close();
          } else {
              request->send(500, "text/plain", "Failed to open file on SD card");
          }
      } else {
          request->send(403, "text/plain", "Data collection is in progress. Try again later.");
      }
  });

  server.begin();

  imuDataQueue = xQueueCreate(10, sizeof(String));  // Ajuste o tamanho conforme necessário

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
