#define VERSION  "0.0.3"
//      Autor Alex Krieg
//      Datum 13.07.2018


#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Timer.h"
#include "button.h"

enum mode
{
  mode_none = 0,
  mode_menue = 1,
  mode_info = 2,
};
mode modus = mode_menue;
mode lastModus = mode_menue;

byte selection = 0;

//-------------------AKKU-----------------
#define AKKUPIN A1
float maxAkkuVoltage = 7.20;
float akkuVoltage = 7.20;
int akkuPercent = 100;
float minAkkuVoltage = 6.5; // 6.0V
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

//-------------------------------BOX
float voltage_Box1 = 0;
//----------------------------------

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
  pinMode(AKKUPIN,INPUT);
  Serial.begin(115200);
  lcd.init();  
  lcd.backlight();
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
  readSerial();
}

void readSerial()
{
  String message = "";
  if(Serial.available() != 0)
  {
    message = Serial.readStringUntil(']');
  }
  else{return;}

  if(message.indexOf("box") == 0)
  {
    message = message.substring(3);
    int boxNr = atoi(message.substring(0,message.indexOf("|")).c_str());
    message = message.substring(message.indexOf("|")+1);
    if(message.indexOf("voltage") == 0)
    {
      float voltage = message.substring(message.indexOf("|")+1).toFloat();
      voltage_Box1 = voltage;
    }
  }
}
void writeSerial(String data)
{
  Serial.print(data+"]");
}

void MODE(mode m)
{
  lastModus = modus;
  modus = m;
  lcd.clear();
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
 // Serial.println("Button back");
  MODE(LASTMODE());
}
void b2H()
{
 // Serial.println("Button ok");
  switch(MODE())
  {
    case mode_menue:
    {
      switch(selection)
      {
        case 0://Info
        {
          writeSerial("tbox1|gvoltage");
          MODE(mode_info);
          break;
        }
        case 1://Mode
        {
          
          break;
        }
      }
      break; 
    }
  }
}
void b3H()
{
 // Serial.println("Button up");
  switch(MODE())
  {
     case mode_menue:
     {
       if(selection > 0)
       {
         selection--;
       }
       break;
     }
  }
}
void b4H()
{
//  Serial.println("Button 4");
}
void b5H()
{
//  Serial.println("Button down");
  switch(MODE())
  {
     case mode_menue:
     {
       if(selection < 1)// max menue positions 0 - 1
       {
         selection++;
       } 
       break;
     }
  }
}
void b6H()
{
//  Serial.println("Button 6");
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
        lcd.noBacklight();
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
        lcd.backlight();
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
    case mode_menue:
    {
       switch(selection)
       {
         case 0:
         {
           lcd_menue_info(true,0);
           lcd_menue_modus(false,1);
           break; 
         }
         case 1:
         {
           lcd_menue_info(false,0);
           lcd_menue_modus(true,1);
           break; 
         }
       }
       break; 
    }
    case mode_info:
    {
       lcd_info();
       break; 
    }
  }
}




//------------------DISPLAY------------------

void lcd_print(String message,byte x,byte y)
{
  lcd.setCursor(x,y);
  lcd.print(message);
}

//----------------------------------------------------MENUE----
void lcd_menue_info(bool selected,byte y)
{
  String message = "Info              ";
  if(selected){lcd_print("- "+message,0,y);}
  else{        lcd_print("  "+message,0,y);}
}
void lcd_menue_modus(bool selected,byte y)
{
  String message = "Modus             ";
  if(selected){lcd_print("- "+message,0,y);}
  else{        lcd_print("  "+message,0,y);}
}
//-------------------------------------------------------------
//----------------------------------------------------INFO-----
void lcd_info()
{
  lcd.setCursor(0,0);
  lcd.print("Version: "+String(VERSION)+"      ");
  lcd.setCursor(0,1);
  lcd.print("Akku: "+String(akkuPercent)+"%    ");
  lcd.setCursor(0,2);
  lcd.print("Voltage: "+String(akkuVoltage)+"V    ");
  lcd.setCursor(0,3);
  lcd.print("Box1: "+String(voltage_Box1)+"V    ");
}
//-------------------------------------------------------------
















