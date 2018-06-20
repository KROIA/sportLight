/*
Autor Alex Krieg
Datum 20.06.2018
Version 0.1.0
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 

const char* host = "192.168.4.1"; //Ip of the Host(Our Case esp8266-01 as server. Its the ip of the esp8266-01 as Access point)
//#define DEBUG
void setup() {
  pinMode(BUILTIN_LED, OUTPUT); 
  Serial.begin(115200);          //Baud Rate for Communication
  delay(10);                     //Baud rate prper initialization
  WiFi.mode(WIFI_STA);           //NodeMcu esp12E in station mode
  WiFi.begin("ESP_D54736");      //Connect to this SSID. In our case esp-01 SSID.  

  while (WiFi.status() != WL_CONNECTED) {      //Wait for getting IP assigned by Access Point/ DHCP. 
                                               //Our case  esp-01 as Access point will assign IP to nodemcu esp12E.
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());             //Check out the Ip assigned by the esp12E
}

void loop() {

  if(Serial.available())
  {
    String inputBuffer = Serial.readStringUntil(']');
    if(inputBuffer.indexOf("color") != -1)
    {
      wifi("color");
    }
  }
                
}//End Loop
void wifi(String topic)
{
 #ifdef DEBUG
              Serial.print("connecting to ");
              Serial.println(host);
          #endif
              // Use WiFiClient class to create TCP connections
              WiFiClient client;
              const int httpPort = 80;
                if (!client.connect("192.168.4.1", httpPort)) {
                  #ifdef DEBUG
                  Serial.println("connection failed");
                  #endif
                  return;
                   }    
              //Request to server to activate the led
              /*client.print(String("GET ") +"/Led"+" HTTP/1.1\r\n" + 
                           "Host: " + host + "\r\n" + 
                           "Connection: close\r\n\r\n");         */
                           
              client.print(String("GET ") +"/"+topic+" HTTP/1.1\r\n" + 
                           "Host: " + host + "\r\n" + 
                           "Connection: close\r\n\r\n");             
              delay(10);
              // Read all the lines of the reply from server and print them to Serial Monitor etc
              while(client.available()){
                String line = client.readStringUntil('\r');
                #ifdef DEBUG
                Serial.print(line);
                #endif
                if(line.indexOf("<MESSAGE>")!=-1)
                {
                  line = line.substring(line.indexOf(">")+1);
                  Serial.print(line);
                }
              }
              //Close the Connection. Automatically
              #ifdef DEBUG
              Serial.println();
              Serial.println("closing connection");
              #endif      
  
}
