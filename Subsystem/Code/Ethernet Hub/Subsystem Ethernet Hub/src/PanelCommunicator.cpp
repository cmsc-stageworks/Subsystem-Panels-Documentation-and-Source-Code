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
        for(i = 0; i < 16; i++){
            #if USE_MQTT
            oldButtonStatus[i] = buttonStatus[i];
            #endif
            buttonStatus[i] = Serial.parseInt();
        }
        for(i = 0; i < 16; i++){
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
        if(Serial.available() > 50){ // if we have a message waiting
            updateFromSerial();
            recievedLastUpdateRequest = true;
        }
    }
    #if USE_MQTT
        bool sectionChanged(){
            bool hasChanged = sectionNumber != oldSectionNumber;
            oldSectionNumber = sectionNumber;
            return hasChanged;
        }
        bool systemChanged(){
            bool hasChanged = system != oldSystem;
            oldSystem = system;
            return hasChanged;
        }
        bool subsystemChanged(){
            bool hasChanged = subsystem != oldSubsystem;
            oldSubsystem = subsystem;
            return hasChanged;
        }
        bool jackChanged(){
            bool hasChanged = false;
            for(byte i = 0; i < 10; i++){
                if(jackStatus[i] != oldJackStatus[i]){
                    hasChanged = true;
                    oldJackStatus[i] = jackStatus[i];
                }
            }
            return hasChanged;
        }
        bool jackSolutionChanged(){
            bool hasChanged = false;
            for(byte i = 0; i < 10; i++){
                if(jackSolution[i] != oldJackSolution[i]){
                    hasChanged = true;
                    oldJackSolution[i] = jackSolution[i];
                }
            }
            return hasChanged;
        }
        bool buttonStatusChanged(){
            bool hasChanged = false;
            for(byte i = 0; i < 16; i++){
                if(buttonStatus[i] != oldButtonStatus[i]){
                    hasChanged = true;
                    oldButtonStatus[i] = buttonStatus[i];
                }
            }
            return hasChanged;
        }
        bool buttonSolutionChanged(){
            bool hasChanged = false;
            for(byte i = 0; i < 16; i++){
                if(buttonSolution[i] != oldButtonSolution[i]){
                    hasChanged = true;
                    oldButtonSolution[i] = buttonSolution[i];
                }
            }
            return hasChanged;
        }
        bool lastSubmissionResultChanged(){
            bool hasChanged = lastSubmissionResult != oldLastSubmissionResult;
            oldLastSubmissionResult = lastSubmissionResult;
            return hasChanged;
        }
        bool lastCompletedSubsystemChanged(){
            bool hasChanged = lastCompletedSubsystem != oldLastCompletedSubsystem;
            oldLastCompletedSubsystem = lastCompletedSubsystem;
            return hasChanged;
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