#include "include/tasks.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
QueueHandle_t imuDataQueue;
AsyncWebServer server(80);

int runCollect = 0;
const char *ssid = "ESP32_AP";
const char *password = "123456789";
String fileName = "/imuData.txt";
bool dataSent = false;
unsigned long initialTime;
const int LED_GREEN_PIN = 5;
const int LED_RED_PIN = 18;

void setup() {
    Serial.begin(115200);
    delay(500);  // Atraso para garantir a inicialização da serial

    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, HIGH);

    // Teste de Montagem do SPIFFS
    Serial.println("Iniciando SPIFFS...");
    if (!SPIFFS.begin(true)) {
        Serial.println("Erro ao montar o SPIFFS");
    } else {
        Serial.println("SPIFFS montado com sucesso.");
    }

    // Checagem de Memória Antes de Inicializar o Wi-Fi
    delay(500);

    // Criação do Ponto de Acesso Wi-Fi
    Serial.println("Tentando criar o ponto de acesso...");
    WiFi.softAP(ssid, password);

    // Verificação do Estado do Ponto de Acesso
    if (WiFi.softAPgetStationNum() >= 0) {
        Serial.println("Ponto de acesso criado com sucesso");
    } else {
        Serial.println("Erro ao criar o ponto de acesso");
    }

    // Verifique se o Ponto de Acesso está ativo e obtenha o IP
    Serial.println("Obtendo o IP address...");
    IPAddress ip = WiFi.softAPIP();
    
    // Certifique-se de que o IP foi obtido corretamente
    if (ip) {
        Serial.print("IP address: ");
        Serial.println(ip);
    } else {
        Serial.println("Erro ao obter o IP address.");
    }

    delay(500);

    // Endpoint para inverter o valor de runCollect
    server.on("/toggle", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!runCollect) {
            File file = SPIFFS.open(fileName, "w");  // Abre o arquivo em modo de escrita, o que apaga o conteúdo
            if (!file) {
                Serial.println("Erro ao abrir o arquivo para apagar o conteúdo.");
            } else {
                file.close();  // Fecha o arquivo depois de apagá-lo
                Serial.println("Conteúdo do arquivo apagado com sucesso.");
            }
        }
        runCollect = !runCollect;

        if (runCollect) {
            digitalWrite(LED_GREEN_PIN, HIGH);
            digitalWrite(LED_RED_PIN, LOW);
        } else {
            digitalWrite(LED_GREEN_PIN, LOW);
            digitalWrite(LED_RED_PIN, HIGH);
        }

        initialTime = millis();
        Serial.println("Valor de runCollect invertido para: " + String(runCollect));
        request->send(200, "text/plain", String(runCollect));
    });

    Serial.println("Criado /toggle");

    delay(500);

    server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) {
        File file = SPIFFS.open(fileName, "r");
        if (!file) {
            Serial.println("Erro: Falha ao abrir o arquivo no SPIFFS.");
            request->send(500, "text/plain", "Falha ao abrir o arquivo no SPIFFS");
            return;
        }

        AsyncWebServerResponse *response = request->beginChunkedResponse("text/plain", 
            [file](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
                if (!file.available()) {
                    file.close();
                    return 0;
                }

                size_t bytesRead = file.read(buffer, maxLen);
                if (bytesRead == 0) {
                    file.close();
                }

                return bytesRead;
            }
        );

        request->send(response);
        Serial.println("Resposta enviada ao cliente.");
    });

    Serial.println("Criado /getData");
    delay(500);

    server.begin();

    Serial.println("Servidor iniciado");
    delay(500);
  
    imuDataQueue = xQueueCreate(1000, sizeof(IMUData));

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
        4096,
        NULL,
        1,
        &task2Handle,
        1
    );
}

void loop() {}
