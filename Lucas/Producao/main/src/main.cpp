#include <Arduino.h>
#include <Wire.h>
#include <esp_system.h> // Certifique-se de incluir este cabeçalho
#include <esp_task_wdt.h> // Certifique-se de incluir este cabeçalho

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

void IRAM_ATTR onSampleTime() {
  portENTER_CRITICAL_ISR(&sampleTimerMux);
    PulseSensorPlayground::OurThis->onSampleTime();
  portEXIT_CRITICAL_ISR(&sampleTimerMux);
}
const int THRESHOLD = 530;   // TODO Ajustar o valor caso tenha ruído: 

void setupPulse(){
    analogReadResolution(10);    
    /*  Configure the PulseSensor manager  */
    pulseSensor.analogInput(35);
    pulseSensor.setSerial(Serial);
    pulseSensor.setThreshold(THRESHOLD);
}
// *****FIM DAS CONFIGURACOES DO PULSE SENSOR****


// ****INICIO CONFIGURACOES DO WIFI****
// DEFINICOES DO WIFI:
#include <WiFi.h>

// TODO - Alterar para o SSID e senha da rede
const char* ssid = "brisa-175976"; // SSID da rede WiFi  
const char* password = "ohmg6d06"; // Senha da rede WiFi
// TODO - Alterar para o IP e porta do servidor
const char* serverIP = "127.0.1.1"; // Endereço IP do servidor
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

// *** INICIO DAS CONFIGURACOES DO MAX ***

#include <MAX30105.h>  //MAX3010x library
#include "heartRate.h" //Heart rate calculating algorithm
MAX30105 particleSensor;

// Variables for heart rate calculation
const byte RATE_SIZE = 4; // What is the average quantity
byte rates[RATE_SIZE];    // heartbeat array
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

// Variables for blood oxygen calculation
double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;

double SpO2 = 0;
double ESpO2 = 90.0; // Initial value
double FSpO2 = 0.7;  // filter factor for estimated SpO2
double frate = 0.95; // low pass filter for IR/red LED value to eliminate AC component
int i = 0;
int Num = 30;             // Calculate once every 30 samples
#define FINGER_ON 7000    // Minimum infrared value (detecting finger presence)
#define MINIMUM_SPO2 90.0 // Minimum blood oxygen level

byte ledBrightness = 0x7F; // recommended =127, Options: 0=Off to 255=50mA
byte sampleAverage = 4;    // Options: 1, 2, 4, 8, 16, 32
byte ledMode = 2;          // Options: 1 = Red only(心跳), 2 = Red + IR(血氧)
int sampleRate = 800;      // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulseWidth = 215;      // Options: 69, 118, 215, 411
int adcRange = 16384;      // Options: 2048, 4096, 8192, 16384
int sendBeat, sendSPo2; // variavel que vai guardar o beatAvg e o SPo2

double getSpo2() // FUNCAO RETORNA O SPO2
{
  long irValue = particleSensor.getIR(); // Reading the IR value it will permit us to know if there's a finger on the sensor or not
  // Check if a finger is placed
  if (irValue > FINGER_ON)
  {

    // Check for a heartbeat and measure heart rate
    if (checkForBeat(irValue) == true)
    {
      sendBeat = beatAvg;
      // Serial.println(" BPM"); // Display heart rate value
      //  Display blood oxygen value
      if (beatAvg > 30)
      {
        return ESpO2;
      }
      else
        ESpO2 = 0;

      sendBeat = beatAvg;
      long delta = millis() - lastBeat; // Calculate heart rate difference
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0); // Calculate average heart rate
      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        // The heartbeat must be between 20-255
        rates[rateSpot++] = (byte)beatsPerMinute; // Array to store heartbeat values
        rateSpot %= RATE_SIZE;
        beatAvg = 0; // Calculate average
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

    // Medir o oxigênio no sangue
    uint32_t ir, red;
    double fred, fir;
    particleSensor.check(); // Check the sensor, read up to 3 samples
    if (particleSensor.available())
    {
      i++;
      ir = particleSensor.getFIFOIR();   // Ler infravermelho
      red = particleSensor.getFIFORed(); // Leia a luz vermelha
      // Serial.println("red=" + String(red) + ",IR=" + String(ir) + ",i=" + String(i));
      fir = (double)ir;                                      // Convert double
      fred = (double)red;                                    // Convert double
      aveir = aveir * frate + (double)ir * (1.0 - frate);    // average IR level by low pass filter
      avered = avered * frate + (double)red * (1.0 - frate); // average red level by low pass filter
      sumirrms += (fir - aveir) * (fir - aveir);             // square sum of alternate component of IR level
      sumredrms += (fred - avered) * (fred - avered);        // square sum of alternate component of red level

      if ((i % Num) == 0)
      {
        double R = (sqrt(sumirrms) / aveir) / (sqrt(sumredrms) / avered);
        SpO2 = -23.3 * (R - 0.4) + 100;
        ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2; // low pass filter
        if (ESpO2 <= MINIMUM_SPO2)
          ESpO2 = MINIMUM_SPO2; // indicator for finger detached
        if (ESpO2 > 100)
          ESpO2 = 99.9;
        // Serial.print(",SPO2="); Serial.println(ESpO2);
        sumredrms = 0.0;
        sumirrms = 0.0;
        SpO2 = 0;
        i = 0;
      }
      particleSensor.nextSample(); // We're finished with this sample so move to next sample
    }

    // exibir dados em serial
    sendBeat = beatAvg;
    // Exibir o valor do oxigênio no sangue para evitar medições incorretas. É necessário exibir o oxigênio no sangue quando o batimento cardíaco excede 30
    if (beatAvg > 30)
      sendBeat = beatAvg;
    else
      return ESpO2;
  }
  // Nenhum dedo detectado, limpe todos os dados e conteúdo da tela "Finger Please"
  else
  {
    // Limpar dados de batimentos cardíacos
    for (byte rx = 0; rx < RATE_SIZE; rx++)
      rates[rx] = 0;
    beatAvg = 0;
    rateSpot = 0;
    lastBeat = 0;
    // Limpar dados de oxigênio no sangue
    avered = 0;
    aveir = 0;
    sumirrms = 0;
    sumredrms = 0;
    SpO2 = 0;
    ESpO2 = 90.0;
    // Exibição Finger Please
    // Serial.println("Dedo não encontrado ");
  }
  return 1;
}

int getBeatAvg() // FUNCAO RETORNA O BeatAvg
{
  long irValue = particleSensor.getIR(); // Reading the IR value it will permit us to know if there's a finger on the sensor or not
  // Check if a finger is placed
  if (irValue > FINGER_ON)
  {

    // Check for a heartbeat and measure heart rate
    if (checkForBeat(irValue) == true)
    {
      // Serial.println(" BPM"); // Display heart rate value
      //  Display blood oxygen value

      sendBeat = beatAvg;
      long delta = millis() - lastBeat; // Calculate heart rate difference
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0); // Calculate average heart rate
      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        // The heartbeat must be between 20-255
        rates[rateSpot++] = (byte)beatsPerMinute; // Array to store heartbeat values
        rateSpot %= RATE_SIZE;
        beatAvg = 0; // Calculate average
        for (byte x = 0; x < RATE_SIZE; x++){
          Serial.println(rates[x]);
          beatAvg += rates[x];
        }

        beatAvg /= RATE_SIZE;
      }
    }

    // Medir o oxigênio no sangue
    uint32_t ir, red;
    double fred, fir;
    particleSensor.check(); // Check the sensor, read up to 3 samples
    if (particleSensor.available())
    {
      i++;
      ir = particleSensor.getFIFOIR();   // Ler infravermelho
      red = particleSensor.getFIFORed(); // Leia a luz vermelha
      // Serial.println("red=" + String(red) + ",IR=" + String(ir) + ",i=" + String(i));
      fir = (double)ir;                                      // Convert double
      fred = (double)red;                                    // Convert double
      aveir = aveir * frate + (double)ir * (1.0 - frate);    // average IR level by low pass filter
      avered = avered * frate + (double)red * (1.0 - frate); // average red level by low pass filter
      sumirrms += (fir - aveir) * (fir - aveir);             // square sum of alternate component of IR level
      sumredrms += (fred - avered) * (fred - avered);        // square sum of alternate component of red level

      if ((i % Num) == 0)
      {
        double R = (sqrt(sumirrms) / aveir) / (sqrt(sumredrms) / avered);
        SpO2 = -23.3 * (R - 0.4) + 100;
        ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2; // low pass filter
        if (ESpO2 <= MINIMUM_SPO2)
          ESpO2 = MINIMUM_SPO2; // indicator for finger detached
        if (ESpO2 > 100)
          ESpO2 = 99.9;
        // Serial.print(",SPO2="); Serial.println(ESpO2);
        sumredrms = 0.0;
        sumirrms = 0.0;
        SpO2 = 0;
        i = 0;
      }
      particleSensor.nextSample(); // We're finished with this sample so move to next sample
    }

    if (beatAvg > 30)
      return beatAvg;
    // Nenhum dedo detectado, limpe todos os dados
    else
    {
      // Limpar dados de batimentos cardíacos
      for (byte rx = 0; rx < RATE_SIZE; rx++)
        rates[rx] = 0;
      beatAvg = 0;
      rateSpot = 0;
      lastBeat = 0;
      // Limpar dados de oxigênio no sangue
      avered = 0;
      aveir = 0;
      sumirrms = 0;
      sumredrms = 0;
      SpO2 = 0;
      ESpO2 = 90.0;
    }
  }
  return 1;
}


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

// *** INICIO DAS CONFIGURACOES ***

#define SDA 13
#define SCL 14
hw_timer_t *timer = NULL; //faz o controle do temporizador (interrupção por tempo)


void setup() {
    Serial.begin(115200);

  // WATCHDOG SETUP:
    // Print the CPU frequency to the Serial Monitor
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");

  // Começa o watchdog com 3 segundos de timeout

  timer = timerBegin(1, ESP.getCpuFreqMHz(), true); //timerID 1, div cpuFreqMHz, contagem ascendente
  //timer, callback, interrupção de borda
  timerAttachInterrupt(timer, &resetModule, true);
  //timer, tempo (us), repetição
  timerAlarmWrite(timer, 5000000, true); //3s
  timerAlarmEnable(timer); //habilita a interrupção 





    // MPU setup
    // Wire.begin();
    // Wire.beginTransmission(MPU_addr);
    // Wire.write(0x6B);
    Wire.begin(SDA,SCL);
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    Wire.write(0);
    Wire.endTransmission(true);



    // Pulse Sensor setup
    setupPulse();
    sampleTimer = timerBegin(0, ESP.getCpuFreqMHz(), true);                
    timerAttachInterrupt(sampleTimer, &onSampleTime, true);  
    timerAlarmWrite(sampleTimer, 480, true); // TODO: Aumentar o valor do tempo do PulseSensor
    timerAlarmEnable(sampleTimer);



    // analogReadResolution(10);
    // pulseSensor.analogInput(PULSE_INPUT);
    // pulseSensor.setThreshold(THRESHOLD);


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
    pinMode(BOTAO_PINO, PULLUP);
    // Configurar interrupção no pino
    attachInterrupt(digitalPinToInterrupt(BOTAO_PINO), trataInterrupcao, FALLING);

    //TODO: Remover comentario MAX
    // // MAX30102 SETUP:
    // // Check if the MAX30102 sensor is available
    // if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
    // {
    //     Serial.println("MAX30102 não encontrado"); // sensor not found
    //     while (1)
    //         ;
    // }
    // particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor with these settings
    // particleSensor.enableDIETEMPRDY();

    // particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
    // particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED

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
double spo2;
unsigned int avgBPM;


bool maxExecuted = false;
bool mpuExecuted = false;
bool pulseSensorExecuted = false;
bool cicloComplete = false;


void loop() {
    unsigned long agora = millis();

    //Reinicia o watchdog
    timerWrite(timer, 1); //reseta o timer
  

    //Serial.printf("\nBOTAO: %d\n", digitalRead(21)); TODO: Remover Linha
    /*

    //TODO: Remover bloco de codigo
    // Executar tafefa do botao
    if (botaoPressionado) {
        handleWiFiConnection();
        avgBPM = (bpmPulse + maxPulse) / 2;
        sprintf(packet, "%d %f %d %d", avgBPM, spo2, valFall, botaoPressionado);
        send_event(packet);
        Serial.println(packet);
        sprintf(packet, "");
        botaoPressionado = false;  // Reiniciar o estado do botao
    }
    */


    //TODO: Remover Comentario MAX
    // // Executar tarefa do MAX
    // if (agora - lastExecutedMax >= PERIODO_MAX && !maxExecuted) {
    //     lastExecutedMax = agora;
    //     maxPulse = getBeatAvg();
    //     spo2 = getSpo2();
    // }

    // Executar tarefa do MPU
    if (agora - lastExecutedMPU >= PERIODO_MPU && !mpuExecuted) {
        lastExecutedMPU = agora;
        valFall = detectFall();
    }

    // Executar tarefa do Pulse Sensor
    if (agora - lastExecutedPulseSensor >= PERIODO_PULSE_SENSOR && !pulseSensorExecuted) {
        lastExecutedPulseSensor = agora;
        if (pulseSensor.sawStartOfBeat()) {
          bpmPulse = pulseSensor.getBeatsPerMinute();
        }
        Serial.printf("BPM: %d\n", bpmPulse);  // Exibir o BPM
        Serial.printf("Analog: %d\n", pulseSensor.getLatestSample());  // Exibir o Analog
    }

    // Executar tarefa do WiFi
    if (agora - lastExecutedWifi >= PERIODO_WIFI) {
        lastExecutedWifi = agora;
        handleWiFiConnection();
        
        avgBPM = (bpmPulse + bpmPulse) / 2; //TODO: Trocar um dos bpmPulse por maxPulse

        //TODO: Remover comentarios
        //sprintf(packet, "%d %f %d %d", avgBPM, spo2, valFall, botaoPressionado);
        //send_event(packet);
        //Serial.println(packet);
        sprintf(packet, "");
        botaoPressionado = false;  // Reiniciar o estado do botao
        maxExecuted = false;
        mpuExecuted = false;
        pulseSensorExecuted = false;
    }

}


