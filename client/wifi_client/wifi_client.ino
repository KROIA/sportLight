/*
Autor Alex Krieg
Datum 13.07.2018
Version 0.2.0
*/

#include <ESP8266WiFi.h>
#include <MQTT.h>
#define BOX_INDEX 1
#define CLIENT_ID "client3"
//#define DEBUG

// Update these with values suitable for your network.

const char* ssid = "bedienbox";
const char* password = "passwort";
const char* mqtt_server = "192.168.4.1";

struct color
{
  byte red;
  byte green;
  byte blue;
};
void callback(String& topic, String& data);
void myPublishedCb();
void myDisconnectedCb();
void myConnectedCb();

MQTT myMqtt(CLIENT_ID, "192.168.4.1", 1883);
const String topic_fbox = "fbox"+String(BOX_INDEX);
const String topic_tbox = "tbox"+String(BOX_INDEX);
const String topic_fmaster = "fmaster";
const String topic_tmaster = "tmaster";

void mqttSplit(String topic , String message);
void mqttSend(String topic , String message);
void serialRead();
void serialSend(String data);
void serialSplit(String data);

void mqttError(String message);
void serialError(String message);
void debug(String message);
String macToStr(const uint8_t* mac);

void setup_wifi() {

 // if(WiFi.status() != WL_CONNECTED)
{
    delay(10);
    // We start by connecting to a WiFi network
    debug("");
    debug("Connecting to "+String(ssid));
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      #ifdef DEBUG
      Serial.print(".");
      #endif
    }
  
    //randomSeed(micros());
  
    debug("");
    debug("WiFi connected");
    debug("IP address: ");
    #ifdef DEBUG
    Serial.print("D: ");
    Serial.println(WiFi.localIP());
    #endif
    WiFi.localIP();
  }
  
}
void callback(String& topic, String& data) {
  debug("topic: \""+topic+"\" message: \""+data+"\"");
  mqttSplit(topic,data);
}

void reconnect() {
  if(WiFi.status() != WL_CONNECTED){
    debug("Connecting to ");
    debug(ssid);
    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      #ifdef DEBUG
      Serial.print(".");
      #endif
    }
    debug("");
    debug("WiFi connected");  
    debug("IP address: ");
    #ifdef DEBUG
    Serial.println(WiFi.localIP());
    #endif
  }
  myMqtt.connect();
  debug("reconnect done");
}

void setup() {
  Serial.begin(115200);
  //delay(3000);
  setup_wifi();
  myMqtt.onConnected(myConnectedCb);
  myMqtt.onDisconnected(myDisconnectedCb);
  myMqtt.onPublished(myPublishedCb);
  myMqtt.onData(callback);
  debug("connect mqtt...");
  //while(!myMqtt.isConnected())
  {
    myMqtt.connect();
    #ifdef DEBUG
    Serial.print(".");
    #endif
    delay(100);
  }
  debug("");
  debug("subscribe to topic...");
  debug("topic \""+topic_tbox+"\"");
  myMqtt.subscribe(topic_tbox);
  //debug("topic \""+topic_fmaster+"\"");
  //myMqtt.subscribe(topic_fmaster);
  //reconnect();
  debug("setup done");
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){reconnect();}
  serialRead();
}
void myConnectedCb()
{
  debug("connected to MQTT server");
  mqttSend(topic_fbox,"connected");
}

void myDisconnectedCb()
{
  debug("disconnected. try to reconnect...");
  delay(500);
  myMqtt.connect();
}

void myPublishedCb()
{
  //Serial.println("published.");
}
void debug(String message)
{
  #ifdef DEBUG
  Serial.println("D: "+message);
  #endif
}
void mqttSplit(String topic , String message)
{
  if(topic == topic_fmaster)
  {
    
  }else if(topic == topic_tbox)
  {
    /*if(message.indexOf("gvoltage") != -1)
    {
      serialSend("get|voltage");
    }*/
    serialSend(message);
  }else
  {
    mqttError(topic+" :\""+message+"\" -> unknown topic");
  }
}
void mqttError(String message)
{
  debug("mqtt Error: " + message);
}
void serialError(String message)
{
  debug("serial Error: " + message);
}
void mqttSend(String topic , String message)
{
  debug("mqtt_send topic: \""+topic+"\" message: \""+message+"\"");
  bool state = myMqtt.publish(topic,message);
  debug("return: "+String(state));
}

void serialRead()
{
  if(Serial.available() != 0)
  {
    String data = Serial.readStringUntil(']');
    if(data.indexOf("D: ") == 0){return;}
    //serialSplit(data);
    mqttSend(topic_fbox,data);
  }
}
void serialSend(String data)
{
  Serial.print(data+"]");
}
void serialSplit(String data)
{
 /* if(data.indexOf("voltage") == 0)
  {
    if(data.indexOf("voltageLow") == 0)
    {
      
    }else if(data.indexOf("voltageDOut") == 0)
    {
      
    }else
    {
      mqttSend(topic_fbox,data);
    }
  }*/
}
