#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Timer.h"
#include "button.h"


LiquidCrystal_I2C lcd(0x3F,20,4);
Button button_1(5);
Button button_2(6);
Button button_3(7);
Button button_4(8);
Button button_5(9);
Button button_6(10);

void b1H();
void b2H();
void b3H();
void b4H();
void b5H();
void b6H();

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
}

void loop()
{
  //Serial.println("U");
  button_1.update();
  button_2.update();
  button_3.update();
  button_4.update();
  button_5.update();
  button_6.update();

  
}
void b1H()
{
  Serial.println("Button 1");
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
