// Autor Alex Krieg
// Datum 11.09.2018
#define VERSION "0.3.0"

#include <ESP8266WiFi.h>
#include "uMQTTBroker.h"


const char SEPARATOR = '|';

struct color
{
  byte red;
  byte green;
  byte blue;
};

char ssid[] = "bedienbox";  	// your network SSID (name)
char pass[] = "passwort";	    // your network password

unsigned int mqttPort = 1883;       // the standard MQTT broker port
unsigned int max_subscriptions = 30;
unsigned int max_retained_topics = 30;


void mqttSplit(String topic , String message);
void mqttSend(String topic , String message);
void serialRead();
void serialSend(String data);
void serialSplit(String data);

void mqttError(String message);
void serialError(String message);
void debug(String message);

void data_callback(uint32_t *client, const char* topic, uint32_t topic_len, const char *data, uint32_t lengh) {
  char topic_str[topic_len+1];
  os_memcpy(topic_str, topic, topic_len);
  topic_str[topic_len] = '\0';

  char data_str[lengh+1];
  os_memcpy(data_str, data, lengh);
  data_str[lengh] = '\0';
  
  debug("received topic '"+String(topic_str)+"' with data '"+String(data_str)+"'");
  
  mqttSplit(topic_str,data_str);
}

void setup()
{
  Serial.begin(115200);
  debug("Connecting to ");
  debug(ssid);
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, pass);
  debug("");
  
  debug("WiFi connected");
  debug("IP address: ");
  debug(String(WiFi.softAPIP()));

  MQTT_server_onData(data_callback);

  debug("Starting MQTT broker");
  MQTT_server_start(mqttPort, max_subscriptions, max_retained_topics);

  MQTT_local_subscribe((unsigned char *)"#", 0);
}

int counter = 0;

void loop()
{
  serialRead();
}
void mqttSplit(String topic , String message)
{
    if(topic.indexOf("fbox") == 0)
    {
      topic = topic.substring(1);
      serialSend(topic+SEPARATOR+message);
    }
 /* if(topic.substring(0,1) == "f") //---------------FROM
  {
    topic = topic.substring(1);
    if(topic.indexOf("master") != -1)
    {
      
    }else if(topic.indexOf("box") != -1)
    {
      topic = topic.substring(3);
      int boxNr = atoi(topic.c_str());
      if(message.indexOf("voltage") == 0)
      {
        message = message.substring(message.indexOf("|")+1);
        float voltage = message.toFloat();
        serialSend("box"+String(boxNr)+"|voltage|"+String(voltage));
      }else if(message.indexOf("color") == 0)
      {
        message = message.substring(message.indexOf("|")+1);
        color col;
        col.red = atoi(message.substring(0,message.indexOf("|")).c_str());
        message = message.substring(message.indexOf("|")+1);
        col.green = atoi(message.substring(0,message.indexOf("|")).c_str());
        message = message.substring(message.indexOf("|")+1);
        col.blue = atoi(message.substring(0,message.indexOf("|")).c_str());
      }else if(message.indexOf("trigger") == 0)
      {
        message = message.substring(message.indexOf("|")+1);
        String trigger = message.substring(0,message.indexOf("|"));
        message = message.substring(message.indexOf("|")+1);
        unsigned int triggerTime = 0;
        if(message != "-")
        {
           triggerTime = atoi(message.c_str());
        }
      }else if(message.indexOf("avtive") == 0)
      {
        
      }else if(message.indexOf("inactive") == 0)
      {
        
      }else if(message.indexOf("ping") == 0)
      {
        mqttSend("tbox"+String(boxNr),"pong");
      }else if(message.indexOf("on") == 0)
      {
        
      }else if(message.indexOf("off") == 0)
      {
        
      }
      else
      {
        mqttError("fbox"+String(boxNr)+" :\""+message+"\" -> unknown message");
      }
      
    }
    else
    {
      mqttError(topic+" :\""+message+"\" -> unknown topic");
    }
  }else if(topic.substring(0,1) == "t")      //-----------------TO
  {
    topic = topic.substring(1);
    if(topic.indexOf("master") != -1)
    {
      
    }else if(topic.indexOf("box") != -1)
    {
     // topic = topic.substring(3);
     // int boxNr = atoi(topic.c_str());
      
    }
  }
  else
  {
    mqttError(topic+" :\""+message+"\" -> unknown topic");
  }*/
  
}
void mqttSend(String topic , String message)
{
  MQTT_local_publish((unsigned char *)topic.c_str(), (unsigned char *)message.c_str(), message.length(), 0, 0);
}
void serialRead()
{
  if(Serial.available() != 0)
  {
    String data = Serial.readStringUntil(']');
    if(data.indexOf("D: ") == 0){return;}
    serialSplit(data);
  }
}
void serialSend(String data)
{
  Serial.print(data+"]");
}
void serialSplit(String data)
{
  if(data.indexOf("tbox") == 0)
  {
    mqttSend(data.substring(0,data.indexOf("|")),data.substring(data.indexOf("|")+1));
  }else if(data.indexOf("fmaster") == 0)
  {
    mqttSend("fmaster",data.substring(data.indexOf("|")+1));
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
void debug(String message)
{
  #ifdef DEBUG
  Serial.println("D: "+message);
  #endif
}










