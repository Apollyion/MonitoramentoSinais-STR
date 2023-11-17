/*
   This example sketch is for use with the ESP32.
   The code below will serve a web page on the local network
   and will refresh the BPM with every heartbeat.
   On startup, the ESP32 will send it's network address
   over the serial port. Use that url in any browser
   on the same network to connect and veiw the webpage.
   
   Code to detect pulses from the PulseSensor,
   using an interrupt service routine.

   Here is a link to the tutorial\
   https://pulsesensor.com/pages/getting-advanced

   Copyright World Famous Electronics LLC - see LICENSE
   Contributors:
     Joel Murphy, https://pulsesensor.com
     Yury Gitman, https://pulsesensor.com
     Bradford Needham, @bneedhamia, https://bluepapertech.com

   Licensed under the MIT License, a copy of which
   should have been included with this software.

   This software is not intended for medical use.
*/


/*
   The following libraries are necessary
   for the asynchronous web server 
*/
#include <Arduino.h>

/*
   The following hardware timer setup supports ESP32
*/
hw_timer_t * sampleTimer = NULL;
portMUX_TYPE sampleTimerMux = portMUX_INITIALIZER_UNLOCKED;

/*
   Every Sketch that uses the PulseSensor Playground must
   define USE_ARDUINO_INTERRUPTS before including PulseSensorPlayground.h.
   Here, #define USE_ARDUINO_INTERRUPTS true tells the library to use
   interrupts to automatically read and process PulseSensor data.
*/
#define USE_ARDUINO_INTERRUPTS true
//#define NO_PULSE_SENSOR_SERIAL true
#include <PulseSensorPlayground.h>
PulseSensorPlayground pulseSensor;

/*
   Pinout:
     PULSE_INPUT = Analog Input. Connected to the pulse sensor
      purple (signal) wire.
     PULSE_BLINK = digital Output. Connected to an LED (and 1K series resistor)
      that will flash on each detected pulse.
     PULSE_FADE = digital Output. PWM pin onnected to an LED (and 1K series resistor)
      that will smoothly fade with each pulse.
      NOTE: PULSE_FADE must be a pin that supports PWM. Do not use
      pin 9 or 10, because those pins' PWM interferes with the sample timer.
     THRESHOLD should be set higher than the PulseSensor signal idles
      at when there is nothing touching it. The expected idle value
      should be 512, which is 1/2 of the ADC range. To check the idle value
      open a serial monitor and make note of the PulseSensor signal values
      with nothing touching the sensor. THRESHOLD should be a value higher
      than the range of idle noise by 25 to 50 or so. When the library
      is finding heartbeats, the value is adjusted based on the pulse signal
      waveform. THRESHOLD sets the default when there is no pulse present.
      Adjust as neccesary.
*/
const int PULSE_INPUT = 34;
//const int PULSE_BLINK = 2;    
//const int PULSE_FADE = 5;
const int THRESHOLD = 685;   

/*
    This is the interrupt service routine.
    We need to declare it after the PulseSensor Playground
    library is compiled, so that the onSampleTime
    function is known.
*/
void IRAM_ATTR onSampleTime() {
  portENTER_CRITICAL_ISR(&sampleTimerMux);
    PulseSensorPlayground::OurThis->onSampleTime();
  portEXIT_CRITICAL_ISR(&sampleTimerMux);
}

/* 
   When sendPulseSignal is true, PulseSensor Signal data
   is sent to the serial port for user monitoring.
   Modified by keys received on the Serial port.
   Use the Serial Plotter to view the PulseSensor Signal wave.
*/
//boolean sendPulseSignal = false;

void setup() {
/*
   115200 baud provides about 11 bytes per millisecond.
   The delay allows the port to settle so that 
   we don't miss out on info about the server url
   in the Serial Monitor so we can connect a browser.
*/
  Serial.begin(115200);
  delay(1500);
  
/*
   ESP32 analogRead defaults to 13 bit resolution
   PulseSensor Playground library works with 10 bit
*/
  analogReadResolution(10);
  
/*  Configure the PulseSensor manager  */
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.setSerial(Serial);
  pulseSensor.setThreshold(THRESHOLD);

/*  Now that everything is ready, start reading the PulseSensor signal. */
  if (!pulseSensor.begin()) {
    while(1) {
    }
  }
  sampleTimer = timerBegin(0, 80, true);                
  timerAttachInterrupt(sampleTimer, &onSampleTime, true);  
  timerAlarmWrite(sampleTimer, 2000, true);      
  timerAlarmEnable(sampleTimer);

}

void loop() {

  Serial.println(calculaBpm(1));
  delay(5000);
  Serial.println("------");
  
}


int calculaBpm(int amostras){
  int media = 0;
  for(int i = 0; i<amostras;){
  if (pulseSensor.sawStartOfBeat()) {
    media = pulseSensor.getBeatsPerMinute() + media;
    i++;
  }
  }
  return media/amostras;
}

/*
    This function checks to see if there are any commands available
    on the Serial port. When you send keyboard characters 'b' or 'x'
    you can turn on and off the signal data stream.
*/
