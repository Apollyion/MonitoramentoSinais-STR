#include <Arduino.h>
#include <Wire.h>
#include <esp_system.h> // Certifique-se de incluir este cabeçalho
#include <esp_task_wdt.h> // Certifique-se de incluir este cabeçalho
#include "MAX30105.h"       // Core functions of the
#include "spo2_algorithm.h" // Header file for SpO2 calculation
#include "heartRate.h"      // Header file for Heart rate calculation

// variavel para usar de ponteiro
int bpm;
int spo2Value;
int bpmatual;

/* Objects & Variables */
MAX30105 pox; // Pulse Oximeter Objects
String lcdMessage;
// Variables for SpO2
uint32_t irBuffer[100];           // infrared LED sensor data
uint32_t redBuffer[100];          // red LED sensor data
const int32_t bufferLength = 100; // buffer length of 100 stores 4 seconds of samples running at 25sps
int32_t spo2;                     // SPO2 value
int8_t validSPO2;                 // indicator to show if the SPO2 calculation is valid
int32_t heartRate;                // heart rate value
int8_t validHeartRate;            // indicator to show if the heart rate calculation is valid
// Variables for Heart Rate
int setSize = 25;
const byte RATE_SIZE = 2; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];    // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
bool notifyBPM = false;

/* Custom Functions */
void getHeartRate(int *pontbeat)
{
  unsigned long int tempounix = millis();
  unsigned long int tempoLimite = 3000;  // Defina o limite de tempo em milissegundos
  for (int cont18 = 0; cont18 < 18; cont18++)
  {
    for (int i = 0; i < setSize; i++)
    {
      long irValue = pox.getIR();
      if (checkForBeat(irValue))
      {
        // We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20)
        {
          rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
          rateSpot %= RATE_SIZE;                    // Wrap variable

          // Take average of readings
          beatAvg = 0;
          for (byte x = 0; x < RATE_SIZE; x++)
            beatAvg += rates[x];
          beatAvg /= RATE_SIZE;
        }
        /* Debugging */
        Serial.print("IR=");
        Serial.print(irValue);
        Serial.print("\tBPM=");
        Serial.print(beatsPerMinute);
        Serial.print("\tAvg BPM=");
        Serial.println(beatAvg); /* */
        *pontbeat = beatAvg;
        bpmatual = beatsPerMinute;
      }
      if (irValue < 50000)
      {
        //Serial.println("No finger?");
        *pontbeat = -1;
      }
  unsigned long int agora = millis();
  if(agora - tempounix > tempoLimite)
    break;    
    }
  //if(beatAvg > 40 && beatAvg != bpmatual)
  //  break;  
  }
}

void getSpO2(int *pontspo2)
{

  // After gathering 25 new samples, recalculate HR and SpO2
  uint16_t irBufferUInt16[bufferLength];
  uint16_t redBufferUInt16[bufferLength];

  // Convert os buffers para uint16_t
  for (int i = 0; i < bufferLength; i++)
  {
    irBufferUInt16[i] = (uint16_t)irBuffer[i];
    redBufferUInt16[i] = (uint16_t)redBuffer[i];
  }

  // dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
  for (byte i = 25; i < 100; i++)
  {
    redBuffer[i - 25] = redBuffer[i];
    irBuffer[i - 25] = irBuffer[i];
  }

  // take 25 sets of samples before calculating the heart rate.
  for (byte i = 75; i < 100; i++)
  {
    while (pox.available() == false) // do we have new data?
      pox.check();                   // Check the sensor for new data

    redBuffer[i] = pox.getRed();
    irBuffer[i] = pox.getIR();
    pox.nextSample(); // We're finished with this sample so move to next sample
  }
  // After gathering 25 new samples recalculate HR and SP02
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer,
                                         &spo2, &validSPO2, &heartRate, &validHeartRate);

  // send samples and calculation result to terminal program through UART
  Serial.print(F("HR="));
  Serial.print(heartRate, DEC);

  Serial.print(F("\tHRvalid="));
  Serial.print(validHeartRate, DEC);

  Serial.print(F("\tSPO2="));
  Serial.print(spo2, DEC);

  Serial.print(F("\tSPO2Valid="));
  Serial.println(validSPO2, DEC);
  *pontspo2 = spo2;
}


// ****INICIO CONFIGURACOES DO MPU**** 
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

// }*****FIM DAS CONFIGURACOES DO MPU****


// ****INICIO CONFIGURACOES DO PULSE SENSOR****
hw_timer_t * sampleTimer = NULL;
portMUX_TYPE sampleTimerMux = portMUX_INITIALIZER_UNLOCKED;

#define USE_ARDUINO_INTERRUPTS true
//#define NO_PULSE_SENSOR_SERIAL true
#include <PulseSensorPlayground.h>
PulseSensorPlayground pulseSensor;
const int PULSE_INPUT = 34;
//const int PULSE_BLINK = 2;    
//const int PULSE_FADE = 5;
const int THRESHOLD = 685;   
#include <math.h>

void IRAM_ATTR onSampleTime() {
  portENTER_CRITICAL_ISR(&sampleTimerMux);
    PulseSensorPlayground::OurThis->onSampleTime();
  portEXIT_CRITICAL_ISR(&sampleTimerMux);
}

int calculaBpm(int amostras) {
  int media = 0;
  unsigned long int tempounix = millis();
  unsigned long int tempoLimite = 3000;  // Defina o limite de tempo em milissegundos
  int i;

  for (i = 0; i < amostras;) {
    unsigned long int agora = millis();
    if (pulseSensor.sawStartOfBeat()) {
      media += pulseSensor.getBeatsPerMinute();
      i++;
      
      if (agora - tempounix > tempoLimite) {
        return media / i;
      }
    } else if (agora - tempounix > tempoLimite)
    {
      return media / i;
    }
    
  }

  // Se o loop terminar sem atingir o número de amostras desejado, ainda retornamos a média parcial
  return media / i;
}
// *****FIM DAS CONFIGURACOES DO PULSE SENSOR****


// ****INICIO CONFIGURACOES DO WIFI****
// DEFINICOES DO WIFI:
#include <WiFi.h>

// TODO - Alterar para o SSID e senha da rede
const char* ssid = "brisa-175976"; // SSID da rede WiFi  
const char* password = "ohmg6d06"; // Senha da rede WiFi
// TODO - Alterar para o IP e porta do servidor
const char* serverIP = "192.168.0.10"; // Endereço IP do servidor
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
#define BOTAO_PINO 21 // Define o pino do botão
volatile bool botaoPressionado = false; // Variável que será alterada pela interrupção

// Função de tratamento de interrupção
void IRAM_ATTR trataInterrupcao() {
  botaoPressionado = true;
}

// ***** FIM DAS CONFIGURACOES DO BOTAO INTERRUPTOR *****






// Configurandio WATCHDOG 

//função que o temporizador irá chamar, para reiniciar o ESP32
void IRAM_ATTR resetModule(){
    ets_printf("(watchdog) reiniciar\n"); //imprime no log
    // Blink LED 3 times to indicate reset (GPIO 2)
    pinMode(2, OUTPUT);
    for(int i = 0; i < 3; i++){
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }
    esp_restart(); //reinicia o chip
}





// *** FUNÇÕES AUXILIARES PARA O SETUP ***

void ledEntrouSetup(){ // função para piscar o led 2 vezes quando ligar
  pinMode(2, OUTPUT);
  for(int i = 0; i < 2; i++){
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }
}

void ledSaiuSetup(){ // função para piscar o led 3 vezes quando sair do setup
  pinMode(2, OUTPUT);
  for(int i = 0; i < 3; i++){
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }
}
// *** FIM DAS FUNÇÕES PARA O SETUP ***


// *** INICIO DAS CONFIGURACOES ***

#define SDA 13
#define SCL 14
hw_timer_t *timer = NULL; //faz o controle do temporizador (interrupção por tempo)


void setup() {
    Serial.begin(115200);
    delay(1500);
    ledEntrouSetup();

  // WATCHDOG SETUP:
    // Print the CPU frequency to the Serial Monitor
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");

  // // Começa o watchdog com 3 segundos de timeout
  //   timer = timerBegin(1, 80, true); //timerID 1, div cpuFreqMHz, contagem ascendente
  //   //timer, callback, interrupção de borda
  //   timerAttachInterrupt(timer, &resetModule, true);
  //   //timer, tempo (us), repetição
  //   timerAlarmWrite(timer, 20000000L, true); //2s
  //   timerAlarmEnable(timer); //habilita a interrupção 

    Wire.begin(SDA,SCL);
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    Wire.write(0);
    Wire.endTransmission(true);

    // timerWrite(timer, 1); //reseta o timer




    // Pulse Sensor setup
    analogReadResolution(10);
    /*  Configure the PulseSensor manager  */
    pulseSensor.analogInput(PULSE_INPUT);
    pulseSensor.setSerial(Serial);
    pulseSensor.setThreshold(THRESHOLD);
    if (!pulseSensor.begin()) {
      while(1) {
      }
    }
    sampleTimer = timerBegin(0, 80, true);                
    timerAttachInterrupt(sampleTimer, &onSampleTime, true);  
    timerAlarmWrite(sampleTimer, 2000, true);      
    timerAlarmEnable(sampleTimer);

    // timerWrite(timer, 1); //reseta o timer




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

    // timerWrite(timer, 1); //reseta o timer

    // BOTAO INTERRUPTOR SETUP:
    pinMode(BOTAO_PINO, PULLUP);
    // Configurar interrupção no pino
    attachInterrupt(digitalPinToInterrupt(BOTAO_PINO), trataInterrupcao, FALLING);
    // timerWrite(timer, 1); //reseta o timer

    // MAX SETUP
// MAX
    Serial.println("Initializing...");
    // Initialize sensor
    // Verifique se o sensor MAX30102 está disponível
    if (!pox.begin(Wire, I2C_SPEED_FAST))
    {
      Serial.println("MAX30102 Sensor was not found. Please check wiring/power.\nTry pressing the Reset key");
    }
    else
    {
      /* Configure the sensor */
      byte ledBrightness = 60; // Options: 0=Off to 255=50mA
      byte sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, 32
      byte ledMode = 2;        // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
      byte sampleRate = 100;   // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
      int pulseWidth = 411;    // Options: 69, 118, 215, 411
      int adcRange = 4096;     // Options: 2048, 4096, 8192, 16384
      pox.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
    }
    /* Initial Collection of Data */
    Serial.println("Collecting Data...");
    // read the first 100 samples, and determine the signal range
    for (byte i = 0; i < bufferLength; i++)
    {
      while (pox.available() == false) // do we have new data?
        pox.check();                   // Check the sensor for new data

      redBuffer[i] = pox.getRed();
      irBuffer[i] = pox.getIR();
      pox.nextSample(); // We're finished with this sample so move to next sample
      // Serial.print(F("red="));
      // Serial.print(redBuffer[i], DEC);
      // Serial.print(F(", ir="));
      // Serial.println(irBuffer[i], DEC);            
    }
    Serial.println("Data Collected!");
    // timerWrite(timer, 1); //reseta o timer

    // Saiu do Setup
    ledSaiuSetup();

}







// *** FIM DAS CONFIGURACOES ***

// *** LOOP PRINCIPAL ***

// Definir os períodos (ms) das tarefas
#define PERIODO_MPU 100
#define PERIODO_PULSE_SENSOR 200
#define PERIODO_WIFI 500
#define PERIODO_MAX 20


// Definir os tempos de execução (ms) das tarefas
unsigned long lastExecutedMPU = 0;
unsigned long lastExecutedPulseSensor = 0;
unsigned long lastExecutedWifi = 0;
unsigned long lastExecutedMax = 0;


// Variaveis de uso:
bool valFall;
unsigned int bpmPulse;
char packet[50];
unsigned int maxPulse;
unsigned int avgBPM;


bool maxExecuted = false;
bool mpuExecuted = false;
bool pulseSensorExecuted = false;
bool cicloComplete = false;



// AMOSTRAS DO PULSESENSOR
#define AMOSTRAS 5


void loop() {
    unsigned long agora = millis();

    //Reinicia o watchdog
    // timerWrite(timer, 1); //reseta o timer


    //TODO: Remover Comentario MAX
    // // Executar tarefa do MAX
     if (agora - lastExecutedMax >= PERIODO_MAX && !maxExecuted) {
        lastExecutedMax = agora;
        getHeartRate(&bpm);
        // bpm = getHeartRate();
        getSpO2(&spo2Value);
        // Serial.print("TESTE ANTES DO SPRINT bpm: ");
        // Serial.println(bpm);
        // Serial.print("TESTE ANTES DO SPRINT Spo2: ");
        // Serial.println(spo2Value);
     }
    // timerWrite(timer, 1); //reseta o timer

    // Executar tarefa do MPU
    if (agora - lastExecutedMPU >= PERIODO_MPU && !mpuExecuted) {
        lastExecutedMPU = agora;
        valFall = detectFall();
    }
    // timerWrite(timer, 1); //reseta o timer

    // Executar tarefa do Pulse Sensor
    if (agora - lastExecutedPulseSensor >= PERIODO_PULSE_SENSOR && !pulseSensorExecuted) {
        lastExecutedPulseSensor = agora;
        bpmPulse = calculaBpm(AMOSTRAS);  // Exibir o Analog
        Serial.printf("BPM PULSE: %d\n", bpmPulse);
    }
    // timerWrite(timer, 1); //reseta o timer

    // Executar tarefa do WiFi
    if (agora - lastExecutedWifi >= PERIODO_WIFI) {
        lastExecutedWifi = agora;
        handleWiFiConnection();
        
        avgBPM = (bpmPulse + bpm) / 2; //TODO: Trocar um dos bpmPulse por maxPulse

        //TODO: Remover comentarios
        sprintf(packet, "%d %d %d %d\n", avgBPM, spo2Value, valFall, botaoPressionado);
        send_event(packet);
        Serial.println(packet);
        sprintf(packet, "");
        botaoPressionado = false;  // Reiniciar o estado do botao
        maxExecuted = false;
        mpuExecuted = false;
        pulseSensorExecuted = false;
    }

}

