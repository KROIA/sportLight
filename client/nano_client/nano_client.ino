/*
Autor Alex Krieg
Datum 29.06.2018
Version 0.2.5
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
bool mode_reflex_returnReactionTime = false;
bool mode_reflex_returnTrigger      = false;

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
float shutDownAkkuVoltage = 6.0;
bool shutDownAkku = false;
bool downAkku = false;
Timer checkAkkuTimer;

int loadingPos = 1;

accel acceleration;
color pixels_color;
byte  pixels_brightness = 128;
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
void triggerFunction(byte source = 0);
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
  Serial.println("0.2.5");
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
  calibration();
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
//-----------------------------------------------SET-----------------------------------------
    if(inputBuffer.indexOf("set") == 0)
    {
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      if(inputBuffer.indexOf("color") == 0)  // color|100|100|100|
      {
        inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
        pixels_color.red = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
        inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
        pixels_color.green = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
        inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
        pixels_color.blue = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
        MODE(modus_reflex);
      }
      if(inputBuffer.indexOf("brightness") == 0)
      {
        inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
        pixels_brightness = atoi(inputBuffer.substring(0,inputBuffer.indexOf("|")).c_str());
      }
      if(inputBuffer.indexOf("reactionTime") == 0)
      {
        inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
        if(inputBuffer.indexOf("1") == 0){  mode_reflex_returnReactionTime = true;}else
                                         {  mode_reflex_returnReactionTime = false;}
      }
      if(inputBuffer.indexOf("trigger") == 0)
      {
        inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
        if(inputBuffer.indexOf("1") == 0){  mode_reflex_returnTrigger = true;}else
                                         {  mode_reflex_returnTrigger = false;}
      }
      
    } else 
//-----------------------------------------------GET-----------------------------------------
    if(inputBuffer.indexOf("get") == 0)
    {
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      if(inputBuffer.indexOf("color") == 0)  // color|100|100|100|
      {
        writeSerial("color|"+String(pixels_color.red)+"|"+String(pixels_color.green)+"|"+String(pixels_color.blue));
      }
      if(inputBuffer.indexOf("brightness") == 0)
      {
        writeSerial("brightness|"+String(pixels_brightness));
      }
      if(inputBuffer.indexOf("voltage") == 0)
      {
        writeSerial("voltage|"+String(akkuVoltage));
      }
    }
    if(inputBuffer.indexOf("mode") == 0)  //data|
    {
      inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
      if(inputBuffer.indexOf("voltage") == 0)  
      {
        if(MODE() != modus_akku && RGB_MODE() != RGB_akku)
        {
          inputBuffer = inputBuffer.substring(inputBuffer.indexOf("|")+1);
          displayAkkuTimer.start(atoi(inputBuffer.c_str())*1000);
          MODE(modus_akku);
          RGB_MODE(RGB_akku);
        }
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
  getAcceleration(accelgyro,tmpAX,tmpAY,tmpAZ);
  gesAcceleration = sqrt(tmpAX*tmpAX+tmpAY*tmpAY+tmpAZ*tmpAZ)-9.804;
  /* 
  Serial.print(tmpAX);
  Serial.print("\t");
  Serial.print(tmpAY);
  Serial.print("\t");
  Serial.print(tmpAZ);
  Serial.print("\t");
  Serial.println(gesAcceleration);
  */
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
          triggerFunction(2);
          break;
        }
      }
   }
}
void triggerFunction(byte source)
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
      if(mode_reflex_returnReactionTime)
      {
        writeSerial("reactionTime|"+String((float)mode_reflex_time/1000));
      }
      if(mode_reflex_returnTrigger)
      {
        //source ==:
        //  0 -> none
        //  1 -> IR_Sensor
        //  2 -> Accelometer
        writeSerial("trigger|"+String(source));
      }
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
           /* if(mode_akku_numLedOn > 25)
              mode_akku_numLedOn = 0;*/
            if(a>mode_akku_numLedOn)
              continue;
             int brightness = 100;
             if(a == pixelPos-1)
             {
               brightness = (int)((maxAkkuVoltage-(minAkkuVoltage-1.0))*100)  / NUMPIXELS;
               brightness = map((int)((akkuVoltage-(minAkkuVoltage-1.0))*100)% NUMPIXELS,0,brightness,0,100);
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
  pixels.setBrightness(pixels_brightness);
  pixels.show();
}
void IR_highTriggerFunction()
{
  #ifdef DEBUG
    Serial.println("IR_sensor trigger high");
  #endif
  triggerFunction(1);
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
  accelgyro.setFullScaleAccelRange(1);
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
  RGB_MODE(RGB_loading);
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
  RGB_MODE(RGB_none);
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
  if(akkuVoltage < minAkkuVoltage)
  {
    MODE(modus_akku);
    pixels_color.red = 0;
    pixels_color.green = 0;
    pixels_color.blue = 0;
    
    if(akkuVoltage < shutDownAkkuVoltage)
    {
      pixelsUpdateTimer.autoRestart(false);
      pixelsUpdateTimer.stop();
      updateRGB();
      pixels_brightness = 0;
      pixels_enable = false;
      if(!shutDownAkku)
      {
        writeSerial("voltageDOut|"+String(akkuVoltage));
        shutDownAkku = true;
      }
    }
    else
    {
      if(!downAkku)
      {
        writeSerial("voltageLow|"+String(akkuVoltage));
        downAkku = true;
      }
      if(shutDownAkku)
      {
        shutDownAkku = false;
        pixels_enable = true;
        MODE(LASTMODE());
        pixelsUpdateTimer.autoRestart(true);
        pixelsUpdateTimer.start(20);
      }
    }
  }
  else
  {
    if(downAkku)
    downAkku = false;
  }
}

void displayAkkuTimerFunction()
{
  MODE(LASTMODE());
  RGB_MODE(LAST_RGB_MODE());
}

