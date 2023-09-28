// Importando bibliotecas
#include <Arduino.h>
const int sensorPin = 13; // Pino analógico onde o sensor de pulsação está conectado
const int ledPin = 2;     // Pino para acender um LED quando uma batida for detectada

int valorLido;             // Valor lido do sensor de pulsação
int valorMinimo = 2500;     // Valor mínimo para considerar uma batida
unsigned long ultimoTempo = 0; // Último tempo em que uma batida foi detectada
int bpm = 0;               // Batimentos por minuto

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  valorLido = analogRead(sensorPin); // Ler o valor do sensor de pulsação

  if (valorLido > valorMinimo && (millis() - ultimoTempo) > 600) {
    // Se um pico for detectado e tempo suficiente passou desde a última detecção
    digitalWrite(ledPin, HIGH); // Acender o LED
    ultimoTempo = millis();     // Registrar o tempo da detecção
    bpm++;                      // Incrementar o contador de batimentos
  } else {
    digitalWrite(ledPin, LOW); // Desligar o LED
  }

  if (millis() - ultimoTempo > 5000) {
    // Se não houver detecções por 5 segundos, redefinir o contador de BPM
    bpm = 0;
  }

  // Exibir o valor atual de BPM no Monitor Serial
  Serial.print("Valor lido: ");
  Serial.print(valorLido);
  Serial.print("  BPM: ");
  Serial.println(bpm);
  

  delay(10); // Pequeno atraso para estabilidade
}
