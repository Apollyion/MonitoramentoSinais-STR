#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid = "brisa-2508471";
const char* password = "jp3lhy8a";
const char* serverIP = "192.168.0.106";
int serverPort = 12345;
int flag = 0;

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

      // Faz isso somente 10 vezes e tira a media
      int n = 10;
      unsigned long sum = 0;
      if (flag == 0) {
      // Medindo tempo de envio de dados
      for (int i = 0; i < n; i++) {
        unsigned long start = millis();
        client.print("Dados do ESP32");
        client.flush();
        unsigned long end = millis();
        sum += end - start;
        flag=1;
      } 

      Serial.print("Tempo médio de envio: ");
      Serial.print(sum/n);
      }


    } else {
      Serial.println("Falha na conexão com o servidor");
    }
  }
}
