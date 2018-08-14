#define VERSION  "0.0.5"
//      Autor Alex Krieg
//      Datum 14.08.2018


#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Timer.h"
#include "button.h"

#define MAX_CLIENTS 8
byte clients;

enum mode
{
  mode_none = 0,
  mode_menue = 1,
  mode_info = 2,
  mode_settings = 3,
  mode_mode = 4,
};
mode modus = mode_menue;
mode lastModus = mode_menue;

//-----------------Betriebs modie---------------
const byte anzModies = 1;

bool testModus_enable = false;
//----------------------------------------------

byte selection = 0;
byte selection_untermenue = 0;

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
bool standby = false;
bool displayOn = true;
Timer standbyTimer;
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
float voltage_Box2 = 0;
float voltage_Box3 = 0;
float voltage_Box4 = 0;
float voltage_Box5 = 0;
float voltage_Box6 = 0;
float voltage_Box7 = 0;
float voltage_Box8 = 0;
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
void standbyOn();

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

  standbyTimer.onFinished(standbyOn);
  standbyTimer.autoRestart(true);
  standbyTimer.start(10000);
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
  standbyTimer.update();
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
    byte boxNr = (byte)atoi(message.substring(0,message.indexOf("|")).c_str());
    message = message.substring(message.indexOf("|")+1);
    if(message.indexOf("voltage") == 0)
    {
      float voltage = message.substring(message.indexOf("|")+1).toFloat();
      switch(boxNr)
      { case 1: {voltage_Box1 = voltage; break;}
        case 2: {voltage_Box2 = voltage; break;}
        case 3: {voltage_Box3 = voltage; break;}
        case 4: {voltage_Box4 = voltage; break;}
        case 5: {voltage_Box5 = voltage; break;}
        case 6: {voltage_Box6 = voltage; break;}
        case 7: {voltage_Box7 = voltage; break;}
        case 8: {voltage_Box8 = voltage; break;}
      }
    }
  }
}
void writeSerial(String data)
{
  Serial.print(data+"]");
}

void MODE(mode m)
{
  selection_untermenue = 0;
  
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
  lastModus = MODE();
  standbyTimer.stop();
  standby = false;
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
          //writeSerial("tbox1|gvoltage");
          MODE(mode_info);
          break;
        }
        case 1://Mode
        {
          MODE(mode_mode);
          break;
        }
        case 2://Settings
        {
          MODE(mode_settings);
          break;
        }
      }
      break; 
    }
    case mode_settings:
    {
      displayOn = !displayOn;
      break;
    }
    case mode_mode:
    {
      switch(selection_untermenue)
      {
        case 0:
        {
          testModus_enable = !testModus_enable;
          if(testModus_enable)
          {
            writeSerial("fmaster|smode|1");//Reflex mode
          }else
          {
            writeSerial("fmaster|smode|0");//None mode
          }
          break;
        }
      }
    }
  }
  standbyTimer.stop();
  standby = false;
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
     case mode_info:
     {
        if(selection_untermenue > 0)
        {
          selection_untermenue--;
        }
        break;
     }
     case mode_settings:
     {
        if(selection_untermenue > 0)
        {
          selection_untermenue--;
        }
        break;
     }
     case mode_mode:
     {
        if(selection_untermenue > 0)
        {
          selection_untermenue--;
        }
        break;
     }
  }
  standbyTimer.stop();
  standby = false;
}
void b4H()
{
  standbyTimer.stop();
  standby = false;
}
void b5H()
{
//  Serial.println("Button down");
  switch(MODE())
  {
     case mode_menue:
     {
       if(selection < 2)// max menue positions 0 - 2
       {
         selection++;
       } 
       break;
     }
     case mode_info:
     {
        if(selection_untermenue < 8)
        {
          selection_untermenue++;
        }
        break;
     }
     case mode_settings:
     {
        if(selection_untermenue < 4)
        {
          selection_untermenue++;
        }
        break;
     }
     case mode_mode:
     {
        if(selection_untermenue < anzModies-1)
        {
          selection_untermenue++;
        }
        break;
     }
  }
  standbyTimer.stop();
  standby = false;
}
void b6H()
{
  standbyTimer.stop();
  standby = false;  
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
        //writeSerial("voltageDOut|"+String(akkuVoltage));
        writeSerial("fmaster|soff");
        shutDownAkku = true;
        lcd.noBacklight();
      }
    }
    else if(akkuVoltage > 1+shutDownAkkuVoltage)
    {
      if(!lowAkku)
      {
        // writeSerial("voltageLow|"+String(akkuVoltage));
        writeSerial("fmaster|soff");
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
  if(standby ||!displayOn)
  {lcd.noBacklight();}else{if(displayOn)lcd.backlight();}
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
           lcd_menue("Info              ",true,0);
           lcd_menue("Modus             ",false,1);
           lcd_menue("Settings          ",false,2);
           break; 
         }
         case 1:
         {
           lcd_menue("Info              ",false,0);
           lcd_menue("Modus             ",true,1);
           lcd_menue("Settings          ",false,2);
           break; 
         }
         case 2:
         {
           lcd_menue("Info              ",false,0);
           lcd_menue("Modus             ",false,1);
           lcd_menue("Settings          ",true,2);
           break;
         }
       }
       break; 
    }
    case mode_info:
    {
       writeSerial("fmaster|gvoltage");
       lcd_info();
       break; 
    }
    case mode_settings:
    {
      lcd_settings();
      break;
    }
    case mode_mode:
    {
      lcd_mode();
      break;
    }
  }
}
void standbyOn()
{
  standby = true;
}



//------------------DISPLAY------------------

void lcd_print(String message,byte x,byte y)
{
  lcd.setCursor(x,y);
  lcd.print(message);
}

//----------------------------------------------------MENUE----
void lcd_menue(String message,bool selected,byte y)
{
 // String message = "Info              ";
  if(selected){lcd_print("- "+message,0,y);}
  else{        lcd_print("  "+message,0,y);}
}
/*void lcd_menue_modus(bool selected,byte y)
{
  String message = "Modus             ";
  if(selected){lcd_print("- "+message,0,y);}
  else{        lcd_print("  "+message,0,y);}
}
void lcd_menue_settings(bool selected,byte y)
{
  String message = "Settings          ";
  if(selected){lcd_print("- "+message,0,y);}
  else{        lcd_print("  "+message,0,y);}
}*/
//-------------------------------------------------------------
//----------------------------------------------------INFO-----
void lcd_info()
{
  switch(selection_untermenue)
  {
    case 0:{
      lcd_info_version(0);
      lcd_info_akku(1);
      lcd_info_voltage(2);
      lcd_info_voltage_box1(3);
      break;
    }
    case 1:{
      lcd_info_akku(0);
      lcd_info_voltage(1);
      lcd_info_voltage_box1(2);
      lcd_info_voltage_box2(3);
      break;
    }
    case 2:{
      lcd_info_voltage(0);
      lcd_info_voltage_box1(1);
      lcd_info_voltage_box2(2);
      lcd_info_voltage_box3(3);
      break;
    }
    case 3:{
      lcd_info_voltage_box1(0);
      lcd_info_voltage_box2(1);
      lcd_info_voltage_box3(2);
      lcd_info_voltage_box4(3);
      break;
    }
    case 4:{
      lcd_info_voltage_box2(0);
      lcd_info_voltage_box3(1);
      lcd_info_voltage_box4(2);
      lcd_info_voltage_box5(3);
      break;
    }
    case 5:{
      lcd_info_voltage_box3(0);
      lcd_info_voltage_box4(1);
      lcd_info_voltage_box5(2);
      lcd_info_voltage_box6(3);
      break;
    }
    case 6:{
      lcd_info_voltage_box4(0);
      lcd_info_voltage_box5(1);
      lcd_info_voltage_box6(2);
      lcd_info_voltage_box7(3);
      break;
    }
    case 7:{
      lcd_info_voltage_box5(0);
      lcd_info_voltage_box6(1);
      lcd_info_voltage_box7(2);
      lcd_info_voltage_box8(3);
      break;
    }
  }
}
void lcd_info_version(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Version: "+String(VERSION)+"      ");
}
void lcd_info_akku(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Akku: "+String(akkuPercent)+"%    ");
}
void lcd_info_voltage(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Voltage: "+String(akkuVoltage)+"V    ");
}

void lcd_info_voltage_box1(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box1: "+String(voltage_Box1)+"V    ");
}
void lcd_info_voltage_box2(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box2: "+String(voltage_Box2)+"V    ");
}
void lcd_info_voltage_box3(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box3: "+String(voltage_Box3)+"V    ");
}
void lcd_info_voltage_box4(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box4: "+String(voltage_Box4)+"V    ");
}
void lcd_info_voltage_box5(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box5: "+String(voltage_Box5)+"V    ");
}
void lcd_info_voltage_box6(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box6: "+String(voltage_Box6)+"V    ");
}
void lcd_info_voltage_box7(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box7: "+String(voltage_Box7)+"V    ");
}
void lcd_info_voltage_box8(byte y)
{
  lcd.setCursor(0,y);
  lcd.print("Box8: "+String(voltage_Box8)+"V    ");
}
//selection_untermenue
//-------------------------------------------------------------
void lcd_settings()
{
  switch(selection_untermenue)
  {
    case 0:
    {
      lcd_settings_displayOnOff(0);
      break;
    }
  }
}
void lcd_settings_displayOnOff(byte y)
{
  lcd.setCursor(0,y);
  if(displayOn)
  lcd.print("Display:   ON       ");
  else
  lcd.print("Display:   OFF      ");
}
//-------------------------------------------------------------
void lcd_mode()
{
  switch(selection_untermenue)
  {
    case 0:
    {
      lcd_mode_testModeOnOff(0);
      break;
    }
  }
}
void lcd_mode_testModeOnOff(byte y)
{
  lcd.setCursor(0,y);
  if(testModus_enable)
  lcd.print("TestMode:   ON      ");
  else
  lcd.print("TestMode:   OFF     ");
}










