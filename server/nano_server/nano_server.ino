// Autor Alex Krieg
// Datum 11.09.2018
#define VERSION  "0.1.0"

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Timer.h"
#include "button.h"

#define MAX_CLIENTS 8
byte clients;

//-------------------------COMMUNICATION------------------
const String topic_fmaster = "fmaster";
const String topic_tbox = "tbox";

const char GET       = 'g';
const char SET       = 's';
const char SEPARATOR = '|';

const String VOLTAGE = "v";
const String COLOR   = "c";
const String ACTIVE  = "a";
const String INACTIVE= "ia";
const String IR      = "i";
const String ACC     = "ac";
const String TRIGGER = "t";
const String _MODE   = "m";
const String ON      = "on";
const String OFF     = "of";
const String BRIGHTNESS="b";


//--------------------------------------------------------
//----------------------CONST TEXT------------------------
const String TEXT_SPACE     = " ";
const String TEXT_TAB       = "    ";
const String TEXT_Info      = "Info";
const String TEXT_Modus     = "Modus";
const String TEXT_Settings  = "Settings";
const String TEXT_Version   = "Version";
const String TEXT_Akku      = "Akku";
const String TEXT_Voltage   = "Voltage";
const String TEXT_Box       = "Box";
const String TEXT_Display   = "Display";
const String TEXT_ON        = "ON";
const String TEXT_OFF       = "OFF";
const String TEXT_TestMode  = "TestMode";


//--------------------------------------------------------

enum mode
{
  mode_none = 0,
  mode_menue = 1,
  mode_info = 2,
  mode_settings = 3,
  mode_mode = 4,
  mode_message = 5,
};
mode modus = mode_menue;
mode lastModus = mode_menue;
String mode_message_message = "";

//-----------------Betriebs modie---------------
const byte anzModies = 1;

bool testModus_enable = false;
//----------------------------------------------

byte selection = 0;
byte selection_untermenue = 0;

//-------------------AKKU-----------------
#define AKKUPIN A1

float akkuVoltage                 = 7.20;
int akkuPercent                   = 100;
const float maxAkkuVoltage        = 7.20;
const float minAkkuVoltage        = 6.5; // 6.0V
const float shutDownAkkuVoltage   = 6.0;
bool shutDownAkku                 = false;
bool lowAkku                      = false;
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
float voltage_Box[MAX_CLIENTS];

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

    
    if(message.indexOf(VOLTAGE) == 0)
    {
      float voltage = message.substring(message.indexOf("|")+1).toFloat();
      voltage_Box[boxNr] = voltage;
    }
    if(message.indexOf(COLOR) == 0)
    {
      
    }
    if(message.indexOf(TRIGGER) == 0)
    {
      
    }
    if(message.indexOf(ACTIVE) == 0)
    {
      
    }
    if(message.indexOf(INACTIVE) == 0)
    {
      
    }
    if(message.indexOf(ON) == 0)
    {
      
    }
    if(message.indexOf(OFF) == 0)
    {
      
    }
    if(message.indexOf(BRIGHTNESS) == 0)
    {
      
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
  MODE(LASTMODE());
  lastModus = MODE();
  standbyTimer.stop();
  standby = false;
}
void b2H()
{
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
            writeSerial(topic_fmaster+SEPARATOR+SET+_MODE+SEPARATOR+"1");//Reflex mode
          }else
          {
            writeSerial(topic_fmaster+SEPARATOR+SET+_MODE+SEPARATOR+"0");//None mode
          }
          break;
        }
      }
    }
    case mode_message:
    {
      MODE(LASTMODE());
      break;
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
  if(akkuVoltage < minAkkuVoltage)
  {
    if(akkuVoltage < shutDownAkkuVoltage)
    {
      if(!shutDownAkku)
      {
        //writeSerial("voltageDOut|"+String(akkuVoltage));
        writeSerial(topic_fmaster+SEPARATOR+SET+OFF);
        shutDownAkku = true;
        lcd.noBacklight();
      }
    }
    else if(akkuVoltage > 1+shutDownAkkuVoltage)
    {
      if(!lowAkku)
      {
        MODE(mode_message);
        mode_message_message = ">> LOW VOLTAGE <<";
        writeSerial(topic_fmaster+SEPARATOR+SET+OFF);
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
           lcd_menue(TEXT_Info+TEXT_TAB+TEXT_TAB,true,0);
           lcd_menue(TEXT_Modus+TEXT_TAB+TEXT_TAB,false,1);
           lcd_menue(TEXT_Settings+TEXT_TAB+TEXT_TAB,false,2);
           break; 
         }
         case 1:
         {
           lcd_menue(TEXT_Info+TEXT_TAB+TEXT_TAB,false,0);
           lcd_menue(TEXT_Modus+TEXT_TAB+TEXT_TAB,true,1);
           lcd_menue(TEXT_Settings+TEXT_TAB+TEXT_TAB,false,2);
           break; 
         }
         case 2:
         {
           lcd_menue(TEXT_Info+TEXT_TAB+TEXT_TAB,false,0);
           lcd_menue(TEXT_Modus+TEXT_TAB+TEXT_TAB,false,1);
           lcd_menue(TEXT_Settings+TEXT_TAB+TEXT_TAB,true,2);
           break;
         }
       }
       break; 
    }
    case mode_info:
    {
       writeSerial(topic_fmaster+SEPARATOR+GET+VOLTAGE);
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
    case mode_message:
    {
      lcd_message(0,mode_message_message);
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
  if(selected){lcd_print("-"+TEXT_SPACE+message,0,y);}
  else{        lcd_print(TEXT_SPACE+TEXT_SPACE+message,0,y);}
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
      lcd_info_voltage_box(1,3);
      break;
    }
    case 1:{
      lcd_info_akku(0);
      lcd_info_voltage(1);
      lcd_info_voltage_box(1,2);
      lcd_info_voltage_box(2,3);
      break;
    }
    case 2:{
      lcd_info_voltage(0);
      lcd_info_voltage_box(1,1);
      lcd_info_voltage_box(2,2);
      lcd_info_voltage_box(3,3);
      break;
    }
    case 3:{
      lcd_info_voltage_box(1,0);
      lcd_info_voltage_box(2,1);
      lcd_info_voltage_box(3,2);
      lcd_info_voltage_box(4,3);
      break;
    }
    case 4:{
      lcd_info_voltage_box(2,0);
      lcd_info_voltage_box(3,1);
      lcd_info_voltage_box(4,2);
      lcd_info_voltage_box(5,3);
      break;
    }
    case 5:{
      lcd_info_voltage_box(3,0);
      lcd_info_voltage_box(4,1);
      lcd_info_voltage_box(5,2);
      lcd_info_voltage_box(6,3);
      break;
    }
    case 6:{
      lcd_info_voltage_box(4,0);
      lcd_info_voltage_box(5,1);
      lcd_info_voltage_box(6,2);
      lcd_info_voltage_box(7,3);
      break;
    }
    case 7:{
      lcd_info_voltage_box(5,0);
      lcd_info_voltage_box(6,1);
      lcd_info_voltage_box(7,2);
      lcd_info_voltage_box(8,3);
      break;
    }
  }
}
void lcd_info_version(byte y)
{
  lcd.setCursor(0,y);
  lcd.print(TEXT_Version+TEXT_SPACE+String(VERSION)+TEXT_TAB);
}
void lcd_info_akku(byte y)
{
  lcd.setCursor(0,y);
  lcd.print(TEXT_Akku+TEXT_SPACE+String(akkuPercent)+"%"+TEXT_TAB);
}
void lcd_info_voltage(byte y)
{
  lcd.setCursor(0,y);
  lcd.print(TEXT_Voltage+TEXT_SPACE+String(akkuVoltage)+"V"+TEXT_TAB);
}

void lcd_info_voltage_box(byte box,byte y)
{
  lcd.setCursor(0,y);
  lcd.print(TEXT_Box+String(box)+TEXT_SPACE+String(voltage_Box[box])+"V"+TEXT_TAB);
}
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
  lcd.print(TEXT_Display+TEXT_SPACE+TEXT_SPACE+TEXT_SPACE+TEXT_ON+TEXT_TAB);
  else
  lcd.print(TEXT_Display+TEXT_SPACE+TEXT_SPACE+TEXT_SPACE+TEXT_OFF+TEXT_TAB);
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
  lcd.print(TEXT_TestMode+TEXT_SPACE+TEXT_SPACE+TEXT_SPACE+TEXT_ON+TEXT_TAB);
  else
  lcd.print(TEXT_TestMode+TEXT_SPACE+TEXT_SPACE+TEXT_SPACE+TEXT_OFF+TEXT_TAB);
}
//--------------------------------------------------------------
void lcd_message(byte y,String message)
{
  lcd.setCursor(0,y);
  lcd.print(message+TEXT_TAB);
}








