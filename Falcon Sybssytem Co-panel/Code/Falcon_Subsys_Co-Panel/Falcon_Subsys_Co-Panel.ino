/*
 * CMSC Falcon Subsystem Co-Panel
 * Written by: Nathan Ford
 * 
 */

#include <LiquidCrystal.h>
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>


#define VersionNumber "Version: 1.4.6 (Release)"
#define Scn1 6
#define Scn2 7
#define rs 8
#define d4 5
#define d5 4
#define d6 3
#define d7 2

#define Bar1Start 0
#define Bar1End 45
#define Bar2Start 46
#define Bar2End 93
#define BAR_PIN 9
#define BAR_COUNT 93

#define FEATHERPIN A2
#define FEATHERCOUNT 99
#define FEATHERGRIDCOUNT 32

#define DelayTimePerLevel 50
#define PixelsPerCircle 12
#define CIRCLE_PIN 10
#define CIRCLE_COUNT 36

#define Screen1Text "Inductor 1"
#define Screen2Text "Inductor 2"

#define slider1 A0
#define slider2 A1

#define circ1button 22
#define circ2button 23
#define circ3button 24

#define circ1buttonlight 25
#define circ2buttonlight 26
#define circ3buttonlight 27

#define screen1Activate 28
#define screen2Activate 29

#define screen1Light 30
#define screen2Light 31

#define dataPin 11
#define latchPin 12
#define clockPin 13


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//INSERT UNIQUE COLORS DOWN HERE (lol as you can tell im not very good at it :) )

//P.S. insert key on computer is overpowered for this

const byte buttonColors[15][3] = {
  {255,  0,  0},//button 1
  {  0,255,  0},//button 2
  {  0,  0,255},//button 3
  {255,255,  0},//button 4
  {255,  0,255},//button 5
  {  0,255,255},//button 6
  {255,255,255},//button 7
  {255,128,128},//button 8
  {128,255,128},//button 9
  {128,128,255},//button 10
  {255,128,  0},//button 11
  {255,  0,128},//button 12
  {128,255,  0},//button 13
  {  0,255,128},//button 14
  {255,200,100},//button 15
  };

const byte fuelColor[3]
  {  0,255,200};

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//venture past this point at your own risk...



//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                               first 5     bottom 5     last 5
const byte animationSize[15] = {7,5,4,5,6   ,5,3,2,3,5   ,6,5,4,5,7};
const byte animationIndexAdd[15][7] = {   //64 is bottom of middle, and yes, these were as hard to aquire as you might think... lol
  {94,93,92,91,90,89,81},{85,86,87,88,81},{84,83,82,81},{77,78,79,80,81},{76,75,74,73,80,81},
  {68,69,70,71,72},      {67,66,65},      {62,63},      {61,60,59},      {54,55,56,57,58},
  {53,52,51,50,48,49},   {45,46,47,48,49},{44,43,42,49},{38,39,40,41,49},{37,36,35,34,33,32,49}
  };
const int middleButtons[15] = {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46};
const int middleBarIndexes[5] = {95,96,97,98,64};
const byte featherBoardGrid[8][4]{
  {24,16,8,0},
  {25,17,9,1},
  {26,18,10,2},
  {27,19,11,3},
  {28,20,12,4},
  {29,21,13,5},
  {30,22,14,6},
  {31,23,15,7},
};

short featherBoardPreRender[8][4];
byte barOffset; 
int maxLoopExecutionTime = 40; //This is the max value I got on 5/28/21 @ 12:31 PM on Version 1.4.6 (Release)
byte animationCount[15];
byte animationsOn[15][3];
unsigned long additiveTurnedOn[15];
unsigned long additiveTurnedOff[15];
unsigned long lastAnimationCycle[3];
int animationCycles[3] = {500,100,250};
byte colorsInQueue[99][3]; //eats memory for breakfast, but greatly simplifies code

byte additivesIn = 0;

bool screen1Active = true;
bool screen2Active = true;

bool oldScreen1Button = false;
bool oldScreen2Button = false;

int barGraphColors[3][3] {
  {0,255,0},
  {0,255,255},
  {255,0,0}
};

byte Screen1Avail = 100;
short Screen1Percent = 0;
byte oldScreen1Percent = 0;
byte Screen2Avail = 100;
short Screen2Percent = 0;
byte oldScreen2Percent = 0;

long timeOfLastScreenUpdates = 0;

long timeOfLastCircleMovement1 = 0;
long timeOfLastCircleMovement2 = 0;
long timeOfLastCircleMovement3 = 0;

byte activePixel1 = 0;
byte activePixel2 = 0;
byte activePixel3 = 0;

byte Speed1 = 2;
byte Speed2 = 6;
byte Speed3 = 9;

long oldPos1;
long oldPos2;
long oldPos3;

long pos1;
long pos2;
long pos3;

short count1 = 0;
short count2 = 0;
short count3 = 0;

bool enable1 = true;
bool enable2 = false;
bool enable3 = true;

bool debounce1;
bool debounce2;
bool debounce3;

byte middleButtonLights[15];
byte oldMiddleButtonLights[15];

unsigned long lastMiddleUpdate = 0;
unsigned long lastButtonCheck = 0;

LiquidCrystal Screen1(rs, Scn1, d4, d5, d6, d7);
LiquidCrystal Screen2(rs, Scn2, d4, d5, d6, d7);

Adafruit_NeoPixel barGraphs(BAR_COUNT, BAR_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel circles(CIRCLE_COUNT, CIRCLE_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel feather(FEATHERCOUNT, FEATHERPIN, NEO_GRB + NEO_KHZ800);

Encoder Rotary1(14,15);
Encoder Rotary2(16,17);
Encoder Rotary3(18,19);

void setup() {//standard initilazations
  Serial.begin(9600);
  Serial.println(VersionNumber);
  Screen1.begin(16,2);
  Screen2.begin(16,2);
  delay(50);
  Screen1.print(Screen1Text);
  Screen2.print(Screen2Text);
  oldPos1 = Rotary1.read();
  oldPos2 = Rotary2.read();
  oldPos3 = Rotary3.read();
  pinMode(slider1, INPUT);
  pinMode(slider2, INPUT);
  pinMode(circ1button, INPUT_PULLUP);
  pinMode(circ2button, INPUT_PULLUP);
  pinMode(circ3button, INPUT_PULLUP);
  pinMode(circ1buttonlight, OUTPUT);
  pinMode(circ2buttonlight, OUTPUT);
  pinMode(circ3buttonlight, OUTPUT);
  digitalWrite(circ1buttonlight, LOW);
  digitalWrite(circ2buttonlight, LOW);
  digitalWrite(circ3buttonlight, LOW);
  pinMode(screen1Activate, INPUT_PULLUP);
  pinMode(screen2Activate, INPUT_PULLUP);
  pinMode(screen1Light, OUTPUT);
  pinMode(screen2Light, OUTPUT);
  digitalWrite(screen1Light, LOW);
  digitalWrite(screen2Light, LOW);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  for(int i = 0; i < 15; i++)
    pinMode(middleButtons[i],INPUT_PULLUP);
  barGraphs.begin();
  barGraphs.show();
  barGraphs.setBrightness(10);
  circles.begin();
  circles.show();
  circles.setBrightness(10);
  circles.setPixelColor(0, circles.Color(255,0,0));
  circles.setPixelColor(PixelsPerCircle, circles.Color(0,255,0));
  circles.setPixelColor(PixelsPerCircle * 2, circles.Color(0,0,255));
  feather.begin();
  feather.show();
  feather.setBrightness(10);
}

void loop() {
  unsigned long startLoop = millis(); // used for calculating how long each loop takes to complete
  Screen1Percent = floor(abs((analogRead(slider1))/1000.0*20))*5; // calculate those percents
  Screen2Percent = floor(abs((analogRead(slider2))/1000.0*20))*5;
  if(Screen1Percent < 0){ // keep it in bounds
    Screen1Percent = 0;
  }
  if(Screen2Percent < 0){
    Screen2Percent = 0;
  }
  updateSpeeds();
  moveCircles();
  if(Screen1Percent != oldScreen1Percent || Screen2Percent != oldScreen2Percent || millis() - timeOfLastScreenUpdates > 1000){ // if changed percents or just hase been a second since last update
    updateBarGraphs(); // update lcd's
    updateLCDs();
    timeOfLastScreenUpdates = millis();
  }
  oldScreen1Percent = Screen1Percent;
  oldScreen2Percent = Screen2Percent;
  
  bool temp1 = digitalRead(screen1Activate);
  bool temp2 = digitalRead(screen2Activate);
  
  if(!temp1 && oldScreen1Button){ // actuate wether inductors are on or not if button is pressed
    screen1Active = !screen1Active;
  }
  if(!temp2 && oldScreen2Button){
    screen2Active = !screen2Active;
  }

  oldScreen1Button = temp1;
  oldScreen2Button = temp2;
  if(millis() - lastButtonCheck > 10){
    lastButtonCheck = millis(); // check fuel additives every 10ms
    checkMiddleButtons();
  }
  if(millis()-startLoop > maxLoopExecutionTime){ // if we set a new record for length of loop to execute (BAD)
    maxLoopExecutionTime = millis() - startLoop;
    Serial.print("New Max loop execution time: ");
    Serial.print(maxLoopExecutionTime);
    Serial.println(" Milliseconds");
  }
}

/*
 * helpful vars for animaiton (may have missed one or two)
 FEATHERCOUNT 99                    max led
 FEATHERGRIDCOUNT 32                where stuff actually happens
 buttonColors[15][3]                colors
 const byte animationSize[15]       how long
const byte animationIndexAdd[15][7] light inexes on lines
const int middleBarIndexes[5]       what the indexes are for the middle scrolling bar
const byte featherBoardGrid[8][4]   pixel grid coordinates to pixel number converter
short featherBoardPreRender[8][4]    pixel grid buffer
byte barOffset                      what keeps it scrolling;
byte animationCount[15]             keeps lines moving
byte animationsOn[15][3]            what sections are on
unsigned long additiveTurnedOn[15]  when was it turned on
unsigned long additiveTurnedOff[15] when it was turned off
unsigned long lastAnimationCycle[3] timer so animations are timed
int animationCycles[3]              how long these delays are for, how fast the engines are narfing down the fuel
byte additivesIn                    number of additives, so we know when to clear everything.
Adafruit_NeoPixel feather(FEATHERCOUNT, FEATHERPIN, NEO_GRB + NEO_KHZ800)
byte colorsInQueue[99][3]           color queue system, uses a lot of memory, but since ive been saving a bunch using shorts and bytes instead of ints in places, i feel less bad, and this GREATLY simplifies it.
 */

void featherAnimation(){
  animationCycles[0] = round(5 * (250 - 50 * (enable1) * ((Speed1)/10.0) - 50 * (enable2) * ((Speed2)/10.0) - 50 * (enable3) * ((Speed3)/10.0)  )); //calculate animation cycle, based on engine output. 
  animationCycles[1] = round(animationCycles[0] / 5); // we know that the lines move 5x slower than the middle ones (saves some cpu power, because arduinos stink at floating point calculations)
  animationCycles[2] = round(animationCycles[0]/2); // we know that the lines move 2x as fast as the feather grid (saves some cpu power, because arduinos stink at floating point calculations)
  bool notStopped = true;
  if((!enable1 && !enable2 && !enable3) && additivesIn == 0){ // if system is off
    notStopped = false;
    if( millis() -  lastAnimationCycle[0] > 100){ // clear it all
      lastAnimationCycle[0] = millis();
      for(int i = 0; i < FEATHERCOUNT; i++){
          colorsInQueue[i][0] = 0;
          colorsInQueue[i][1] = 0;
          colorsInQueue[i][2] = 0;
      }
      pushQueue();
    }
  }
  if(millis() -  lastAnimationCycle[0] > animationCycles[0] && notStopped){ //lines animation loop
    lastAnimationCycle[0] = millis();
    for(int i = FEATHERGRIDCOUNT; i < FEATHERCOUNT-4; i++){
      if(i != 64){
        colorsInQueue[i][0] = 0;
        colorsInQueue[i][1] = 0;
        colorsInQueue[i][2] = 0;
      }
    }
    for(int i = 0; i < 15; i++){
      if(middleButtonLights[i]){
        if(animationsOn[i][0] == false){
          animationsOn[i][0] = true; 
          additiveTurnedOn[i] = millis();
        }
        //yes i know this is a war crime, but i have some sort of issue with booleans, because they are wastefull and due to how it works, 0 == false  and any other value == true, 
        //I generally use bytes instead of bools, because they take same amount of space, and later if we have use for a number to signify something else allong side, it will work
        animationCount[i]++;
        if(animationCount[i] >= animationSize[i]){
          animationCount[i] = 0;
        }
        for(int j = 0; j < animationSize[i]; j++){
          if(j == animationCount[i]){
            for(int k = 0; k < 3; k++){
              short temp = colorsInQueue[animationIndexAdd[i][j]][k] + buttonColors[i][k];
              if(temp > 255){
                temp = 255;
              }
              colorsInQueue[animationIndexAdd[i][j]][k] = temp;
            }
          }
        }
      }
      else{
        if(animationsOn[i][0]){
          animationsOn[i][0] = false;
          additiveTurnedOff[i] = millis();
        }
      }
    }
    pushQueue();
  }
  if(millis() -  lastAnimationCycle[1] > animationCycles[1] && notStopped){
    lastAnimationCycle[1] = millis();
    barOffset++;
    if(barOffset >= 16){
      barOffset = 0;
    }
    for(int i  = 0; i < 15; i++){
      if(!animationsOn[i][1] && animationsOn[i][0] && millis() - additiveTurnedOn[i] >= animationCycles[0] * animationSize[i]){
        animationsOn[i][1] = true;
      }
      if(!animationsOn[i][0] && animationsOn[i][1] && millis() - additiveTurnedOff[i] >= animationCycles[0] * animationSize[i]){
        animationsOn[i][1] = false;
      }
    }
    for(int i = 0; i < 5; i++){
      if(i+barOffset == 15){
        colorsInQueue[middleBarIndexes[i]][0] = fuelColor[0];
        colorsInQueue[middleBarIndexes[i]][1] = fuelColor[1];
        colorsInQueue[middleBarIndexes[i]][2] = fuelColor[2];
      }
      else if(animationsOn[(i + barOffset)%16][1]){
        for(int j = 0; j < 3; j++){
          colorsInQueue[middleBarIndexes[i]][j] = buttonColors[(i + barOffset)%16][j];
        }
      }
      else{
        for(int j = 0; j < 3; j++){
          colorsInQueue[middleBarIndexes[i]][j] = 0;
        }
      }
    }
    pushQueue();
  }
  if(millis() - lastAnimationCycle[2] > animationCycles[2] && notStopped){
    lastAnimationCycle[2] = millis();
    randomSeed(millis());
    for(int i = 0; i < FEATHERGRIDCOUNT; i++){
      for(int x = 0; x < 3; x++){
        colorsInQueue[i][x] = 0;
      }
    }
    for(int x = 0; x < 8; x ++){
      for(int y = 0; y < 4; y++){
        featherBoardPreRender[x][y] = -1;
      }
    }
    for(int i = 0; i < 15; i++){
      if(!animationsOn[i][2] && animationsOn[i][1] && millis() - additiveTurnedOn[i] > (animationCycles[0] * animationSize[i] + animationCycles[1] * 16)){
        animationsOn[i][2] = true;
      }
      if(!animationsOn[i][1] && animationsOn[i][2] && millis() - additiveTurnedOff[i] > (animationCycles[0] * animationSize[i] + animationCycles[1] * 16)){
        animationsOn[i][2] = false;
      }
      if(animationsOn[i][2]){
        bool findingSpot = true;
        while(findingSpot){
          byte _x = random(0,4);
          byte _y = random(0,12) % 8; //this is for weighting purposes of the lower half, so it looks like the fuel is somewhat effected by gravity, we use modulus to keep it in the correct range
          if(featherBoardPreRender[_y][_x] < 0){
            featherBoardPreRender[_y][_x] = i;
            findingSpot = false;
          }
        }
      }
    }
    for(int x = 0; x < 8; x++){
      for(int y = 0; y < 4; y++){
        if(featherBoardPreRender[x][y] >= 0){
          for(int colorNum = 0; colorNum < 3; colorNum++){
            colorsInQueue[featherBoardGrid[7-x][y]][colorNum] = buttonColors[featherBoardPreRender[x][y]][colorNum];
          }
        }
      }
    }
    for(int i = 0; i < 4; i++){
      bool findingSpot = true; // reason i have this go again, is the base fuel isnt one of the additives, and must be added later
      while(findingSpot){
        byte _x = random(0,4);
        byte _y = random(0,12) % 8; //this is for weighting purposes of the lower half, so it looks like the fuel is somewhat effected by gravity, we use modulus to keep it in the correct range
        if(featherBoardPreRender[_y][_x] < 0){
          for(int colorNum = 0; colorNum < 3; colorNum++){
            colorsInQueue[featherBoardGrid[7-_x][_y]][colorNum] = fuelColor[colorNum];
          }
          findingSpot = false;
        }
      }
    }
    pushQueue();
  }
}
void pushQueue(){ // writes colors to neopixels
  for(int i = 0; i < FEATHERCOUNT; i++){
   feather.setPixelColor(i, feather.Color(colorsInQueue[i][0],colorsInQueue[i][1],colorsInQueue[i][2]));
  }
  feather.show();
}

void moveCircles(){
  if(millis() - timeOfLastCircleMovement1 > (10 - Speed1) * DelayTimePerLevel + 25 && enable1){ // if ready for next frame
    activePixel1++; // increment frame
    if(activePixel1 >= PixelsPerCircle){ // keep in bounds
      activePixel1 = 0;
    }
    for(int i = 0; i < PixelsPerCircle; i++){ // clear pixels
      circles.setPixelColor(i + 0, circles.Color(0,0,0));
    }
    circles.setPixelColor(activePixel1 + 0, circles.Color(255,0,0));//set active pixel
    timeOfLastCircleMovement1 = millis();
  }
  if(millis() - timeOfLastCircleMovement2 > (10 - Speed2) * DelayTimePerLevel + 25 && enable2){ // same code, just for next circle
    activePixel2++;
    if(activePixel2 >= PixelsPerCircle){
      activePixel2 = 0;
    }
    for(int i = 0; i < PixelsPerCircle; i++){
      circles.setPixelColor(i + PixelsPerCircle, circles.Color(0,0,0));
    }
    circles.setPixelColor(activePixel2 + PixelsPerCircle, circles.Color(0,255,0));
    timeOfLastCircleMovement2 = millis();
  }
  if(millis() - timeOfLastCircleMovement3 > (10 - Speed3) * DelayTimePerLevel + 25 && enable3){ // same code, just for next circle
    activePixel3++;
    if(activePixel3 >= PixelsPerCircle){
      activePixel3 = 0;
    }
    for(int i = 0; i < PixelsPerCircle; i++){
      circles.setPixelColor(i + PixelsPerCircle * 2, circles.Color(0,0,0));
    }
    circles.setPixelColor(activePixel3 + PixelsPerCircle * 2, circles.Color(0,0,255));
    timeOfLastCircleMovement3 = millis();
  }
  circles.show(); // write to pixels
}

void checkMiddleButtons(){
    additivesIn = 0;// reset additive count
    for(int i = 0; i < 15; i++){//check all additives and store
      bool raw = !digitalRead(middleButtons[i]);
      if(raw && !oldMiddleButtonLights[i]){
        middleButtonLights[i] = !middleButtonLights[i];
      }
      oldMiddleButtonLights[i] = raw; 
      if(middleButtonLights[i]){
        additivesIn++;
      }
    }
    //write to the additive buttons
    shiftOutLEDs(middleButtonLights);
    //animate
    featherAnimation();
}

void shiftOutLEDs(byte _list[]){ // shifting  out data to the shift registers.
  byte _firstHalf = 0;
  byte _secondHalf = 0;
  for(int i = 0; i < 16; i++){
    if(i < 8){
      if(_list[i] > 0){
        bitSet(_firstHalf, i);
      }
    }
    else{
      if(_list[i] > 0){
        bitSet(_secondHalf, i-8);
      }
    }
  }
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin , MSBFIRST, _secondHalf);
  shiftOut(dataPin, clockPin, MSBFIRST, _firstHalf);
  digitalWrite(latchPin, HIGH);
}

byte decideColor(byte _value){ // decides what color to show on bars based on value
  if(_value > 12){
    return 0;
  }
  else if(_value > 7){
    return 1;
  }
  else{
    return 2;
  }
}
void drawBar( int _cs,short _sp, byte _s){ // draws a bar, given starting pixel and value.
  for(short i = _sp; i < _sp + 20; i++){ // for each bar of 20 (3 in between slits)
    if(i - _sp < 10){ // if under 10 just write to pixel number
      if(_s > i-_sp){
        barGraphs.setPixelColor(i, barGraphs.Color(barGraphColors[_cs][0],barGraphColors[_cs][1],barGraphColors[_cs][2]));
      }
      else{
        barGraphs.setPixelColor(i, barGraphs.Color(0,0,0));
      }
    }
    else{ // if over 10 remember to add 3 for the ones in the middle
      if(_s > i-_sp){ 
        barGraphs.setPixelColor(i + 3, barGraphs.Color(barGraphColors[_cs][0],barGraphColors[_cs][1],barGraphColors[_cs][2]));
      }
      else{
        barGraphs.setPixelColor(i + 3, barGraphs.Color(0,0,0));
      }
    }
  }
}
void updateBarGraphs(){// update the bar graphs
  if(Screen1Percent > Screen1Avail){// keep in bounds
    Screen1Percent = Screen1Avail;
  }
  if(Screen2Percent > Screen2Avail){// keep in bounds
    Screen2Percent = Screen2Avail;
  }
  byte bar1a = round(Screen1Percent/5.0); // calculate values for all 4 bars
  byte bar1b = round(Screen1Avail/5.0) - bar1a;
  byte bar2a = round(Screen2Percent/5.0);
  byte bar2b = round(Screen2Avail/5.0) - bar2a;
  byte colorSelect = decideColor(bar1a);
  // draw those bars
  if(screen1Active)
    drawBar( colorSelect, Bar1Start, bar1a);
  else
    drawBar( colorSelect, Bar1Start, 0);
  colorSelect = decideColor(bar1b);
  if(screen1Active)
    drawBar( colorSelect, Bar1Start + 23, bar1b);
  else
    drawBar( colorSelect, Bar1Start + 23, 0);
  colorSelect = decideColor(bar2a);
  if(screen2Active)
    drawBar( colorSelect, Bar2Start, bar2a);
  else
    drawBar( colorSelect, Bar2Start, 0);
  colorSelect = decideColor(bar2b);
  if(screen2Active)
    drawBar( colorSelect, Bar2Start + 23, bar2b); 
  else
    drawBar( colorSelect, Bar2Start + 23, 0); 

  barGraphs.show(); // write to pixels
}

void updateLCDs(){
  Screen1.clear(); // clear the screens
  Screen2.clear();
  if(screen1Active){// if inductor is on
    digitalWrite(screen1Light, HIGH); // turn on button light
    Screen1.setCursor(0,0); // write percent
    Screen1.print(Screen1Text);
    Screen1.print("   ");
    Screen1.setCursor(0,1);
    Screen1.print(Screen1Percent);
    Screen1.print("%   ");
  }
  else{
    digitalWrite(screen1Light, LOW); // turn off button light
    Screen1.setCursor(0,0);
    Screen1.print("Deactivated");// write deactivated
    Screen1.setCursor(0,1);
    Screen1.print("      ");
  }
  
  if(screen2Active){ // same thing for screen 2
    digitalWrite(screen2Light, HIGH);
    Screen2.setCursor(0,0);
    Screen2.print(Screen2Text);
    Screen2.print("   ");
    Screen2.setCursor(0,1);
    Screen2.print((Screen2Percent));
    Screen2.print("%   ");
  }
  else{
    digitalWrite(screen2Light, LOW);
    Screen2.setCursor(0,0);
    Screen2.print("Deactivated");
    Screen2.setCursor(0,1);
    Screen2.print("      ");
  }
}

void updateSpeeds(){ // reads the potentiometers and calcs new engine speed
  bool read1 = digitalRead(circ1button); // get button states
  bool read2 = digitalRead(circ2button);
  bool read3 = digitalRead(circ3button);
  if(!read1 && debounce1){ // get button status
    enable1 = !enable1;
  }
  if(!read2 && debounce2){
    enable2 = !enable2;
  }
  if(!read3 && debounce3){
    enable3 = !enable3;
  }
  if(enable1){ // set button lights
    digitalWrite(circ1buttonlight, HIGH);
  }
  else{
    digitalWrite(circ1buttonlight, LOW);
  }
  if(enable2){
    digitalWrite(circ2buttonlight, HIGH);
  }
  else{
    digitalWrite(circ2buttonlight, LOW);
  }
  if(enable3){
    digitalWrite(circ3buttonlight, HIGH);
  }
  else{
    digitalWrite(circ3buttonlight, LOW);
  }
  debounce1 = read1; // store debounce
  debounce2 = read2;
  debounce3 = read3;
  pos1 = Rotary1.read(); // read rotarys
  pos2 = Rotary2.read();
  pos3 = Rotary3.read();
  count1 += pos1-oldPos1; // add to counts
  count2 += pos2-oldPos2;
  count3 += pos3-oldPos3;
  oldPos1 = pos1; // store old positions
  oldPos2 = pos2;
  oldPos3 = pos3;
  if(count1 >= 4){ // keep them all in bounds
    if(Speed1 < 10){
      Speed1++;
    }
    count1 -= 4;
  }
  else if(count1 <= -4){
    if(Speed1 > 0){
      Speed1--;
    }
    count1 += 4;
  }

  if(count2 >= 4){
    if(Speed2 < 10){
      Speed2++;
    }
    count2 -= 4;
  }
  else if(count2 <= -4){
    if(Speed2 > 0){
      Speed2--;
    }
    count2 += 4;
  }
  
  if(count3 >= 4){
    if(Speed3 < 10){
      Speed3++;
    }
    count3 -= 4;
  }
  else if(count3 <= -4){
    if(Speed3 > 0){
      Speed3--;
    }
    count3 += 4;
  }
}
