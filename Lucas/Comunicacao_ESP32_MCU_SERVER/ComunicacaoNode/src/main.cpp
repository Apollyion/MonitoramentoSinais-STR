#include <ESP8266WiFi.h>
#include <WiFiClient.h>

char ssid[] = "brisa-2508471"; // SSID da rede WiFi
char password[] = "jp3lhy8a"; // Senha da rede WiFi
const char* serverIP = "192.168.0.106"; // Endereço IP do servidor
int serverPort = 12345; // Porta do servidor

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
      client.print("nodemcu"); // Envia o nome do dispositivo
    } else {
      Serial.println("Falha na conexão com o servidor");
    }
  }

  // Ler dados do servidor, se necessário


  while (client.available()) {
    String data = client.readStringUntil('\n');
    Serial.println("Dados recebidos do servidor: " + data);
  }
}