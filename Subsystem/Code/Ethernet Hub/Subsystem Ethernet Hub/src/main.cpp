#include <Arduino.h>
#include <Ethernet3.h>
#include "ErrorCodes.h"
#include "EthernetLiterals.h"
#include "WebServer.h"
#include "PanelCommunicator.h"

#define DEBUG true

WebServer webServer;

PanelCommunicator panelCommunicator;

void errorCode(int numFlashes, bool repeatForever = true);

void setup() {
  Serial.begin(9600);
  delay(250);
  while(Serial.available()){
    Serial.read();
  }

  Serial.println("Initialize");
  while(!Serial.available());
  String panelResponse = Serial.readStringUntil('\n');
  if(!panelResponse.equals("OK")){
    errorCode(INVALID_PANEL);
  }

  Serial.println("Identify");
  while(!Serial.available());
  String panelName = Serial.readStringUntil('\n');
  

  byte mac[6] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED //default address, overwriten with panel name
  };

  for(byte i = 0; i < panelName.length() && i < 6; i++){
    mac[i] = panelName[i];
  }

  Ethernet.setCsPin(ETHERNET_CS); 

  while(!Ethernet.begin(mac)){
    errorCode(NO_ETHERNET, false);
  }

  #if DEBUG
    Serial.println("connection established, starting server:");
  #endif

  webServer.init(&panelCommunicator);

  #if DEBUG
    Serial.print("server is at ");
    Serial.print(Ethernet.localIP());
    Serial.print(':');
    Serial.println(WEB_INTERFACE_PORT);
  #endif
}

void loop() {
  panelCommunicator.updateFromSerial();
  webServer.updateClients();
}


void errorCode(int numFlashes, bool repeatForever){
  pinMode(LED_BUILTIN, OUTPUT);
  do {
    for(int i = 0; i < numFlashes; i++){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
    delay(1000);
  } while(repeatForever);
}