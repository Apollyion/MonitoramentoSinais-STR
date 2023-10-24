#include <Arduino.h>
#include <Wire.h>

// ***INICIO CONFIGURACOES DO MPU*** 
// DEFINICOES DO MPU
#include <MPU6050_tockn.h>
MPU6050 mpu6050(Wire); 
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
    ax = (AcX)/16384.00;
    ay = (AcY)/16384.00;
    az = (AcZ)/16384.00;
    gx = (GyX)/131.0;
    gy = (GyY)/131.0;
    gz = (GyZ)/131.0;
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

// }****FIM DAS CONFIGURACOES DO MPU***


// ***INICIO CONFIGURACOES DO PULSE SENSOR***
hw_timer_t * sampleTimer = NULL;
portMUX_TYPE sampleTimerMux = portMUX_INITIALIZER_UNLOCKED;
#define USE_ARDUINO_INTERRUPTS true
//#define NO_PULSE_SENSOR_SERIAL true
#include <PulseSensorPlayground.h>
PulseSensorPlayground pulseSensor;

void IRAM_ATTR onSampleTime() {
  portENTER_CRITICAL_ISR(&sampleTimerMux);
    PulseSensorPlayground::OurThis->onSampleTime();
  portEXIT_CRITICAL_ISR(&sampleTimerMux);
}
const int THRESHOLD = 685;   // TODO Ajustar o valor caso tenha ruído

void setupPulse(){
    analogReadResolution(10);    
    /*  Configure the PulseSensor manager  */
    pulseSensor.analogInput(35);
    pulseSensor.setSerial(Serial);
    pulseSensor.setThreshold(THRESHOLD);
}


//-----------------------------------------------------------------

// #define USE_ARDUINO_INTERRUPTS false
// #include <PulseSensorPlayground.h>

// const int OUTPUT_TYPE = SERIAL_PLOTTER;
// const int PULSE_INPUT = 35;
// const int THRESHOLD = 2300;
// PulseSensorPlayground pulseSensor;
// const byte SAMPLES_PER_SERIAL_SAMPLE = 1;
// byte samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;


// int getBpmSample(){
//     if (pulseSensor.sawNewSample()) {
//         int latest = pulseSensor.getBeatsPerMinute();
//         pulseSensor.getLatestSample();
//         return latest;
//     }

// }

//--------------------------------------------------------------------------



// // DEFINICOES DO PULSE SENSOR:
// hw_timer_t * sampleTimer = NULL;
// portMUX_TYPE sampleTimerMux = portMUX_INITIALIZER_UNLOCKED;
// #define USE_ARDUINO_INTERRUPTS true
// //#define NO_PULSE_SENSOR_SERIAL true

// #include <PulseSensorPlayground.h>
// PulseSensorPlayground pulseSensor;


// const int PULSE_INPUT = 35; // Pino de entrada do sensor
// const int PULSE_BLINK = 13; // FIXME - Não está sendo usado
// const int PULSE_FADE = 5; // FIXME - Não está sendo usado
// const int THRESHOLD = 685;   // TODO Ajustar o valor caso tenha ruído


// // Pulse Sensor Read function
// int getBPM() {
//   if (pulseSensor.sawStartOfBeat()) {
//     return pulseSensor.getBeatsPerMinute();
//   } else {
//     return 0;
//   }
// }

// ****FIM DAS CONFIGURACOES DO PULSE SENSOR***


// ***INICIO CONFIGURACOES DO WIFI***
// DEFINICOES DO WIFI:
#include <WiFi.h>

// TODO - Alterar para o SSID e senha da rede
const char* ssid = "AcerAmigo"; // SSID da rede WiFi
const char* password = "senhafacil"; // Senha da rede WiFi
// TODO - Alterar para o IP e porta do servidor
const char* serverIP = "192.168.0.14"; // Endereço IP do servidor
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



// ****FIM DAS CONFIGURACOES DO WIFI***

// ***** CONFIGURACAO DO BOTAO INTERRUPTOR *****
// DEFINICOES DO BOTAO INTERRUPTOR:
#define BOTAO_PINO 21 // Define o pino do botão
volatile bool botaoPressionado = false; // Variável que será alterada pela interrupção

// Função de tratamento de interrupção
void IRAM_ATTR trataInterrupcao() {
  botaoPressionado = true;
}

// ***** FIM DAS CONFIGURACOES DO BOTAO INTERRUPTOR *****


#define SDA 13
#define SCL 14

void setup() {
    Serial.begin(115200);

    // MPU setup

    Wire.begin(SDA,SCL);
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    Wire.write(0);
    Wire.endTransmission(true);


    // PULSE SENSOR SETUP:
    setupPulse();
    sampleTimer = timerBegin(0, 80, true);                
    timerAttachInterrupt(sampleTimer, &onSampleTime, true);  
    timerAlarmWrite(sampleTimer, 2000, true);      
    timerAlarmEnable(sampleTimer);
    // pulseSensor.analogInput(PULSE_INPUT);
    // pulseSensor.setSerial(Serial);
    // pulseSensor.setOutputType(OUTPUT_TYPE);
    // pulseSensor.setThreshold(THRESHOLD);


    // WIFI SETUP:
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { //XTODO: Remover comentario
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println("Endereço IP: ");
    Serial.println(WiFi.localIP());

    // BOTAO INTERRUPTOR SETUP:
    pinMode(BOTAO_PINO,PULLUP);
    // Configurar interrupção no pino
    attachInterrupt(digitalPinToInterrupt(BOTAO_PINO), trataInterrupcao, FALLING);

}

// *** FIM DAS CONFIGURACOES ***

// *** LOOP PRINCIPAL ***

// Definir os períodos (ms) das tarefas
#define PERIODO_MPU 100
#define PERIODO_PULSE_SENSOR 200
#define PERIODO_WIFI 500

// Definir os tempos de execução (ms) das tarefas
unsigned long lastExecutedMPU = 0;
unsigned long lastExecutedPulseSensor = 0;
unsigned long lastExecutedWifi = 0;


// Variaveis de uso:
bool valFall;
unsigned int bpmPulse;
char packet[50];

bool mpuExecuted = false;
bool pulseSensorExecuted = false;
bool cicloComplete = false;

void loop() {
    unsigned long agora = millis();
    
    // Executar tafefa do botao
    if (botaoPressionado) {
        handleWiFiConnection(); // XTODO: Remover Comentario
        sprintf(packet, "%d %d %d", bpmPulse, valFall, botaoPressionado);
        send_event(packet);
        //Serial.println(packet);
        sprintf(packet, "");
        botaoPressionado = false;  // Reiniciar o estado do botao
    }


    // Executar tarefa do MPU
    if (agora - lastExecutedMPU >= PERIODO_MPU && !cicloComplete) {
        lastExecutedMPU = agora;
        valFall = detectFall();
        mpuExecuted = true;
    }

    // Executar tarefa do Pulse Sensor
    if (agora - lastExecutedPulseSensor >= PERIODO_PULSE_SENSOR && !pulseSensorExecuted) {
        lastExecutedPulseSensor = agora;
        if (pulseSensor.sawStartOfBeat()) {
            bpmPulse = pulseSensor.getBeatsPerMinute();
        }
    }
    

    // Executar tarefa do WiFi
    if (agora - lastExecutedWifi >= PERIODO_WIFI) {
        lastExecutedWifi = agora;
        handleWiFiConnection(); //XTODO - Remover comentario
        sprintf(packet, "%d %d %d", bpmPulse, valFall, botaoPressionado);
        send_event(packet);
        //Serial.println(packet);
        sprintf(packet, "");
        botaoPressionado = false;  // Reiniciar o estado do botao
        mpuExecuted, pulseSensorExecuted = false;
    }

}