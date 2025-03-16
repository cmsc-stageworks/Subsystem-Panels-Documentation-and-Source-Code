#include <Arduino.h>
#include "EthernetLiterals.h"
#define UPDATE_PANEL_INFO_PERIOD 1000

class PanelCommunicator{
private:
    byte sectionNumber;
    byte system;
    byte subsystem;
    byte lastSubmissionResult;
    byte lastCompletedSubsystem;

    byte jackStatus[10];
    byte jackSolution[10];

    byte buttonStatus[16];
    byte buttonSolution[16];

    #if USE_MQTT
        byte oldSectionNumber, oldSystem, oldSubsystem, oldLastSubmissionResult, oldLastCompletedSubsystem;
        byte oldJackStatus[10];
        byte oldJackSolution[10];
        byte oldButtonStatus[16];
        byte oldButtonSolution[16];
    #endif

    unsigned long lastUpdateRequest;

    bool recievedLastUpdateRequest = true;

    bool shouldAskAgain(){
        bool routineCheckup = recievedLastUpdateRequest && (millis()-lastUpdateRequest) > UPDATE_PANEL_INFO_PERIOD;
        bool retryCooldownEllapsed = (millis()-lastUpdateRequest) > (UPDATE_PANEL_INFO_PERIOD * 10);
        return routineCheckup || retryCooldownEllapsed;
    }

    void updateFromSerial(){
        byte i;

        char command[3] = {' ',' ','\0'};
        command[0] = Serial.read();
        if(command[0] == '\n'){
            return;
        }
        command[1] = Serial.read();
        if(command[1] == '\n'){
            return;
        }
        Serial.read();
        if(command[0] != '$' && command[1] != 'u'){
            while(Serial.available() && command[0] != '$' && command[1] != 'u'){
                command[0] = command[1];
                command[1] = Serial.read();
                if(command[1] == '\n'){
                    return;
                }
            }
        }

        //general data
        #if USE_MQTT
        oldSectionNumber = sectionNumber;
        oldSystem = system;
        oldSubsystem = subsystem;
        oldLastSubmissionResult = lastSubmissionResult;
        oldLastCompletedSubsystem = lastCompletedSubsystem;
        #endif

        sectionNumber = Serial.parseInt();
        system = Serial.parseInt();
        subsystem = Serial.parseInt();
        lastSubmissionResult = Serial.parseInt();
        lastCompletedSubsystem = Serial.parseInt();

        //jacks
        for(i = 0; i < 10; i++){
            #if USE_MQTT
            oldJackStatus[i] = jackStatus[i];
            #endif
            jackStatus[i] = Serial.parseInt();
        }
        for(i = 0; i < 10; i++){
            #if USE_MQTT
            oldJackSolution[i] = jackSolution[i];
            #endif
            jackSolution[i] = Serial.parseInt();
        }

        //buttons
        for(i = 0; i < 10; i++){
            #if USE_MQTT
            oldButtonStatus[i] = buttonStatus[i];
            #endif
            buttonStatus[i] = Serial.parseInt();
        }
        for(i = 0; i < 10; i++){
            #if USE_MQTT
            oldButtonSolution[i] = buttonSolution[i];
            #endif
            buttonSolution[i] = Serial.parseInt();
        }
    }
public:
    void tick(){
        if(shouldAskAgain()){
            lastUpdateRequest = millis();
            Serial.println("Update");
            recievedLastUpdateRequest = false;
        }
        if(Serial.available()){ // if we have a message waiting
            updateFromSerial();
            recievedLastUpdateRequest = true;
        }
    }
    #if USE_MQTT
        bool sectionChanged(){
            return sectionNumber != oldSectionNumber;
        }
        bool systemChanged(){
            return system != oldSystem;
        }
        bool subsystemChanged(){
            return subsystem != oldSubsystem;
        }
        bool jackChanged(){
            for(byte i = 0; i < 10; i++){
                if(jackStatus[i] != oldJackStatus[i]){
                    return true;
                }
            }
            return false;
        }
        bool jackSolutionChanged(){
            for(byte i = 0; i < 10; i++){
                if(jackSolution[i] != oldJackSolution[i]){
                    return true;
                }
            }
            return false;
        }
        bool buttonStatusChanged(){
            for(byte i = 0; i < 16; i++){
                if(buttonStatus[i] != oldButtonStatus[i]){
                    return true;
                }
            }
            return false;
        }
        bool buttonSolutionChanged(){
            for(byte i = 0; i < 16; i++){
                if(buttonSolution[i] != oldButtonSolution[i]){
                    return true;
                }
            }
            return false;
        }
        bool lastSubmissionResultChanged(){
            return lastSubmissionResult != oldLastSubmissionResult;
        }
        bool lastCompletedSubsystemChanged(){
            return lastCompletedSubsystem != oldLastCompletedSubsystem;
        }
    #endif
    byte getSection(){
        return sectionNumber;
    }
    byte getSystem(){
        return system;
    }
    byte getSubsystem(){
        return subsystem;
    }
    byte* getJackStatus(){
        return jackStatus;
    }
    byte* getJackSolution(){
        return jackSolution;
    }
    byte* getButtonStatus(){
        return buttonStatus;
    }
    byte* getButtonSolution(){
        return buttonSolution;
    }
    byte getLastSubmissionResult(){
        return lastSubmissionResult;
    }
    byte getLastCompletedSubsystem(){
        return lastCompletedSubsystem;
    }
};