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
// Set up the wanted parameters

int sendBeat, sendSPo2; // variavel que vai guardar o beatAvg e o SPo2

void setup()
{

  // Check if the MAX30102 sensor is available
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
  {
    // Serial.println("MAX30102 não encontrado"); // sensor not found
    while (1)
      ;
  }
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor with these settings
  particleSensor.enableDIETEMPRDY();

  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED
}

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

void loop()
{
  int B = getBeatAvg();
  double S = getSpo2();
}
