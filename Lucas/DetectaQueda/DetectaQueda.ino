/**********************************************************************
  Filename    : Acceleration detection
  Description : Read the MPU6050 data and print it out through the serial port
  Auther      : www.freenove.com
  Modification: 2020/07/11
**********************************************************************/
#include <MPU6050_tockn.h>
#include <Wire.h>

#define SDA 13
#define SCL 14

MPU6050 mpu6050(Wire);//Attach the IIC
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;

boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;

long timer = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);          //attach the IIC pin
  mpu6050.begin();               //initialize the MPU6050
  mpu6050.calcGyroOffsets(true); //get the offsets value
}
// Void loop function - running continuously 
void loop()
{
    mpu6050.update();            //update the MPU6050
    getMotion6();                //gain the values of Acceleration and Gyroscope value
    ax = (AcX-2050)/16384.00;
    ay = (AcY-77)/16384.00;
    az = (AcZ-1947)/16384.00;
    gx = (GyX+270)/131.07;
    gy = (GyY-351)/131.07;
    gz = (GyZ+136)/131.07;

    // calculating Amplitute vactor for 3 axis
    float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
    int Amp = Raw_Amp * 10;  // Multiplied by 10 to values are between 0 to 1
    Serial.println(Amp);
    if (Amp<=2 && trigger2==false) //if AM breaks lower threshold (0.4g)
    { 
        trigger1=true;
        Serial.println("TRIGGER 1 ACTIVATED");
    }
    if (trigger1==true)
    {
        trigger1count++;
        if (Amp>=12) //if AM breaks upper threshold (3g)
        { 
            trigger2=true;
            Serial.println("TRIGGER 2 ACTIVATED");
            trigger1=false; trigger1count=0;
        }
    }
    // Trigger function 
    if (trigger2==true)
    {
        trigger2count++;
        angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); Serial.println(angleChange);
        if (angleChange>=30 && angleChange<=400) //if orientation changes by between 80-100 degrees
        { 
            trigger3=true; trigger2=false; trigger2count=0;
            Serial.println(angleChange);
            Serial.println("TRIGGER 3 ACTIVATED");
        }
    }
    if (trigger3==true)
    {
        trigger3count++;
        if (trigger3count>=10)
        { 
            angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
            //delay(10);
            Serial.println(angleChange); 
            if ((angleChange>=0) && (angleChange<=10)) //if orientation changes remains between 0-10 degrees
            { 
                fall=true; trigger3=false; trigger3count=0;
                Serial.println(angleChange);
            }
         else  //user regained normal orientation
         {
             trigger3=false; trigger3count=0;
            Serial.println("TRIGGER 3 DEACTIVATED");
         }
        }
    }
    if (fall==true) //in event of a fall detection
    { 
        Serial.println("FALL DETECTED using MPU sensor");
        //send_event("Webhooks Event Name Sending"); 
        fall=false;
    }
    if (trigger2count>=6) //allow 0.5s for orientation change
    { 
        trigger2=false; trigger2count=0;
        Serial.println("TRIGGER 2 DECACTIVATED");
    }
    if (trigger1count>=6) //allow 0.5s for AM to break upper threshold
    { 
        trigger1=false; trigger1count=0;
        Serial.println("TRIGGER 1 DECACTIVATED");
    }
    delay(100);
}

void getMotion6(void){
  AcX=mpu6050.getRawAccX();//gain the values of X axis acceleration raw data
  AcY=mpu6050.getRawAccY();//gain the values of Y axis acceleration raw data
  AcZ=mpu6050.getRawAccZ();//gain the values of Z axis acceleration raw data
  GyX=mpu6050.getRawGyroX();//gain the values of X axis Gyroscope raw data
  GyY=mpu6050.getRawGyroX();//gain the values of Y axis Gyroscope raw data
  GyZ=mpu6050.getRawGyroX();//gain the values of Z axis Gyroscope raw data
  
}
