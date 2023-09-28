#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid = "brisa-2508471";
const char* password = "jp3lhy8a";
const char* serverIP = "127.0.1.1";
int serverPort = 12345;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");
}

void loop() {
  if (!client.connected()) {
    Serial.println("Conectando ao servidor...");
    if (client.connect(serverIP, serverPort)) {
      Serial.println("Conectado ao servidor");
      // Enviar dados para o servidor
      client.print("Dados do ESP32");
      client.flush();
    } else {
      Serial.println("Falha na conexão com o servidor");
    }
  }
}
