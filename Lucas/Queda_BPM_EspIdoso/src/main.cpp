#include <Arduino.h>
#include <Wire.h>

// Setting up constants values
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

// Rest of the code remains the same

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
}

void loop() {
    if (detectFall()) {
        Serial.println("FALL DETECTED using MPU sensor");
        // send_event("Webhooks Event Name Sending"); 
    }
}