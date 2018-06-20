/*
Autor Alex Krieg
Datum 20.06.2018
Version 0.1.1
*/

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include "I2Cdev.h"
#include "MPU6050.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
#include "button.h"
#include "Timer.h"
#include "IR_sensor.h"

#define NUMPIXELS 16
#define PIXELPIN 2

//#define DEBUG

struct accel
{
  int16_t x;
  int16_t y;
  int16_t z;
};
struct color
{
  byte red;
  byte green;
  byte blue;
};
enum modus
{
  modus_none = 0,
  modus_reflex = 1
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);
MPU6050 accelgyro;
IR_sensor ir_sensor(3,A0); 
Timer sensor_UpdateIntervalTimer;
Timer irSensorCooldownTimer;
Timer pixelsUpdateTimer;
Timer wifiServerUpdateTimer;

//---------------------------MODE
Timer mode_reflex_Timer;
unsigned int mode_reflex_time = 0;
//------------------------------

const float accelerationTrigger = 1.0;
float accelerationAverage = 0;

accel acceleration;
color pixels_color;
bool  pixels_enable = false;
modus mode = modus_none;
modus lastMode = modus_none;

void readSerial();
void wifiUpdate();
void sensorUpdateFunction();
void triggerFunction();
void updateRGB();
void IR_highTriggerFunction();
void irSensorCooldownTimerFinishedFunction();
void handleMode();

void MODE(modus m);
modus MODE();
modus LASTMODE();


void setup()
{
  Serial.begin(115200);
  accelgyro.initialize();
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  pixels.begin(); // This initializes the NeoPixel library.
  #ifdef DEBUG
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  #endif
    
  sensor_UpdateIntervalTimer.onFinished(sensorUpdateFunction);
  sensor_UpdateIntervalTimer.autoRestart(true);
  sensor_UpdateIntervalTimer.start(50);
  irSensorCooldownTimer.onFinished(irSensorCooldownTimerFinishedFunction);
  pixelsUpdateTimer.onFinished(updateRGB);
  pixelsUpdateTimer.autoRestart(true);
  pixelsUpdateTimer.start(100);
  wifiServerUpdateTimer.onFinished(wifiUpdate);
  wifiServerUpdateTimer.autoRestart(false);
  wifiServerUpdateTimer.start(3000);
  
  ir_sensor.onHighTrigger(IR_highTriggerFunction);

  pixels_color.red = 10;
  pixels_color.green = 10;
  pixels_color.blue = 10;
}

void loop()
{
  pixelsUpdateTimer.update();
  irSensorCooldownTimer.update();
  sensor_UpdateIntervalTimer.update();
  wifiServerUpdateTimer.update();
  handleMode();
  readSerial();
}
void wifiUpdate()
{
  Serial.print("color]");
  //-------------einzeltest
  pixels_color.red = random(0,100);
  pixels_color.green = random(0,100);
  pixels_color.blue = random(0,100);
  MODE(modus_reflex);
  //----------------------
  wifiServerUpdateTimer.start(3000);
}

void readSerial()
{
  if(Serial.available())
  {
    String inputBuffer = Serial.readStringUntil(']');
    #ifdef DEBUG
      Serial.print("serialRead: ");
      Serial.println(inputBuffer);
    #endif
    if(inputBuffer.indexOf("color") != -1)  // color|100|100|100|]
    {
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      pixels_color.red = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      pixels_color.green = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      pixels_color.blue = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
      MODE(modus_reflex);
    }
  }
}
void sensorUpdateFunction()
{
  ir_sensor.update();
  accelgyro.getAcceleration(&acceleration.x,&acceleration.y,&acceleration.z);
  //                                                                                                      9.81*2048
  accelerationAverage = ((float)abs(acceleration.x)+(float)abs(acceleration.y)+(float)abs(acceleration.z)-15357.5424)/(3*2048);
  #ifdef DEBUG
    Serial.print("accelerationAverage: ");
    Serial.println(accelerationAverage);
   #endif
   if(accelerationAverage > accelerationTrigger)
   {
      switch(MODE())
      {
        case modus_none:
        {
          
          break;
        }
        case modus_reflex:
        {
          triggerFunction();
          break;
        }
      }
   }
}
void triggerFunction()
{
  switch(MODE())
  {
    case modus_none:
    {
      
      break;
    }
    case modus_reflex:
    {
      mode_reflex_time = mode_reflex_Timer.runtime();
      mode_reflex_Timer.stop();
      pixels_enable = false;
      MODE(modus_none);
     #ifdef DEBUG
        Serial.print("Reflex time: ");
        Serial.println(mode_reflex_time);
     #endif
      break;
    }
  }
}
void updateRGB()
{
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i,pixels_enable*pixels_color.red,pixels_enable*pixels_color.green,pixels_enable*pixels_color.blue);
  }
  pixels.show();
}
void IR_highTriggerFunction()
{
  #ifdef DEBUG
    Serial.println("IR_sensor trigger high");
  #endif
  
  /*irSensorCooldownTimer.stop();
  irSensorCooldownTimer.start(1000);
  MODE(modus_rgbTest);*/
 /* pixels_enable = false;
  MODE(modus_none);*/
  triggerFunction();
}

void irSensorCooldownTimerFinishedFunction()
{
  #ifdef DEBUG
    Serial.println("TIMER");
  #endif
  pixels_enable = false;
  MODE(modus_none);
}

void handleMode()
{
  switch(MODE())
  {
    case modus_none:
    {
      
      break;
    }
    case modus_reflex:
    {
      pixels_enable = true;
      mode_reflex_Timer.start();
      mode_reflex_time = 0;
      //updateRGB();
      
      break;
    }
  }
}


void MODE(modus m)
{
  lastMode = mode;
  mode = m;
}
modus MODE()
{
  return mode;
}
modus LASTMODE()
{
  return lastMode;
}

