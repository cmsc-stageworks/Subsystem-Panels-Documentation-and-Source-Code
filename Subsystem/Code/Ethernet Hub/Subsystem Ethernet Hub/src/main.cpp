#include <Arduino.h>
#include <Ethernet3.h>
#include "ErrorCodes.h"
#include "EthernetLiterals.h"
#include "WebServer.h"
#include "PanelCommunicator.h"


#if USE_MQTT
#include <MQTT.h>
#include "MQTTCommunicator.h"

MQTTCommunicator mqttCom;
#endif


#define DEBUG true

#if !USE_MQTT
WebServer webServer;
#endif

PanelCommunicator panelCommunicator;

char* panelName;

void errorCode(int numFlashes, bool repeatForever = true);

void setup() {
  Serial.begin(9600);
  Serial.println(Serial.availableForWrite());
  delay(250);
  while(Serial.available()){
    Serial.read();
  }

  Serial.println("Initialize");
  while(!Serial.available());
  String panelResponse = Serial.readStringUntil('\n');
  panelResponse.replace("\r","");
  if(!panelResponse.equals("OK")){
    Serial.print("Unidentified Command: ");
    Serial.println(panelResponse);
    for(byte i = 0; i < panelResponse.length(); i++){
      Serial.print(panelResponse[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
    errorCode(INVALID_PANEL);
  }

  Serial.println("Identify");
  while(!Serial.available());
  String panelNameStr = Serial.readStringUntil('\n');
  panelName = new char[panelNameStr.length()+1];
  strcpy(panelName, panelNameStr.c_str());

  #if DEBUG
  Serial.print("Panel Name: ");
  Serial.println(panelName);
  #endif
  

  byte mac[6] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED //default address, overwriten with panel name
  };

  for(byte i = 0; i < panelNameStr.length() && i < 6; i++){
    mac[i] = panelNameStr[i];
  }

  #if DEBUG
  Serial.print("Created Mac Address: ");
  for(int i = 0; i < 6; i++){
    Serial.print(mac[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  #endif

  Ethernet.setHostname(panelName);
  Ethernet.setCsPin(ETHERNET_CS);

  Serial.println("Starting Ethernet");

  while(!Ethernet.begin(mac)){
    Serial.println("Failed to initialize Ethernet!");
    errorCode(NO_ETHERNET, false);
  }

  #if DEBUG
    Serial.println("connection established, starting server:");
  #endif
  
  #if !USE_MQTT
  webServer.init(&panelCommunicator, panelName);
  #endif

  #if DEBUG
    Serial.print("server is at ");
    Serial.print(Ethernet.localIP());
    Serial.print(':');
    Serial.println(WEB_INTERFACE_PORT);
  #endif

  #if USE_MQTT
    mqttCom.init(&panelCommunicator, panelName);
  #endif
}

void loop() {
  panelCommunicator.tick();
  #if USE_MQTT
    mqttCom.tick();
  #else
    webServer.updateClients();
  #endif
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