/*
 * CMSC Magellan Subsystem Co Panel
 * Programmed and assembled Nathan Ford 2021
 */
#include <Adafruit_NeoPixel.h>

#define micPin A0               // mic pin
#define pixelBarLength 37       // neopixel bar length
#define pixelBarOnePin 2        // neopixel bar 1 pin
#define pixelBarTwoPin 3        // neopixel bar 2 pin
#define pixelRingPin 4          // neopixel ring pin
#define pixelRingLength 12      // neopixel ring length
#define jackLightPin 5          // neopixel jack lights pin
#define jackLightNum 8          // neopixel jack lights length
#define BIGREDBUTTONPIN 9       // BIG RED BUTTON pin
#define bigRedButtonLight 8     // BIG RED BUTTON light
#define defaultRefreshSpeed 250 // base refresh speed
#define osiliscopePixelCount 45 // neopixel osilliscope light count
#define osiliscopePixelPin 6    // neopixel osilliscope light pin
#define VERSIONNUMBER "Magellan Subsystem Co-Panel Version 1.0" // version number

// Red, Yellow, Green
const byte osiliscopeSwitchPins[3] = {A1,A2,A3}; // osilliscope switch pins on arduino
const byte jackColors[8][3] = {{255,0,0},{0,255,0},{0,0,255},{255,255,0},{255,0,255},{0,255,255},{255,255,255},{255,140,0}}; // what color is assigned to each jack
const byte jackPins[8][2] = {{48,49},{46,47},{44,45},{42,43},{40,41},{38,39},{36,37},{34,35}};  // pins on the arduino each jack (second) and jack detect(first) is hooked to
const byte colorSwitches[2][6] = {{22,23,24,25,26,27},{28,29,30,31,32,33}}; // what arduino pin each color switch is hooked to
const byte speedSwitches[2][4] = {{14,15,16,17},{18,19,20,21}};             // what arduino pin each speed switch is hooked to
const byte switchColors[9][3]{  // what color is assigned to each switch.
  {255,0,0},
  {0,255,0},
  {0,0,255},
  {255,255,0},
  {255,0,255},
  {0,255,255},
  {255,255,255},
  {128,128,255},
  {128,255,128},
};

byte osilliscopeColor[15][3]; // graph colors buffer
byte osilliscopeBgd[15][3];   // background colors for graph
byte osilliscopePoints[15];   // record of amplitude of mike
byte middleRingOffset = 0;    // offset of ring
byte middleRingColors[4][3];  // ring colors
unsigned long jacksPlugged[8];// jack is plugged
byte jacksConnectedTo[8];     // jack is plugged to what
byte jackLights[8][3];        // jack light buffer
byte randomStrandColors[pixelBarLength][3];// random strand color buffer
byte stringStrandColors[9][3];//buffer for string strand
unsigned long osilliscopeUpdate = 0;// last update for osilliscope
unsigned long middleRingUpdate = 0; // last middle ring update
unsigned long lastRandomUpdate = 0; // last random bar update
unsigned long lastStringUpdate = 0; // last string bar update
unsigned long lastJackUpdate = 0;   // last reading of the jacks
unsigned long bigRedClearedTime = 0;// when big red button was pressed
byte offSets[2] = {0,0};            // bar offsets
byte bigRedCleared = 0;             // if big red is clearing panel
short bigRedDelaySpeedUp = 0;       // slowdown for the "system reboot"

Adafruit_NeoPixel osclscppx(osiliscopePixelCount,osiliscopePixelPin, NEO_GRB + NEO_KHZ800); // osilliscope neopixels
Adafruit_NeoPixel pixelBar1(pixelBarLength, pixelBarOnePin, NEO_GRB + NEO_KHZ800);          // pixel bar 1
Adafruit_NeoPixel pixelBar2(pixelBarLength, pixelBarTwoPin, NEO_GRB + NEO_KHZ800);          // pixel bar 2
Adafruit_NeoPixel pixelRing(pixelRingLength, pixelRingPin, NEO_GRB + NEO_KHZ800);           // pixel ring
Adafruit_NeoPixel jackPixels(jackLightNum, jackLightPin, NEO_GRB + NEO_KHZ800);             // jack pixels
Adafruit_NeoPixel *lights[5] = {&pixelBar1,&pixelBar2,&pixelRing, &jackPixels, &osclscppx}; // pointer array of all pixel strands

void setup() {
  //init all neopixels
  for(int i = 0; i < 5; i ++){
    (*lights[i]).begin();
    (*lights[i]).clear();
    (*lights[i]).setBrightness(10);
    (*lights[i]).show();
  }
  //set pins to inputs and outputs (pullup means you tell it to use its internal pullup resistor)
  for(int x = 0; x < 2; x++){
    for(int y = 0; y < 6; y++){
      pinMode(colorSwitches[x][y], INPUT_PULLUP);
    }
    for(int y = 0; y < 4; y++){
      pinMode(speedSwitches[x][y], INPUT_PULLUP);
    }
  }
  for(int i = 0; i < 8; i++){
      pinMode(jackPins[i][1],INPUT_PULLUP);
      pinMode(jackPins[i][0],INPUT_PULLUP);
  }
  for(int i = 0; i < 3; i++){
    pinMode(osiliscopeSwitchPins[i],INPUT_PULLUP);
  }
  pinMode(micPin, INPUT);
  pinMode(bigRedButtonLight, OUTPUT);
  pinMode(BIGREDBUTTONPIN, INPUT_PULLUP);
  Serial.begin(9600);// start serial and print version number
  Serial.print(VERSIONNUMBER);
}

void loop() {
  //check if big red button is pressed, and due to pullup resistors, this statement returns true if button isn't pressed and false if it is
  if(digitalRead(BIGREDBUTTONPIN)){
    if(!bigRedCleared){ //if it isn't in a cleared state
      randomLights();//run random strand
      stringLights();// run conveyor strand
      osilliscope(); // run the microphone osilliscope
      if(bigRedDelaySpeedUp ==0){ // if there is no slowdown due to restart after button
        digitalWrite(bigRedButtonLight,LOW); //turn off button light
      }
    }
    else{ // if it is in a cleared state
      if(millis() - bigRedClearedTime > 1000){ // if 1 second has passed since entering
        bigRedCleared = 0; // stop the clear
      }
    }
  }
  else{ // if BIG RED BUTTON is pressed
    digitalWrite(bigRedButtonLight,HIGH); //turn on its light
    bigRedCleared = 1; // turn on the clear
    bigRedClearedTime = millis(); // record current time
    bigRedDelaySpeedUp = 750; // delay to add to make it feel like system is starting up again after button is pressed (makes animation slow down to speed up) 
    pixelBar1.clear(); // Clear all strands for both bars and osilliscope
    pixelBar1.show();
    pixelBar2.clear();
    pixelBar2.show();
    osclscppx.clear();
    osclscppx.show();
    for(int x = 0; x < 15; x++){
      for(int y = 0; y < 3; y++){
        osilliscopeBgd[x][y] = 0;
        osilliscopeColor[x][y] = 0;
      }
    }
  }
  //run jack routine
  jacks();
}

void randomLights(){
  short refreshSpeed = (defaultRefreshSpeed - ( 75 * !digitalRead(speedSwitches[1][2])/2.0)) * 1/(1 + !digitalRead(speedSwitches[1][3])) + bigRedDelaySpeedUp; // calculate time for each animaiton "Frame"
  if(millis() - lastRandomUpdate >= refreshSpeed){ // if enough time has passed to render next 'frame'
    lastRandomUpdate = millis(); //record time of this frame
    for(int x = 0; x < pixelBarLength; x++){ //clear information from last "frame"
      for(int y = 0; y < 3; y++){
        randomStrandColors[x][y] = 0;
      }
    }
    pixelBar2.clear(); // clear the bar
    for(int switchSel = 0; switchSel < 6; switchSel++){ //read the color switches and add them to the buffer
      if(!digitalRead(colorSwitches[1][switchSel])){
        placeInRandArray(switchColors[switchSel][0],switchColors[switchSel][1],switchColors[switchSel][2]); // randomly place in buffer
      }
    }
    for(int SWS = 0; SWS < 3; SWS++){ // check the speed switches that have color adjustments and add them in too
      if(!digitalRead(speedSwitches[1][SWS])){
        placeInRandArray(switchColors[SWS+6][0], switchColors[SWS+6][1], switchColors[SWS+6][2]);// randomly place in buffer
      }
    }
    for(byte lights = 0; lights < pixelBarLength; lights++){ // copy buffer into neopixels
      pixelBar2.setPixelColor(lights, pixelBar2.Color(randomStrandColors[lights][0],randomStrandColors[lights][1],randomStrandColors[lights][2]));
    }
    pixelBar2.show(); // write to pixels
  }
}

void placeInRandArray(byte r, byte g, byte b){
  byte placed = 0;
  while(placed < 2){// until it has been placed in twice into the buffer
    byte valTest = random(0,37); // generate random number
    if(randomStrandColors[valTest][0] + randomStrandColors[valTest][1] + randomStrandColors[valTest][2] == 0){ // check and make sure that slot is empty
      randomStrandColors[valTest][0] = r; //if empty, just slap it in there
      randomStrandColors[valTest][1] = g;
      randomStrandColors[valTest][2] = b;
      placed++;// record it has been placed
    }
  }
}

void stringLights(){
  short refreshSpeed = (defaultRefreshSpeed - ( 75 * !digitalRead(speedSwitches[0][2])/2.0)) * 1/(1 + !digitalRead(speedSwitches[0][3])) + bigRedDelaySpeedUp; // calculate delay for each "frame"
  if(millis() - lastStringUpdate >= refreshSpeed){ // if time has passed and we are ready for next "frame"
    lastStringUpdate = millis(); // record time
    for(int x = 0; x < 9; x++){//clear buffer
      for(int y = 0; y < 3; y++){
        stringStrandColors[x][y] = 0;
      }
    }
    offSets[0]++; // increment the offset
    offSets[0] = offSets[0]%pixelBarLength; // make sure it is in bounds
    for(int i = 0; i < 6; i++){ // read the color switches and place them in buffer
      if(!digitalRead(colorSwitches[0][i])){
        for(int x = 0; x < 3; x++){
          stringStrandColors[i][x] = switchColors[i][x];
        }
      }
    }
    for(int i = 0; i < 3; i++){ // read the speed switches that also adjust color and place them in buffer as well
      if(!digitalRead(speedSwitches[0][i])){
        for(int x = 0; x < 3; x++){
          stringStrandColors[i+6][x] = switchColors[i+6][x];
        }
      }
    }
    pixelBar1.setPixelColor(0, pixelBar1.Color(0,0,0));
    for(int pixel = 0; pixel < pixelBarLength-1; pixel++){// copy buffer into neopixels, but using the offset to make it seem it is moving
      pixelBar1.setPixelColor((pixel+1+offSets[0])%pixelBarLength, pixelBar1.Color(stringStrandColors[pixel%9][0],stringStrandColors[pixel%9][1],stringStrandColors[pixel%9][2]));
    }
    pixelBar1.show();// write to pixels
    if(bigRedDelaySpeedUp > 0){//if we have some delay from the big red button
      bigRedDelaySpeedUp -= 50; // slowly subtract from it to make it seem like we are speeding up
    }
  }
}
void jacks(){
  if(millis() - lastJackUpdate > 10){// if we havnt checked the jacks in 10ms (we check 100 times per second to make it responsive, but not wastefull of compute cycles)
    lastJackUpdate = millis(); // record time
    for(int y = 0; y < 8; y++){ //clear our variables that store connecitons
      jacksConnectedTo[y] = 0;
      for(int x = 0; x < 3; x++){
        jackLights[y][x] = 0;
        middleRingColors[y%4][x] = 0;
      }
    }
    for(int x = 0; x < 8; x++){ // go through each pin
      pinMode(jackPins[x][1], OUTPUT);// set its detect pin to an output and set it LOW
      digitalWrite(jackPins[x][1], LOW);
      delayMicroseconds(1); // give it a millisecond to apply
      if(!digitalRead(jackPins[x][0])){ // if pin is low (not plugged)
        jacksPlugged[x] = 0;
      }
      else{// if it is plugged
        if(jacksPlugged[x] == 0){
          jacksPlugged[x] = millis(); // record time it was plugged
        }
        for(int c = 0; c < 3; c++){ // set its light to the color of that jack
          jackLights[x][c] = jackColors[x][c];
        }
      }
      digitalWrite(jackPins[x][1],HIGH);// set detect pin high
      delayMicroseconds(1); // give it a millisecond to apply
      pinMode(jackPins[x][1], INPUT_PULLUP);// set it back to pullup
    }
    for(int pinFrom = 0; pinFrom < 8; pinFrom++){// now we check for inter-pin connections
      pinMode(jackPins[pinFrom][1], OUTPUT);// set one jack to output and set it low
      digitalWrite(jackPins[pinFrom][1], LOW);
      delayMicroseconds(1)// give it a millisecond to apply
      for(int pinTest = 0; pinTest < 8; pinTest++){ // check other jacks for connections
        if(!digitalRead(jackPins[pinTest][1]) && pinTest != pinFrom && jacksPlugged[pinTest]){ // if it is low (it is connected to home pin), and isnt the home pin
          if(pinFrom < pinTest){ //if this pair hasnt been tried yet
            jacksConnectedTo[pinFrom] = pinTest; // record they are connected
            if(jacksPlugged[pinFrom] == 0 || jacksPlugged[pinFrom] > jacksPlugged[pinTest]){ // if the second pin was plugged first
              for(int c = 0; c < 3; c++){ // set both jacks colors to that of the second one
                jackLights[pinFrom][c] = jackColors[pinTest][c];
                jackLights[pinTest][c] = jackColors[pinTest][c];
              }
              byte tried = 0;
              while(true){ // now add this color to the next available slot in the middle ring
                if(middleRingColors[tried][0] + middleRingColors[tried][1] + middleRingColors[tried][2] == 0){
                  for(int c = 0; c < 3; c++){
                    middleRingColors[tried][c] = jackColors[pinTest][c];
                  }
                  break; 
                }
                tried++;
              }
            }
            else{ // if first pin was plugged first
              for(int c = 0; c < 3; c++){ // set both jack colors to that of the first one
                jackLights[pinFrom][c] = jackColors[pinFrom][c];
                jackLights[pinTest][c] = jackColors[pinFrom][c];
              }
              byte tried = 0;
              while(true){// now add this color to the next available slot in the middle ring
                if(middleRingColors[tried][0] + middleRingColors[tried][1] + middleRingColors[tried][2] == 0){
                  for(int c = 0; c < 3; c++){
                    middleRingColors[tried][c] = jackColors[pinFrom][c];
                  }
                  break; 
                }
                tried++;
              }
            }
          }
        }
      }
      digitalWrite(jackPins[pinFrom][1], HIGH); // set the first pin to HIGH and back to an input with a pullup resistor
      delayMicroseconds(1);
      pinMode(jackPins[pinFrom][1], INPUT_PULLUP);
    }
    for(int pixel = 0; pixel < 8; pixel++){ // copy the jack color array into the neopixels for the jacks
      jackPixels.setPixelColor(pixel, jackPixels.Color(jackLights[pixel][0],jackLights[pixel][1],jackLights[pixel][2]));
    }
    jackPixels.show(); // write to those neopixles
  }
  if(millis() - middleRingUpdate > 500){ // if it has been .5 of a second since last ring "frame" update
    middleRingUpdate = millis(); // record time
    middleRingOffset++; // increment offset
    if(middleRingOffset >= 12){// keep it in bounds
      middleRingOffset = 0;
    }
    for(int pixels = 0; pixels < 12; pixels++){ // copy the middle ring color variable into the neopixels, with the offset (each element is also copied in 3 times so if you have all 4 possible connections plugged, it lights up all lights on the middle circle)
      pixelRing.setPixelColor(pixels, pixelRing.Color(middleRingColors[(pixels + middleRingOffset)%4][0],middleRingColors[(pixels + middleRingOffset)%4][1],middleRingColors[(pixels + middleRingOffset)%4][2]));
    }
    pixelRing.show(); // write to pixels
  }
}

void osilliscope(){
  // read those 3 switches so that we can calculate amplitude and color palate later
  byte rd = !digitalRead(osiliscopeSwitchPins[0]);
  byte yl = !digitalRead(osiliscopeSwitchPins[1]);
  byte gn = !digitalRead(osiliscopeSwitchPins[2]);
  if(millis() - osilliscopeUpdate > 100 + bigRedDelaySpeedUp){ // calculate delay for next "frame"
    osilliscopeUpdate = millis();// record time at start of this frame render, for next one
    for(int x = 14; x > 0; x--){ // shift those values in our "graph" over one and delete the last one to make room for new value
      osilliscopePoints[x] = osilliscopePoints[x-1];
      for(int y = 0; y < 3; y++){
        osilliscopeColor[x][y] = osilliscopeColor[x-1][y];
        osilliscopeBgd[x][y] = osilliscopeBgd[x-1][y];
      }
    }
    //blank bgd
    osilliscopeBgd[0][0] = 0;
    osilliscopeBgd[0][1] = 0;
    osilliscopeBgd[0][2] = 0;
    osilliscopePoints[0] = digitalRead(micPin) * (1 + rd); // record input from the microphone (with amplutude)
    // these next statements with the gn yl and rd variables are just presets for all of your color palates, thats why this part is super long
    if(gn && !yl && !rd){
      //yellow color
      osilliscopeColor[0][0] = 255;
      osilliscopeColor[0][1] = 255;
      osilliscopeColor[0][2] = 0;
    }
    else if(gn && yl && !rd){
      // orange color
      osilliscopeColor[0][0] = 255;
      osilliscopeColor[0][1] = 128;
      osilliscopeColor[0][2] = 0;
      // blue bgd
      osilliscopeBgd[0][0] = 0;
      osilliscopeBgd[0][1] = 0;
      osilliscopeBgd[0][2] = 255;
    }
    else if(gn && !yl && rd){
      // white color
      osilliscopeColor[0][0] = 255;
      osilliscopeColor[0][1] = 255;
      osilliscopeColor[0][2] = 255;
    }
    else if(gn && rd && yl){
      // red color
      osilliscopeColor[0][0] = 255;
      osilliscopeColor[0][1] = 0;
      osilliscopeColor[0][2] = 0;
      // blue bgd
      osilliscopeBgd[0][0] = 0;
      osilliscopeBgd[0][1] = 0;
      osilliscopeBgd[0][2] = 255;
    }
    else if(yl && !rd && !gn) {
      // default (Light blue) color
      osilliscopeColor[0][0] = 0;
      osilliscopeColor[0][1] = 0;
      osilliscopeColor[0][2] = 255;
      // yellow bgd
      osilliscopeBgd[0][0] = 255;
      osilliscopeBgd[0][1] = 255;
      osilliscopeBgd[0][2] = 0;
    }
    else if(yl && rd && !gn){
      // default (Light blue) color
      osilliscopeColor[0][0] = 0;
      osilliscopeColor[0][1] = 255;
      osilliscopeColor[0][2] = 255;
      // purple bgd
      osilliscopeBgd[0][0] = 255;
      osilliscopeBgd[0][1] = 0;
      osilliscopeBgd[0][2] = 255;
    }
    else if(rd && !gn && !yl){
      osilliscopeColor[0][0] = 0;
      osilliscopeColor[0][1] = 255;
      osilliscopeColor[0][2] = 25;
    }
    else{
      osilliscopeColor[0][0] = 0;
      osilliscopeColor[0][1] = 255;
      osilliscopeColor[0][2] = 255;
    }
    // end of the color palates
    osclscppx.clear();// clear the grid
    for(int t = 0; t < 15; t++){ // plot all of the points on the graph also color in the background colors
      if(osilliscopePoints[t] == 0){ // if amplitude is 0
        osclscppx.setPixelColor(t, osclscppx.Color(osilliscopeColor[t][0], osilliscopeColor[t][1], osilliscopeColor[t][2]));

        osclscppx.setPixelColor(29-t, osclscppx.Color(osilliscopeBgd[t][0], osilliscopeBgd[t][1], osilliscopeBgd[t][2]));
        osclscppx.setPixelColor(30+t, osclscppx.Color(osilliscopeBgd[t][0], osilliscopeBgd[t][1], osilliscopeBgd[t][2]));
      }
      else if(osilliscopePoints[t] == 1){ // if amplitude is 1
        osclscppx.setPixelColor(29-t, osclscppx.Color(osilliscopeColor[t][0], osilliscopeColor[t][1], osilliscopeColor[t][2]));

        osclscppx.setPixelColor(t, osclscppx.Color(osilliscopeBgd[t][0], osilliscopeBgd[t][1], osilliscopeBgd[t][2]));
        osclscppx.setPixelColor(30+t, osclscppx.Color(osilliscopeBgd[t][0], osilliscopeBgd[t][1], osilliscopeBgd[t][2]));
      }
      else{ // if amplitude is 2
        osclscppx.setPixelColor(30+t, osclscppx.Color(osilliscopeColor[t][0], osilliscopeColor[t][1], osilliscopeColor[t][2]));

        osclscppx.setPixelColor(t, osclscppx.Color(osilliscopeBgd[t][0], osilliscopeBgd[t][1], osilliscopeBgd[t][2]));
        osclscppx.setPixelColor(29-t, osclscppx.Color(osilliscopeBgd[t][0], osilliscopeBgd[t][1], osilliscopeBgd[t][2]));
      }
    }
    osclscppx.show();// write to pixels
  }
}
