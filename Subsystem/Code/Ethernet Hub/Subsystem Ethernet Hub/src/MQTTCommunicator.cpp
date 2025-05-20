#include "MQTTCommunicator.h"

#if USE_MQTT

#define MQTT_UPDATE_INTERVAL 60000 /*Once a minute*/

#define EEPROM_SIZE 8

bool MQTTCommunicator::shouldUpdate = false;

void MQTTCommunicator::connect() 
{
#if DEBUG
    Serial.print("connecting...");
#endif
    uint8_t numAttempts = 0;
    while (!client.connect(topicName, "space", "BeamMeUp"))
    {
        Serial.print(".");
        numAttempts++;
        checkHTTP();
        if (numAttempts > 5)
        {
            client.disconnect();
            init(panelCommunicator, topicName);
        }
        for (int i = 0; i < 1000; i++)
        {
            checkHTTP();
            delay(10);
            if (client.connected())
            {
                client.subscribe("Subsystem/req");
                return;
            }
        }
    }
    client.subscribe("Subsystem/req");

#if DEBUG
    Serial.println("\nconnected!");
#endif
}

void MQTTCommunicator::updateField(FieldToUpdate field) 
{
    String payload = "";
    String topic = String("Subsystem");
    switch (field)
    {
    case JACKS:
        for (int i = 0; i < 10; i++)
        {
            payload = payload + panelCommunicator->getJackStatus()[i] + (i % 2 ? ',' : '-');
        }
        topic = topic + "/Jacks";
        break;
    case JACKSOLUTION:
        for (int i = 0; i < 10; i++)
        {
            payload = payload + panelCommunicator->getJackSolution()[i] + (i % 2 ? ',' : '-');
        }
        topic = topic + "/JackSolution";
        break;
    case BUTTONS:
        for (int i = 0; i < 16; i++)
        {
            payload = payload + panelCommunicator->getButtonStatus()[i] + (i % 4 == 3 ? ';' : ',');
        }
        topic = topic + "/Buttons";
        break;
    case BUTTONSOLUTION:
        for (int i = 0; i < 16; i++)
        {
            payload = payload + panelCommunicator->getButtonSolution()[i] + (i % 4 == 3 ? ';' : ',');
        }
        topic = topic + "/ButtonsSolution";
        break;
    case SYSTEM:
        payload = payload + panelCommunicator->getSystem();
        topic = topic + "/System";
        break;
    case SECTION:
        payload = payload + panelCommunicator->getSection();
        topic = topic + "/Section";
        break;
    case SUBSYSTEM:
        payload = payload + panelCommunicator->getSubsystem();
        topic = topic + "/Subsystem";
        break;
    case SUBMISSIONRESULT:
        payload = payload + panelCommunicator->getLastSubmissionResult();
        topic = topic + "/SubmissionResult";
        break;
    case COMPLETEDSYSTEM:
        payload = payload + panelCommunicator->getLastCompletedSubsystem();
        topic = topic + "/CompletedSystem";
        break;
    }
    client.publish(topic, payload);
}

void MQTTCommunicator::checkHTTP()
{
    EthernetClient client = webServer.available();
    if (client)
    {
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        boolean IPUpdated = false;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close"); // the connection will be closed after completion of the response
                    client.println();
                    if (IPUpdated)
                    {
                        client.println("<h3>IP updated<h3>");
                        client.println(mqttServer);
                    }
                    else
                    {
                        client.println("\n<h3>Subsystem MQTT setup</h3>");
                        client.print("current IP: ");
                        client.println(mqttServer);
                        client.println("<form>\n<input type=\"text\" name=\"ip\">\n<button>Submit</button>\n</form>");
                    }
                    client.println();
                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                }
                else if (c == '?')
                {
                    if (client.read() == 'i' && client.read() == 'p' && client.read() == '=')
                    {
                        char currIp[4];
                        for (uint8_t currIPdigit = 0; currIPdigit < 4; currIPdigit++)
                        {
                            uint8_t currDigit = client.parseInt();
                            if (currDigit != 0 || currIPdigit != 0)
                            {
                                EEPROM.write(1 + currIPdigit, currDigit);
                                Serial.println(EEPROM.read(1 + currIPdigit));
                                if (currIPdigit == 3)
                                {
                                    IPUpdated = true;
                                    updateIPFromEEPROM();
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }

        // give the web browser time to receive the data
        delay(10);
        // close the connection:
        client.stop();

        if (IPUpdated)
        {
            init(panelCommunicator, topicName);
        }

        checkHTTP(); // check for subsequent client requests (needed for when bad IPs are entered)
    }
}

void MQTTCommunicator::updateIPFromEEPROM()
{
    uint8_t strPos = 0;
    for (int i = 1; i < 5; i++)
    {
        uint8_t currByte = EEPROM.read(i);
        itoa(currByte, mqttServer + strPos, 10);
        if (currByte < 10)
        {
            strPos += 2;
        }
        else if (currByte < 100)
        {
            strPos += 3;
        }
        else
        {
            strPos += 4;
        }
        mqttServer[strPos - 1] = '.';
    }
    mqttServer[strPos - 1] = '\0';
}


void MQTTCommunicator::init(PanelCommunicator *communicator, char *topicName)
{
    panelCommunicator = communicator;
    this->topicName = topicName;

    if (EEPROM.read(0) != 0xAB)
    { // magic number to set EEPROM to to make sure the address it reads will be valid (check to see if formatted)
        EEPROM.write(0, 0xAB);
        EEPROM.write(1, 192);
        EEPROM.write(2, 168);
        EEPROM.write(3, 0);
        EEPROM.write(4, 162);
    }
    updateIPFromEEPROM();
#if DEBUG
    Serial.print("Connecting to server: ");
    Serial.println(mqttServer);
#endif
    webServer.begin();
    client.begin(mqttServer, net);
    client.onMessage(this->processMessage);

    connect();
}

void MQTTCommunicator::tick()
{
    checkHTTP();
    client.loop();

    if (!client.connected())
    {
        connect();
    }

    if (!lastGeneralUpdate || millis() - lastGeneralUpdate > MQTT_UPDATE_INTERVAL || millis() < lastGeneralUpdate || shouldUpdate)
    {
        lastGeneralUpdate = millis();
        for (byte i = 0; i < COMPLETEDSYSTEM; i++)
        { // we don't want to update completed system so that thorium knows when the user actually has finished a system
            updateField((FieldToUpdate)i);
        }
        if (shouldUpdate)
        {
            shouldUpdate = false;
        }
    }
    else
    {
        if (panelCommunicator->jackChanged())
        {
            updateField(JACKS);
        }
        if (panelCommunicator->jackSolutionChanged())
        {
            updateField(JACKSOLUTION);
        }
        if (panelCommunicator->buttonSolutionChanged())
        {
            updateField(BUTTONSOLUTION);
        }
        if (panelCommunicator->buttonStatusChanged())
        {
            updateField(BUTTONS);
        }
        if (panelCommunicator->systemChanged())
        {
            updateField(SYSTEM);
        }
        if (panelCommunicator->sectionChanged())
        {
            updateField(SECTION);
        }
        if (panelCommunicator->subsystemChanged())
        {
            updateField(SUBSYSTEM);
        }
        if (panelCommunicator->lastSubmissionResultChanged())
        {
            updateField(SUBMISSIONRESULT);
        }
        if (panelCommunicator->lastCompletedSubsystemChanged())
        {
            updateField(COMPLETEDSYSTEM);
        }
    }
}

#endif