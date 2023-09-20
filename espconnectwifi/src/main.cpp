#include <Arduino.h>
#include <ESP8266WiFi.h> //biblioteca wifi do nodemcu
#include <aREST.h>

// biblioteca aRest https://www.arduino.cc/reference/en/libraries/arest/

const char *ssid = "brisa-2508471"; // nome da rede wifi
const char *password = "jp3lhy8a";  // senha rede wifi

// declare port number for listening incoming TCP connections
#define LISTEN_PORT 80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Create aREST instance
aREST rest = aREST();

int ledpin = 2; // pino d4 do nodemcu
int botao = 4;  // entrada d2
int estadodobotao = 0;
int estadoanterior = 0;
int contbtn;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(botao, INPUT_PULLUP);
  digitalWrite(ledpin, LOW); // desliga o ledpin

  Serial.begin(921600); // indicar qual porta serial sera utiliada

  // Initialize variables for REST API
  rest.variable("contbtn", &contbtn);

  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("esp8266");

  Serial.println();
  Serial.print("connecting to WiFi: ");
  Serial.print(ssid);

  WiFi.begin(ssid, password); // funcao para se conectar a uma rede wifi
  Serial.println("");
  Serial.print("Connecting...");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(".");
  }

  // Start the server
  server.begin();
  Serial.println("Server started");

  Serial.print("NodeMCU IP Address:");
  Serial.print(WiFi.localIP());
}

void loop()
{
  estadodobotao = digitalRead(botao);

  if (estadodobotao != estadoanterior)
  {
    if (estadodobotao == HIGH)
    {
      digitalWrite(ledpin, LOW);
    }
    else
    {
      digitalWrite(ledpin, HIGH);
      contbtn++;
      Serial.println();
      Serial.println("N de vezes apertado");
      Serial.println(contbtn);
    }
  }
  estadoanterior = estadodobotao;

  // Handle REST calls
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
  while (!client.available())
  {
    delay(1);
  }
  rest.handle(client);
}
