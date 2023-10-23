#include <Arduino.h>
#include <Wire.h>

// ****INICIO CONFIGURACOES DO MPU**** 
// DEFINICOES DO MPU 
const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;
boolean fall = false;
boolean trigger1=false;
boolean trigger2=false;
boolean trigger3=false;
byte trigger1count=0;
byte trigger2count=0;
byte trigger3count=0;
int angleChange=0;

// MPU read function
void mpu_read() {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);
    AcX=Wire.read()<<8|Wire.read();
    AcY=Wire.read()<<8|Wire.read();
    AcZ=Wire.read()<<8|Wire.read();
    Tmp=Wire.read()<<8|Wire.read();
    GyX=Wire.read()<<8|Wire.read();
    GyY=Wire.read()<<8|Wire.read();
    GyZ=Wire.read()<<8|Wire.read();
}

// Function to detect fall
bool detectFall() {
    mpu_read();
    ax = (AcX-2050)/16384.00;
    ay = (AcY-77)/16384.00;
    az = (AcZ-1947)/16384.00;
    gx = (GyX+270)/131.07;
    gy = (GyY-351)/131.07;
    gz = (GyZ+136)/131.07;
    float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
    int Amp = Raw_Amp * 10;

    if (Amp <= 2 && !trigger2) {
        trigger1 = true;
    }

    if (trigger1) {
        trigger1count++;
        if (Amp >= 12) {
            trigger2 = true;
            trigger1 = false;
            trigger1count = 0;
        }
    }

    if (trigger2) {
        trigger2count++;
        angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
        if (angleChange >= 30 && angleChange <= 400) {
            trigger3 = true;
            trigger2 = false;
            trigger2count = 0;
        }
    }

    if (trigger3) {
        trigger3count++;
        if (trigger3count >= 10) {
            angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
            if (angleChange >= 0 && angleChange <= 10) {
                fall = true;
                trigger3 = false;
                trigger3count = 0;
            } else {
                trigger3 = false;
                trigger3count = 0;
            }
        }
    }

    if (fall) {
        return true;
    }

    if (trigger2count >= 6) {
        trigger2 = false;
        trigger2count = 0;
    }

    if (trigger1count >= 6) {
        trigger1 = false;
        trigger1count = 0;
    }

    return false;
}

// }*****FIM DAS CONFIGURACOES DO MPU****


// ****INICIO CONFIGURACOES DO PULSE SENSOR****
// DEFINICOES DO PULSE SENSOR:
hw_timer_t * sampleTimer = NULL;
portMUX_TYPE sampleTimerMux = portMUX_INITIALIZER_UNLOCKED;
#define USE_ARDUINO_INTERRUPTS true
//#define NO_PULSE_SENSOR_SERIAL true

#include <PulseSensorPlayground.h>
PulseSensorPlayground pulseSensor;


const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;    
const int PULSE_FADE = 5;
const int THRESHOLD = 685;   // Ajustar o valor caso tenha ruído


// Pulse Sensor Read function
int getBPM() {
  if (pulseSensor.sawStartOfBeat()) {
    return pulseSensor.getBeatsPerMinute();
  } else {
    return 0;
  }
}

// *****FIM DAS CONFIGURACOES DO PULSE SENSOR****


// ****INICIO CONFIGURACOES DO WIFI****
// DEFINICOES DO WIFI:
#include <WiFi.h>

const char* ssid = "NOME_DA_REDE"; // SSID da rede WiFi
const char* password = "SENHA_DA_REDE"; // Senha da rede WiFi
const char* serverIP = "192.168.0.106"; // Endereço IP do servidor
int serverPort = 12345; // Porta do servidor

WiFiClient client;

// Funcao para conectar ao servidor
void primeira_conecao() {
  Serial.println("Conectando ao servidor...");
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Conectado ao servidor");
    client.println("esp32");
  } else {
    Serial.println("Falha na conexão com o servidor");
  }
}

// Funcao para enviar dados ao servidor
void send_event(String event_name) {
  if (!client.connected()) {
    primeira_conecao();
  }
  client.println(event_name);
  client.flush();
  Serial.println("Dados enviados ao servidor: " + event_name);
}


void handleWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        // Reconnect logic
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi conectado");
        Serial.println("Endereço IP: ");
        Serial.println(WiFi.localIP());
    }
}



// *****FIM DAS CONFIGURACOES DO WIFI****

// ***** CONFIGURACAO DO BOTAO INTERRUPTOR *****
// DEFINICOES DO BOTAO INTERRUPTOR:
#define BOTAO_PINO 2 // Define o pino do botão
volatile bool botaoPressionado = false; // Variável que será alterada pela interrupção

// Função de tratamento de interrupção
void IRAM_ATTR trataInterrupcao() {
  botaoPressionado = true;
}

// ***** FIM DAS CONFIGURACOES DO BOTAO INTERRUPTOR *****


void setup() {
    Serial.begin(115200);
    // MPU setup
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);


    // PULSE SENSOR SETUP:
    analogReadResolution(10);
    pulseSensor.analogInput(PULSE_INPUT);
    pulseSensor.setThreshold(THRESHOLD);


    // WIFI SETUP:
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println("Endereço IP: ");
    Serial.println(WiFi.localIP());


    // BOTAO INTERRUPTOR SETUP:
    pinMode(BOTAO_PINO, INPUT_PULLUP);
    // Configurar interrupção no pino
    attachInterrupt(digitalPinToInterrupt(BOTAO_PINO), trataInterrupcao, RISING);

}

// *** FIM DAS CONFIGURACOES ***

// *** LOOP PRINCIPAL ***

// Definir os períodos (ms) das tarefas
#define PERIODO_MPU 100
#define PERIODO_PULSE_SENSOR 200
#define PERIODO_WIFI 100

// Definir os tempos de execução (ms) das tarefas
unsigned long lastExecutedMPU = 0;
unsigned long lastExecutedPulseSensor = 0;
unsigned long lastExecutedWifi = 0;


// Variaveis de uso:
bool valFall;
unsigned int bpmPulse;
char packet[50];

void loop() {
    unsigned long agora = millis();
    
    // Executar tafefa do botao
    if (botaoPressionado) {
        handleWiFiConnection();
        sprintf(packet, "%d %d %d", bpmPulse, valFall, botaoPressionado);
        send_event(packet);
        Serial.println(packet);
        sprintf(packet, "");
        botaoPressionado = false;  // Reiniciar o estado do botao
    }


    // Executar tarefa do MPU
    if (agora - lastExecutedMPU >= PERIODO_MPU) {
        lastExecutedMPU = agora;
        valFall = detectFall();
    }

    // Executar tarefa do Pulse Sensor
    if (agora - lastExecutedPulseSensor >= PERIODO_PULSE_SENSOR) {
        lastExecutedPulseSensor = agora;
        bpmPulse = getBPM();
    }

    // Executar tarefa do WiFi
    if (agora - lastExecutedWifi >= PERIODO_WIFI) {
        lastExecutedWifi = agora;
        handleWiFiConnection();
        sprintf(packet, "%d %d %d", bpmPulse, valFall, botaoPressionado);
        send_event(packet);
        Serial.println(packet);
        sprintf(packet, "");
        botaoPressionado = false;  // Reiniciar o estado do botao
    }

}



