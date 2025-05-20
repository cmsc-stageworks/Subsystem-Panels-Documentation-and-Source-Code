#ifndef MQTT_SERVER_H
#define MQTT_SERVER_H

#include "PanelCommunicator.h"
#include "EthernetLiterals.h"
#include "EEPROM.h"
#include <Ethernet3.h>
#include <MQTT.h>



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
    EthernetServer webServer = EthernetServer(WEB_INTERFACE_PORT);
    char* topicName;
    unsigned long lastGeneralUpdate;
    char* mqttServer = "192.168.000.1";
    static bool shouldUpdate;

    void connect();

    void updateField(FieldToUpdate field);

    void checkHTTP();

    void updateIPFromEEPROM();

    

public:
    static void processMessage(String &topic, String &payload)
    {
        if (topic.equals("Subsystem/req"))
        {
            if (payload.equals("Update"))
            {
                MQTTCommunicator::shouldUpdate = true;
            }
        }
    }

    void init(PanelCommunicator* communicator, char* topicName);

    void tick();

};

#endif