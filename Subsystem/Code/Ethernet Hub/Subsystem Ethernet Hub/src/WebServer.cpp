#include <Ethernet3.h>
#include "EthernetLiterals.h"
#include "PanelCommunicator.h"

class WebServer {
private:
    EthernetServer webServer = EthernetServer(WEB_INTERFACE_PORT);
    PanelCommunicator* panelCommunicator;
    const byte ProfileConversion[12][7] = {
        {0,1,2,3,4,5,6},
        {0,1,4,0,0,0,0},
        {0,1,3,4,0,0,0},
        {0,1,2,3,4,0,0},
        {0,1,2,3,4,5,0},
        {0,4,0,0,0,0,0},
        {0,1,6,0,0,0,0},
        {0,1,4,6,0,0,0},
        {1,3,6,0,0,0,0},
        {0,1,3,4,5,0,0},
        {0,1,2,0,0,0,0},
        {4,6,0,0,0,0,0}};
public:
    void init(PanelCommunicator* desiredCommunicator){
        panelCommunicator = desiredCommunicator;
        webServer.begin();
    }

    void updateClients(){
        EthernetClient client = webServer.available();
        delayMicroseconds(750); // give them a little time to connect fully
        if (client) {
            Serial.println("Satisfying Client");
            // an http request ends with a blank line
            boolean currentLineIsBlank = true;
            while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank) {
                // send a standard http response header
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println("Connection: close");  // the connection will be closed after completion of the response
                client.println("Refresh: 5");  // refresh the page automatically every 5 sec
                client.println();
                #if !MAKE_PARSE_FRIENDLY
                client.print("<style> .cntr{float:left;} .bx{float:left;width:16;height:16;border: 1px solid grey;} .holder{background-color:#737373;padding:7;width:72;height:72;} .c0{background-color:#000000;} .c1{background-color:#FF0000;} .c2{background-color:#FFFF00;} .c3{background-color:#00FF00;} .c4{background-color:#00FFFF;} .c5{background-color:#b378d3;} .c6{background-color:#FFFFFF;} </style>");
                #endif
                #if !MAKE_PARSE_FRIENDLY
                
                #endif
                client.print("<!DOCTYPE HTML>");
                client.print("<html>");
                //client.print(VersionNumber);
                client.print("<br>Subsystem: ");
                client.println(panelCommunicator->getSection());
                client.print("<br>Current Section:");
                client.print(panelCommunicator->getSection());
                #if !MAKE_PARSE_FRIENDLY
                client.print("<br><div class=\"cntr\">");
                #endif
                client.print("Last Check Result: ");
                #if MAKE_PARSE_FRIENDLY
                client.println(lastSubmitCorrectOrIncorrect);
                #else
                if(panelCommunicator->getLastSubmissionResult()){
                    client.println("Correct </div> <div class=\"bx c3\"></div>");
                }
                else{
                    client.println("Incorrect </div> <div class=\"bx c1\"></div>");
                }
                #endif
                if(panelCommunicator->getSection() == 0){
                    client.print("<br>Current System Dial: ");
                    client.print(panelCommunicator->getSubsystem());
                    client.print("<br><br>Last Completed Section: ");
                    #if MAKE_PARSE_FRIENDLY
                    client.print(lastCompletedSubsystem);
                    #else
                    if(panelCommunicator->getLastCompletedSubsystem() >=0 ){
                    client.print(panelCommunicator->getLastCompletedSubsystem()); 
                    }
                    else{
                    client.print("None");
                    }
                    #endif
                }
                else if(panelCommunicator->getSection() == 1){
                    client.println("<br><br>Current Connections:");
                    for(int i = 0; i < 5; i++){
                    client.print("<br>");
                    client.print(panelCommunicator->getJackStatus()[i]);
                    client.print(" is connected to: ");
                    client.print(panelCommunicator->getJackStatus()[i + 5]);
                    }
                    client.print("<br>Desired Connections:");
                    for(int i = 0; i < 5; i++){
                    client.print("<br>");
                    client.print(panelCommunicator->getJackSolution()[i]);
                    client.print(" is connected to: ");
                    client.print(panelCommunicator->getJackSolution()[i + 5]);
                    }
                }
                if(panelCommunicator->getSection() == 2){
                    client.println("<br><br>This is the current button status: ");
                    #if !MAKE_PARSE_FRIENDLY
                    client.print("<div class=\"holder\">");
                    #endif
                    for(int x = 3; x >= 0; x--){
                    if(x != 3){
                        client.print("<br>");
                    }
                    for(int y = 0; y < 4; y++){
                        #if MAKE_PARSE_FRIENDLY
                        client.print(ProfileConversion[colorProfile][buttonValues[y][x]]);
                        #else
                        client.print("<div class=\"bx c");
                        client.print(ProfileConversion[panelCommunicator->getColorPalate()][panelCommunicator->getButtonStatus()[y*4+x]]);
                        client.print("\"></div>");
                        #endif
                    }
                    }
                    #if !MAKE_PARSE_FRIENDLY
                    client.print("</div>");
                    #endif
                    client.println("<br><br>This is the wanted button status: ");
                    #if !MAKE_PARSE_FRIENDLY
                    client.print("<div class=\"holder\">");
                    #endif
                    for(int i = 0; i < 16; i++){
                        if(i%4 == 0 && i > 0){
                        client.print("<br>");
                        }
                        #if MAKE_PARSE_FRIENDLY
                        client.print(endColors[i]);
                        #else
                        client.print("<div class=\"bx c");
                        client.print(panelCommunicator->getButtonSolution()[i]);
                        client.print("\"></div>");
                        #endif
                    }
                    #if !MAKE_PARSE_FRIENDLY
                    client.print("</div>");
                    #endif
                }
                client.println("</html>");
                break;
                }
                if (c == '\n') {
                // you're starting a new line
                currentLineIsBlank = true;
                } else if (c != '\r') {
                // you've gotten a character on the current line
                currentLineIsBlank = false;
                }
            }
            }
            // give the web browser time to receive the data
            delay(1);
            // close the connection:
            client.stop();
        }
    }
};