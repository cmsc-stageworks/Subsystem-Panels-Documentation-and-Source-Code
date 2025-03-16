#include <Ethernet3.h>
#include <MQTT.h>
#include "PanelCommunicator.h"
#include "EthernetLiterals.h"

#if USE_MQTT

#define MQTT_UPDATE_INTERVAL 60000 /*Once a minute*/

class MQTTCommunicator{
private:
    enum FieldToUpdate:byte{
        JACKS,
        JACKSOLUTION,
        BUTTONS,
        BUTTONSOLUTION,
        SYSTEM,
        SECTION,
        SUBSYSTEM,
        SUBMISSIONRESULT,
        COMPLETEDSYSTEM
    };

    EthernetClient net;
    MQTTClient client;
    PanelCommunicator* panelCommunicator;
    char* topicName;
    unsigned long lastGeneralUpdate;

    void connect() {
      Serial.print("connecting...");
      while (!client.connect("arduino", "public", "public")) {
        Serial.print(".");
        delay(1000);
      }
      
      Serial.println("\nconnected!");
    
      client.subscribe("/hello");
      // client.unsubscribe("/hello");
    }

    void updateField(FieldToUpdate field){
        String payload = "";
        String topic = String(topicName);
        switch(field){
            case JACKS:
                for(int i = 0; i < 10; i++){
                    payload = payload + panelCommunicator->getJackStatus()[i] + (i % 2 ? ';' : ',');
                }
                topic = topic + "/Jacks";
            break; case JACKSOLUTION:
                for(int i = 0; i < 10; i++){
                    payload = payload + panelCommunicator->getJackSolution()[i] + (i % 2 ? ';' : ',');
                }
                topic = topic + "/JackSolution";
            break; case BUTTONS:

            break; case BUTTONSOLUTION:

            break; case SYSTEM:
                payload = payload + panelCommunicator->getSystem();
                topic = topic + "/System";
            break; case SECTION:
                payload = payload + panelCommunicator->getSection();
                topic = topic + "/Section";
            break; case SUBSYSTEM:
            payload = payload + panelCommunicator->getSubsystem();
                topic = topic + "/Subsystem";
            break; case SUBMISSIONRESULT:
            payload = payload + panelCommunicator->getLastSubmissionResult();
                topic = topic + "/SubmissionResult";
            break; case COMPLETEDSYSTEM:
            payload = payload + panelCommunicator->getLastCompletedSubsystem();
                topic = topic + "/CompletedSystem";
            break;
        }
        client.publish(topic, payload);
    }

    

public:
    static void processMessage(String &topic, String &payload){
            
    }

    void init(PanelCommunicator* communicator, char* topicName){
        panelCommunicator = communicator;
        this->topicName = topicName;
        client.begin("public.cloud.shiftr.io", net);
        client.onMessage(this->processMessage);

        connect();
    }

    void tick(){
        client.loop();

        if (!client.connected()) {
            connect();
        }
        
        if(!lastGeneralUpdate || millis()-lastGeneralUpdate > MQTT_UPDATE_INTERVAL || millis() < lastGeneralUpdate){
            lastGeneralUpdate = millis();
            for(byte i = 0; i < COMPLETEDSYSTEM; i++){ //we don't want to update completed system so that thorium knows when the user actually has finished a system
                updateField((FieldToUpdate) i);
            }
        }
        else{
            if(panelCommunicator->jackChanged()){
                updateField(JACKS);
            }
            if(panelCommunicator->jackSolutionChanged()){
                updateField(JACKSOLUTION);
            }
            if(panelCommunicator->systemChanged()){
                updateField(SYSTEM);
            }
            if(panelCommunicator->sectionChanged()){
                updateField(SECTION);
            }
            if(panelCommunicator->subsystemChanged()){
                updateField(SUBSYSTEM);
            }
            if(panelCommunicator->lastSubmissionResultChanged()){
                updateField(SUBMISSIONRESULT);
            }
            if(panelCommunicator->lastCompletedSubsystemChanged()){
                updateField(COMPLETEDSYSTEM);
            }
        }
    }

};

#endif