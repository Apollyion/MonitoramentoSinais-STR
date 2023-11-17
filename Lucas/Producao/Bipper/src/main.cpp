#include <ESP8266WiFi.h>
#include <WiFiClient.h>

char ssid[] = "Moto G (8) Power";          // SSID da rede WiFi
char password[] = "123456789";           // Senha da rede WiFi
const char *serverIP = "192.168.182.133"; // Endereço IP do servidor
int serverPort = 12345;                 // Porta do servidor
int ledPin = D5;                        // Pino do LED
int buttonPin = D6;                     // Pino do botão

WiFiClient client;
bool ledBlinking = false;

void connectToWiFi()
{
    Serial.println("Conectando ao WiFi...");
    WiFi.begin(ssid, password);
    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 10)
    {
        delay(1000);
        Serial.println("Tentativa de conexão WiFi...");
        tentativas++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Conectado ao WiFi");
    }
    else
    {
        Serial.println("Falha na conexão WiFi");
    }
}

void connectToServer()
{
    Serial.println("Conectando ao servidor...");
    if (client.connect(serverIP, serverPort))
    {
        Serial.println("Conectado ao servidor");
        client.print("nodemcu"); // Envia o nome do dispositivo
    }
    else
    {
        Serial.println("Falha na conexão com o servidor");
    }
}


void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);

    // Indica ligação
    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin, LOW);

    connectToWiFi(); // Conectar ao WiFi


    connectToServer(); // Conectar ao servidor

    //Indica que saiu do setup
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);
    }
}

void loop()
{
    // Reconectar ao WiFi se necessário
    if (WiFi.status() != WL_CONNECTED)
    {
        connectToWiFi();
    }

    // Reconectar ao servidor se necessário
    if (!client.connected())
    {
        connectToServer();
    }

    // Ler dados do servidor, se necessário
    while (client.available())
    {
        String data = client.readStringUntil('\n');
        Serial.println("Dados recebidos do servidor: " + data);

        if (data == "1")
        {
            // Acender o LED e fazer piscar até que o botão seja pressionado
            ledBlinking = true;
        }
    }

    // Controlar o LED
    if (ledBlinking)
    {
        digitalWrite(ledPin, HIGH);
        delay(500);
        digitalWrite(ledPin, LOW);
        delay(500);

        // Verificar se o botão foi pressionado para parar o piscar do LED
        if (digitalRead(buttonPin) == LOW)
        {
            ledBlinking = false;
            digitalWrite(ledPin, LOW);
        }
    }
}

