#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid = "brisa-2508471"; // SSID da rede WiFi
const char* password = "jp3lhy8a"; // Senha da rede WiFi
const char* serverIP = "192.168.0.106"; // Endereço IP do servidor
int serverPort = 12345; // Porta do servidor
int flag = 0; // Variável de controle

WiFiClient client; // Cliente WiFi

void setup() {
  Serial.begin(115200); // Inicia a comunicação serial
  WiFi.begin(ssid, password); // Conecta à rede WiFi
  while (WiFi.status() != WL_CONNECTED) { // Espera a conexão com a rede WiFi
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

}

void loop() {
  if (!client.connected()) { // Se não estiver conectado ao servidor
    Serial.println("Conectando ao servidor...");
    if (client.connect(serverIP, serverPort)) { // Conecta ao servidor
      Serial.println("Conectado ao servidor");
      // Enviar dados para o servidor

      // Faz isso somente 10 vezes e tira a media
      int n = 100;
      unsigned long sum = 0;
      for (int i = 0; i < n; i++) {
        unsigned long t0 = millis();
        client.print("Ola servidor!");
        client.flush();
        unsigned long t1 = millis();
        sum += t1 - t0;
      }
      Serial.print("Tempo médio de envio: ");
      Serial.print(sum/n);
    }
  }
}

