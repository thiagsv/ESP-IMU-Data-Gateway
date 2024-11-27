# ESP-IMU-Data-Gateway

Projeto para ESP32 que coleta dados de um sensor IMU (MPU6050) e os transmite via Wi-Fi para um servidor remoto.

---

## Funcionalidades

- Leitura de aceleração e giroscópio do sensor MPU6050.
- Conexão com rede Wi-Fi.
- Transmissão de dados para um servidor remoto via TCP.

---

## Configuração

1. No arquivo `src/main.cpp`, configure:
   - **Credenciais Wi-Fi**:
     ```cpp
     const char* ssid = "SEU_SSID";
     const char* password = "SUA_SENHA";
     ```
   - **Servidor Remoto**:
     ```cpp
     const char* host = "IP_DO_SERVIDOR";
     const int port = 8080;
     ```

2. Faça o upload do código para o ESP32 utilizando o Arduino IDE.
