#include <Arduino.h>
#define UPDATE_PANEL_INFO_PERIOD 100

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
    byte colorPalate;

    unsigned long lastUpdateRequest;

    bool recievedLastUpdateRequest = true;

    bool shouldAskAgain(){
        bool routineCheckup = recievedLastUpdateRequest && (millis()-lastUpdateRequest) > UPDATE_PANEL_INFO_PERIOD;
        bool retryCooldownEllapsed = (millis()-lastUpdateRequest) > (UPDATE_PANEL_INFO_PERIOD * 10);
        return routineCheckup || retryCooldownEllapsed;
    }

    void updateFromSerial(){
        byte i;

        //general data
        sectionNumber = Serial.parseInt();
        system = Serial.parseInt();
        subsystem = Serial.parseInt();
        lastSubmissionResult = Serial.parseInt();
        lastCompletedSubsystem = Serial.parseInt();

        //jacks
        for(i = 0; i < 10; i++){
            jackStatus[i] = Serial.parseInt();
        }
        for(i = 0; i < 10; i++){
            jackSolution[i] = Serial.parseInt();
        }

        //buttons
        for(i = 0; i < 10; i++){
            buttonStatus[i] = Serial.parseInt();
        }
        for(i = 0; i < 10; i++){
            buttonSolution[i] = Serial.parseInt();
        }
        colorPalate = Serial.parseInt();
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
    byte getColorPalate(){
        return colorPalate;
    }
    byte getLastSubmissionResult(){
        return lastSubmissionResult;
    }
    byte getLastCompletedSubsystem(){
        return lastCompletedSubsystem;
    }
};