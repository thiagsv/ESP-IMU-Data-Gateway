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

void setup() {
    Serial.begin(115200);
    delay(500);  // Atraso para garantir a inicialização da serial

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
        runCollect = !runCollect;
        initialTime = millis();
        Serial.println("Valor de runCollect invertido para: " + String(runCollect));
        request->send(200, "text/plain", String(runCollect));
    });

    Serial.println("Criado /toggle");

    delay(500);

    server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Tentar abrir o arquivo para leitura
        // Serial.println("Tentando abrir o arquivo para leitura...");

        // Usar ponteiro dinâmico para garantir que o arquivo seja mantido aberto durante a operação
        File* filePtr = new File(SPIFFS.open(fileName, "r"));
        if (!(*filePtr)) {
            // Log de erro ao abrir o arquivo
            Serial.println("Erro: Falha ao abrir o arquivo no SPIFFS.");
            request->send(500, "text/plain", "Falha ao abrir o arquivo no SPIFFS");
            delete filePtr; // Limpar o ponteiro
            return;
        }

        // Log de sucesso na abertura do arquivo
        // Serial.println("Arquivo aberto com sucesso.");

        // Enviar o conteúdo em chunks
        AsyncWebServerResponse *response = request->beginChunkedResponse("text/plain", 
            [filePtr](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                // VefilePtrse o arquivo está disponível e foi aberto corretamente
                if (!filePtr || !filePtr->available()) {
                    Serial.println("Erro: Arquivo não disponível ou ponteiro inválido.");
                    filePtr->close();
                    delete filePtr;
                    dataSent = true;
                    return 0; // Finaliza a leitura
                }

                // Log do índice e tamanho máximo de leitura
                // Serial.print("Lendo arquivo no índice: ");
                // Serial.println(index);
                // Serial.print("Tamanho máximo de leitura: ");
                // Serial.println(maxLen);

                // Mover o cursor do arquivo para a posição correta
                filePtr->seek(index);

                // Ler o próximo pedaço de dados
                size_t bytesRead = filePtr->read(buffer, maxLen);

                // Log da quantidade de bytes lidos
                // Serial.print("Bytes lidos: ");
                // Serial.println(bytesRead);

                // Verifica se terminou de ler o arquivo
                if (bytesRead == 0) {
                    // Serial.println("Leitura completa, fechando o arquivo.");
                    filePtr->close();  // Fechar o arquivo após a leitura completa
                    delete filePtr;    // Limpar o ponteiro dinâmico
                    dataSent = true;
                    // Serial.println("Arquivo fechado e ponteiro limpo.");
                    return 0; // Retornar 0 para indicar o fim do arquivo
                }

                // Retorna o número de bytes lidos
                return bytesRead;
            }
        );

        // Enviar a resposta
        request->send(response);

        // Log de envio da resposta
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
