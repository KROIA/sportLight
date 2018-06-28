/*
Autor Alex Krieg
Datum 28.06.2018
Version 0.2.3
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
#define AKKUPIN A1

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
enum RGB_mode
{
  RGB_none = 0,
  RGB_loading = 1,
  RGB_akku = 2,
  RGB_normal = 3
};
enum modus
{
  modus_none = 0,
  modus_reflex = 1,
  modus_akku = 2
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);
RGB_mode rgb_mode = RGB_normal;
RGB_mode last_rgb_mode = RGB_normal;

MPU6050 accelgyro;
IR_sensor ir_sensor(3,A0); 
Timer sensor_UpdateIntervalTimer;
Timer pixelsUpdateTimer;
Timer wifiServerUpdateTimer;

//---------------------------MODE
Timer mode_reflex_Timer;
unsigned int mode_reflex_time = 0;

//---------
bool mode_akkuLow_dimUp = true;
Timer mode_akkuTimer;
Timer displayAkkuTimer;
byte mode_akkuLow_colorRed = 0;
byte mode_akku_numLedOn = 1;
//------------------------------

const float accelerationTrigger = 3.0;
float gesAcceleration = 0;
float maxAkkuVoltage = 9.00;
float akkuVoltage = 9.00;
float minAkkuVoltage = 7.0; // 7.0V
Timer checkAkkuTimer;

int loadingPos = 1;

accel acceleration;
color pixels_color;
bool  pixels_enable = false;
modus mode = modus_none;
modus lastMode = modus_none;

float tmpAX = 0;
float tmpAY = 0;
float tmpAZ = 0;
Timer loadingTimer;

void readSerial();
void writeSerial(String data);
void wifiUpdate();
void sensorUpdateFunction();
void triggerFunction();
void updateRGB();
void IR_highTriggerFunction();
void handleMode();

void displayAkkuTimerFunction();

void MODE(modus m);
modus MODE();
modus LASTMODE();


void setup()
{
  Serial.begin(115200);
  Serial.print("version: ");
  Serial.println("0.2.3");
  accelgyro.initialize();
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  pixels.begin(); // This initializes the NeoPixel library.
  #ifdef DEBUG
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  #endif

  pinMode(AKKUPIN,INPUT);
    
  sensor_UpdateIntervalTimer.onFinished(sensorUpdateFunction);
  sensor_UpdateIntervalTimer.autoRestart(true);
  sensor_UpdateIntervalTimer.start(20);
  pixelsUpdateTimer.onFinished(updateRGB);
  pixelsUpdateTimer.autoRestart(true);
  pixelsUpdateTimer.start(20);
  wifiServerUpdateTimer.onFinished(wifiUpdate);
  wifiServerUpdateTimer.autoRestart(true);
  wifiServerUpdateTimer.start(3000);

  checkAkkuTimer.onFinished(checkAkku);
  checkAkkuTimer.autoRestart(true);
  checkAkkuTimer.start(50);

  displayAkkuTimer.onFinished(displayAkkuTimerFunction);
  
  ir_sensor.onHighTrigger(IR_highTriggerFunction);

  pixels_color.red = 10;
  pixels_color.green = 10;
  pixels_color.blue = 10;



    /*accelgyro.setXAccelOffset(-2210);
    accelgyro.setYAccelOffset(-600);
    accelgyro.setZAccelOffset(1702);*/
/*
  accelgyro.setXAccelOffset(-2212);
  accelgyro.setYAccelOffset(-648);
  accelgyro.setZAccelOffset(2585);

  accelgyro.setXGyroOffset(6);
  accelgyro.setYGyroOffset(65);
  accelgyro.setZGyroOffset(-23);
*/
/*
  accelgyro.setXAccelOffset(-4550);
  accelgyro.setYAccelOffset(-1601);
  accelgyro.setZAccelOffset(1252);

  accelgyro.setXGyroOffset(150);
  accelgyro.setYGyroOffset(-47);
  accelgyro.setZGyroOffset(2);

  accelgyro.setFullScaleAccelRange(2);
*/
    /*
    accelgyro.setXAccelOffset(-2159);
    accelgyro.setYAccelOffset(-664);
    accelgyro.setZAccelOffset(1650);
    */
  
  calibration();
  /*
  Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76       -2159
  Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359   -664
  Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688    1788
  Serial.print("\n");*/
 // delay(1000);
}

void loop()
{
  pixelsUpdateTimer.update();
  sensor_UpdateIntervalTimer.update();
  wifiServerUpdateTimer.update();
  checkAkkuTimer.update();
  displayAkkuTimer.update();
  
  handleMode();
  readSerial();
}
void wifiUpdate()
{
  if(mode_reflex_Timer.isRunning())
  {  
    return;
  }
  writeSerial("color");
  //-------------einzeltest
  pixels_color.red = random(0,100);
  pixels_color.green = random(0,100);
  pixels_color.blue = random(0,100);
  MODE(modus_reflex);
  //----------------------
  
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
    if(inputBuffer.indexOf("color") != -1)  // color|100|100|100|
    {
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      pixels_color.red = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      pixels_color.green = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      pixels_color.blue = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
      MODE(modus_reflex);
    }
    if(inputBuffer.indexOf("data") != -1)  //data|
    {
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      if(inputBuffer.indexOf("voltage") != -1)  
      {
        if(MODE() != modus_akku && RGB_MODE() != RGB_akku)
        {
          inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
          displayAkkuTimer.start(atoi(inputBuffer.c_str())*1000);
          MODE(modus_akku);
          RGB_MODE(RGB_akku);
        }
        writeSerial("voltage|"+String(akkuVoltage));
      }
    }
  }
}
void writeSerial(String data)
{
  Serial.print(data+"]");
}
void sensorUpdateFunction()
{
  ir_sensor.update();
  //accelgyro.getAcceleration(&acceleration.x,&acceleration.y,&acceleration.z);
  getAcceleration(accelgyro,tmpAX,tmpAY,tmpAZ);
  gesAcceleration = sqrt(tmpAX*tmpAX+tmpAY*tmpAY+tmpAZ*tmpAZ)-9.804;
 // #ifdef DEBUG
 /*Serial.print(ax);
 Serial.print("\t");

  Serial.print(ay);
 Serial.print("\t");
  Serial.print(az);
  Serial.print("\t");
  Serial.println(gesAcceleration);*/
// Serial.print("\t");
 //Serial.print("\n");
   // Serial.print("gesAcceleration: ");
   // Serial.println(gesAcceleration);
 //  Serial.println(gesAcceleration);
 //  #endif
 //Serial.println(gesAcceleration);
   if(gesAcceleration > accelerationTrigger)
   {
      switch(MODE())
      {
        case modus_none:
        {
          
          break;
        }
        case modus_reflex:
        {
          #ifdef DEBUG
            Serial.print("\t accel: ");
            Serial.println(gesAcceleration);
          #endif
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
  if(RGB_MODE() != RGB_akku){mode_akku_numLedOn = 0;} 
  switch(RGB_MODE())
  {
    case RGB_none:
    {

      break;
    }
    case RGB_loading:
    {
      if(loadingTimer.start(20))
      {
        for(int a=0; a<NUMPIXELS; a++)
        {
          if(a == loadingPos || a == loadingPos-1)
           pixels.setPixelColor(a,0,50,0);
          else
           pixels.setPixelColor(a,0,0,0);
      
           
        }
        loadingPos++;
        if(loadingPos >= NUMPIXELS)
        {
          loadingPos = 1;
        }
        pixels.show();
      }
      break;
    }
    case RGB_akku:
    {
      if(mode_akkuTimer.start(40))
      {
          for(int a=0; a<NUMPIXELS; a++)
          {
            pixels.setPixelColor(a,0,0,0);
          }
          byte pixelPos = map((int)((akkuVoltage-(minAkkuVoltage-1.0))*100),0,(maxAkkuVoltage-(minAkkuVoltage-1.0))*100.0,1,NUMPIXELS);
          for(int a=0; a<pixelPos; a++)
          {
            //map((long)(akkuVoltage*100)%NUMPIXELS,0,1223/NUMPIXELS,0,30)
           /* if(mode_akku_numLedOn > 25)
              mode_akku_numLedOn = 0;*/
            if(a>mode_akku_numLedOn)
              continue;
             int brightness = 100;
             if(a == pixelPos-1)
             {
             // Serial.print("brightness: ");
              // brightness = /*(1100-(akkuVoltage-(minAkkuVoltage-1.0))*100)*/(1100-(minAkkuVoltage-1.0)*100)  / NUMPIXELS;
             //  Serial.println(brightness);
             //  brightness = map(akkuVoltage*100-brightness*pixelPos,0,brightness*NUMPIXELS,0,100);
              // brightness = map((1100-(akkuVoltage-(minAkkuVoltage-1.0))*100),0,brightness
               brightness = (int)((maxAkkuVoltage-(minAkkuVoltage-1.0))*100)  / NUMPIXELS;
              // Serial.print(brightness);
               brightness = map((int)((akkuVoltage-(minAkkuVoltage-1.0))*100)% NUMPIXELS,0,brightness,0,100);
             //  Serial.print(" ");
             //  Serial.println(brightness);
             }
             pixels.setPixelColor(a,brightness*map(a,0,NUMPIXELS,30,0)/100,brightness*map(a,0,NUMPIXELS,0,30)/100,0);
          }
          pixels.show();
          if(mode_akku_numLedOn < pixelPos+1)
          {
            mode_akku_numLedOn++;
          }
      }
      break;
    }
    case RGB_normal:
    {
      for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i,pixels_enable*pixels_color.red,pixels_enable*pixels_color.green,pixels_enable*pixels_color.blue);
      }
      break;
    }
  }
  
  pixels.show();
}
void IR_highTriggerFunction()
{
  #ifdef DEBUG
    Serial.println("IR_sensor trigger high");
  #endif
  triggerFunction();
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
      RGB_MODE(RGB_normal);
      //updateRGB();
      
      break;
    }
    case modus_akku:
    {
      RGB_MODE(RGB_akku);
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
void RGB_MODE(RGB_mode m)
{
  last_rgb_mode = rgb_mode;
  rgb_mode = m;
}
RGB_mode RGB_MODE()
{
  return rgb_mode;
}
RGB_mode LAST_RGB_MODE()
{
  return last_rgb_mode;
}


void calibration()
{
  float ax = 0;
  float ay = 0;
  float az = 0;
  float endError = 0.5;
  float error = 0;
  float aGes = 0;

  float toAX = 0;
  float toAY = 0;
  float toAZ = 9.804;
  for(int a=0; a<10; a++)
  {
    getAcceleration(accelgyro,tmpAX,tmpAY,tmpAZ);
    aGes += sqrt(tmpAX*tmpAX+tmpAY*tmpAY+tmpAZ*tmpAZ);
    delay(5);
  }
  aGes /= 10;
  int map_firstError = sqrt((aGes - toAZ)*(aGes - toAZ)) * 150;
  bool ret = false;
  rgb_mode = RGB_loading;
  while(!ret)
  {
      int counter = 0;
      ax = 0;
      ay = 0;
      az = 0;
      while(counter < 10)
      {
        getAcceleration(accelgyro,tmpAX,tmpAY,tmpAZ);
        ax+=tmpAX;
        ay+=tmpAY;
        az+=tmpAZ;
        counter++;
        delay(5);
        updateRGB();
        //LED_loading();
      }
      ax /= 10;
      ay /= 10;
      az /= 10;
      aGes = sqrt(ax*ax+ay*ay+az*az);
      error = aGes - toAZ;
      error = sqrt(error*error);
      
      if(error < endError && sqrt(ax*ax)-toAX < endError && sqrt(ay*ay)-toAY < endError && sqrt(az*az)-toAZ < endError)
      {
        ret = true;
      }
          
      int16_t offsetX = (toAX-ax) * 10 + accelgyro.getXAccelOffset();
      int16_t offsetY = (toAY-ay) * 10 + accelgyro.getYAccelOffset();
      int16_t offsetZ = (toAZ-az) * 10 + accelgyro.getZAccelOffset();
      accelgyro.setXAccelOffset(offsetX);
      accelgyro.setYAccelOffset(offsetY);
      accelgyro.setZAccelOffset(offsetZ);
  }
  rgb_mode = RGB_none;
  for(int b=0; b<50; b++)
  {
     for(int a=0; a<NUMPIXELS; a++)
     {
        pixels.setPixelColor(a,0,b,0);
     }
     pixels.show();
     delay(10);
  }
  for(int b=50; b>=0; b--)
  {
     for(int a=0; a<NUMPIXELS; a++)
     {
        pixels.setPixelColor(a,0,b,0);
     }
     pixels.show();
     delay(10);
  }
}
void getAcceleration(MPU6050 &accel,float &ax,float &ay, float &az)
{
  int16_t _ax;
  int16_t _ay;
  int16_t _az;
  accel.getAcceleration(&_ax,&_ay,&_az);

  ax = (float)_ax/(1024 * 0.8162);
  ay = (float)_ay/(1024 * 0.8162);
  az = (float)_az/(1024 * 0.8162);
}

void checkAkku()
{
  //V factor 0.6911764
  int tmpV = analogRead(AKKUPIN);
  akkuVoltage += (float)map(tmpV,0,1024,0,1223)/100;
  akkuVoltage /= 2;
  //Serial.print("voltage: ");
  //Serial.println(akkuVoltage);
  if(akkuVoltage < minAkkuVoltage)
  {
    //Serial.println("voltage is to low");
    MODE(modus_akku);
    pixels_color.red = 0;
    pixels_color.green = 0;
    pixels_color.blue = 0;
   // pixelsUpdateTimer.autoRestart(false);
   // pixelsUpdateTimer.stop();
  }
  else
  {
    //MODE(mode_none);
  }
}


void displayAkkuTimerFunction()
{
  MODE(LASTMODE());
  RGB_MODE(LAST_RGB_MODE());
}

