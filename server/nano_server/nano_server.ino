/*
Version 0.0.2
Autor Alex Krieg
Datum 11.07.2018
*/


#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Timer.h"
#include "button.h"

enum mode
{
  mode_none = 0,
  mode_menue = 1,
  mode_akku = 2
};
mode modus = mode_none;
mode lastModus = mode_none;
//-------------------AKKU-----------------
#define AKKUPIN A1
float maxAkkuVoltage = 9.00;
float akkuVoltage = 9.00;
int akkuPercent = 100;
float minAkkuVoltage = 7.0; // 7.0V
float shutDownAkkuVoltage = 6.0;
bool shutDownAkku = false;
bool lowAkku = false;
Timer checkAkkuTimer;
//----------------------------------------
//-----------------DISPLAY----------------
Timer displayUpdateTimer;
//----------------------------------------

LiquidCrystal_I2C lcd(0x3F,20,4);
Button button_1(5);
Button button_2(6);
Button button_3(7);
Button button_4(8);
Button button_5(9);
Button button_6(10);
Timer buttonUpdateTimer;

void readSerial();
void writeSerial(String data);

void MODE(mode m);
mode MODE();
mode LASTMODE();

void b1H();
void b2H();
void b3H();
void b4H();
void b5H();
void b6H();

void checkAkku();
void displayUpdate();

void setup()
{
  pinMode(A1,INPUT);
  Serial.begin(115200);

  Serial.println("SETUP");
  lcd.init();  
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");
  lcd.setCursor(2,1);
  lcd.print("Ywrobot Arduino!");
  lcd.setCursor(0,2);
  lcd.print("Arduino LCM IIC 0010");
  lcd.setCursor(2,3);
  lcd.print("Power By Ec-yuan!");
  button_1.OnPressedEdge(b1H);
  button_2.OnPressedEdge(b2H);
  button_3.OnPressedEdge(b3H);
  button_4.OnPressedEdge(b4H);
  button_5.OnPressedEdge(b5H);
  button_6.OnPressedEdge(b6H);
  
  checkAkkuTimer.onFinished(checkAkku);
  checkAkkuTimer.autoRestart(true);
  checkAkkuTimer.start(50);
  
  displayUpdateTimer.onFinished(displayUpdate);
  displayUpdateTimer.autoRestart(true);
  displayUpdateTimer.start(500);
}

void loop()
{
  if(buttonUpdateTimer.start(50))
  {
    button_1.update();
    button_2.update();
    button_3.update();
    button_4.update();
    button_5.update();
    button_6.update();
  }
  displayUpdateTimer.update();
  checkAkkuTimer.update();
  
}

void readSerial()
{
  
}
void writeSerial(String data)
{
  Serial.print(data+"]");
}

void MODE(mode m)
{
  lastModus = modus;
  modus = m;
}
mode MODE()
{
  return modus;
}
mode LASTMODE()
{
  return lastModus;
}
void b1H()
{
  Serial.println("Button 1");
  if(MODE() == mode_akku)
  {
    MODE(LASTMODE()); 
  }
  else
  {
    MODE(mode_akku); 
  }
}
void b2H()
{
  Serial.println("Button 2");
}
void b3H()
{
  Serial.println("Button 3");
}
void b4H()
{
  Serial.println("Button 4");
}
void b5H()
{
  Serial.println("Button 5");
}
void b6H()
{
  Serial.println("Button 6");
}
void checkAkku()
{
  //V factor 0.6911764
  int tmpV = analogRead(AKKUPIN);
  akkuVoltage += (float)map(tmpV,0,1024,0,1223)/100;
  akkuVoltage /= 2;
  akkuPercent = map(akkuVoltage*100,shutDownAkkuVoltage*100,maxAkkuVoltage*100,0,100);
  if(akkuPercent <0){akkuPercent = 0;}
  if(akkuPercent >100){akkuPercent = 100;}
  //Serial.println(String(akkuVoltage) +"\t"+String(shutDownAkkuVoltage) +"\t"+String(maxAkkuVoltage));
  if(akkuVoltage < minAkkuVoltage)
  {
    if(akkuVoltage < shutDownAkkuVoltage)
    {
      if(!shutDownAkku)
      {
        writeSerial("voltageDOut|"+String(akkuVoltage));
        shutDownAkku = true;
      }
    }
    else
    {
      if(!lowAkku)
      {
        writeSerial("voltageLow|"+String(akkuVoltage));
        lowAkku = true;
      }
      if(shutDownAkku)
      {
        shutDownAkku = false;
      }
    }
  }
  else
  {
    if(lowAkku)
    lowAkku = false;
  }
}
void displayUpdate()
{
  switch(MODE())
  {
    case mode_none:
    {
      lcd.clear();
      break;
    } 
    case mode_akku:
    {
       lcd.setCursor(0,0);
       lcd.print("Akku: "+String(akkuPercent)+"%    ");
       lcd.setCursor(0,1);
       lcd.print("Voltage: "+String(akkuVoltage)+"V    ");
    }
  }
}
