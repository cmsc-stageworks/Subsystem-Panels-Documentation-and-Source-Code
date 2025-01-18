/*
  CMSC Galileo Subsytem Co-Panel
  Panel Layout Designed by Mr. Porter
  Panel Code Written and Wired by Nathan Ford
  JUN 2021
*/
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <Encoder.h>
//------------------------------------------------------------------------------------------------------------------
//       --Settings For changing--

// Dice changers, 16 different combonations, INSERT on windows will be your best friend, unless mac also has it too
// how to write what state each light is in goes, {Top Left, Top Right, Left Neopixel, Middle Neopixel, Right Neopixel, Bottom Left, Bottom Right}
//ASCII art to help illistrate:
// []   []    top left and right
//  o o o     neopixels
// []   []    bottom left and right
// values to find each number:
// top left:     +1
// top right:    +2
// bottom left:  +4
// bottom right: +8

// yes, it is binary :)
// sorry, this is the best way I could think of doing it, but may seem like a hot mess at first so i will try and put comments to clear it up
// and yes, if you are the first to come here, these are pretty much all just random values XD, Im not that creative
// anyways, good luck!
/* Helpful hints and explanations from others ---
 *  
 *  
 *  
 *  
 */
 
const byte DiceLightStat[2][16][7]{
  //Die 1
  { // light stat         buttons pressed
    {0,0,0,0,0,0,0},   //all buttons off
    {1,0,0,0,0,0,0},   // top left on, rest off
    {0,1,0,0,0,0,0},   // top right on, rest off
    {1,1,0,0,0,0,0},   // top left and right on, rest off
    {0,0,0,0,0,1,0},   // bottom left on, rest off
    {0,0,1,1,1,0,0},   // bottom left and top left on, rest off
    {0,1,0,1,0,1,0},   // bottom left and top right on, rest off
    {0,0,1,0,1,0,1},   // bottom left, top right, top left on, rest off
    {0,0,0,0,0,0,1},   // bottom right on, rest off
    {1,0,0,1,0,0,1},   // bottom right and top left on, rest off
    {0,1,0,1,0,1,0},   // bottom right and top right on, rest off
    {0,1,0,1,1,1,0},   // bottom right, top right and top left on, rest off
    {0,0,0,1,1,1,0},   // bottom right and bottom left on, rest off
    {1,1,1,1,1,1,1},   // bottom right, bottom left and top left on, rest off
    {0,1,1,1,1,1,0},   // bottom right, bottom left and top right on, rest off
    {1,1,1,0,1,1,1}    // all on
  },

  //Die2
  {
    {0,0,0,0,0,0,0},   //all buttons off
    {1,0,0,0,0,0,0},   // top left on, rest off
    {0,1,0,0,0,0,0},   // top right on, rest off
    {1,1,0,0,0,0,0},   // top left and right on, rest off
    {0,0,0,0,0,1,0},   // bottom left on, rest off
    {0,0,1,1,1,0,0},   // bottom left and top left on, rest off
    {0,1,0,1,0,1,0},   // bottom left and top right on, rest off
    {0,0,1,0,1,0,1},   // bottom left, top right, top left on, rest off
    {0,0,0,0,0,0,1},   // bottom right on, rest off
    {1,0,0,1,0,0,1},   // bottom right and top left on, rest off
    {0,1,0,1,0,1,0},   // bottom right and top right on, rest off
    {0,1,0,1,1,1,0},   // bottom right, top right and top left on, rest off
    {0,0,0,1,1,1,0},   // bottom right and bottom left on, rest off
    {1,1,1,1,1,1,1},   // bottom right, bottom left and top left on, rest off
    {0,1,1,1,1,1,0},   // bottom right, bottom left and top right on, rest off
    {1,1,1,0,1,1,1}    // all on
  }
};

//color for neopixel for each Dice combo (wont effect anything when neopixel insides aren't being used)
// INSERT will be your best friend (if you are using windows, if you are on mac you are on your own... unless it behaves the same)
const byte DiceColors[16][3]{
  {255,  0,  0},   //all buttons off
  {  0,255,  0},   // top left on, rest off
  {  0,  0,255},   // top right on, rest off
  {255,255,  0},   // top left and right on, rest off
  {255,  0,255},   // bottom left on, rest off
  {  0,255,255},   // bottom left and top left on, rest off
  {255,  0,  0},   // bottom left and top right on, rest off
  {  0,255,  0},   // bottom left, top right, top left on, rest off
  {  0,  0,255},   // bottom right on, rest off
  {255,255,  0},   // bottom right and top left on, rest off
  {255,  0,255},   // bottom right and top right on, rest off
  {  0,255,255},   // bottom right, top right and top left on, rest off
  {255,  0,  0},   // bottom right and bottom left on, rest off
  {  0,255,  0},   // bottom right, bottom left and top left on, rest off
  {  0,  0,255},   // bottom right, bottom left and top right on, rest off
  {255,255,  0}    // all on
};

//tells if  each of the switches on the program columbs flashes its color
const byte programFlash[3][5] = {
  {1,0,0,0,0},
  {0,0,1,0,0},
  {0,0,0,1,1}
};
//color of each switch in program colombs (and yes, last row I just threw random numbers in)
const byte programColor[3][5][3]{
  {{  0,255,  0},{255,  0,  0},{  0,  0,255},{255,  0,255},{255,255,  0}},
  {{255,  0,  0},{  0,255,  0},{  0,  0,255},{  0,255,255},{255,255,255}},
  {{  0,255,111},{255,  0,221},{  0,174,255},{255,  0,255},{  0,255, 25}}
};



// Its dangerous to go alone past this point, good luck

//------------------------------------------------------------------------------------------------------------------



#define PIXELPIN 4
#define NUMPIXELS 60
#define ENC1_CLK 2
#define ENC1_DIO 3
#define ENC2_CLK 18
#define ENC2_DIO 19
#define ENC3_CLK 20
#define ENC3_DIO 21
#define BLINKDELAY 500

#define buttonShiftRegisterOffset 6
#define jackPlugCheckStartPin 35
#define DieNeopixelStart 36
#define ProgramNeopixelStart 42
#define jackNeopixelStartOffset 21
#define PROGRAMLIGHTSDELAY 1500
#define PROGRAMFLASHDELAY 125

const byte programSwitchPins[15] = {22,23,24,25,26,27,28,29,30,31,32,33,34,14,15};
const byte jackPlugDetectPins[15] = {A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14};
const byte jackNeopixel[15] ={
  23,22,21,28,27,29,26,25,24,32,31,30,35,34,33
};
const byte buttonGridPins[2][4] = {
  {A15,16,17,12},
  {8,9,10,11}
};
const byte LRAdapt[4] = {
  1,0,3,2
};
//22,21,20,27,26,28,25,24,23,31,30,29,34,33,32
#define dataPin  51
#define latchPin  13
#define clockPin  52

bool bypassed[15];
byte jackSpecialColor[15];
byte currJackCommand;
byte jackAnimationExit;
unsigned long jackCommandStart;
byte jackAnimationTracker;
byte numFlashProg[3];
unsigned long lastProgramUpdate[3];
bool flashbit[3] = {true, true, true};
short programLightsPosition[3] = {-1,-1,-1};
bool gridStat[4][4];
bool shiftRegisterOut[16];
unsigned long jackWasPlugged[15]; //when jack was plugged
unsigned long lastShiftRegisterUpdate;
unsigned long lastLightUpdate;
bool jackIsPlugged[15]; //is jack plugged
short jackConnectedTo[15] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int x;
int y;
int i;
Adafruit_NeoPixel rgbLights(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);
byte PixelStates[60][3];
unsigned long lastPixelUpdate;
Encoder Rotary1(ENC1_CLK,ENC1_DIO);
Encoder Rotary2(ENC2_CLK,ENC2_DIO);
Encoder Rotary3(ENC3_CLK,ENC3_DIO);
Encoder* RotaryEncoders[3] = {&Rotary1,&Rotary2,&Rotary3};
long pos[3];
long oldPos[3];
int count[3];
byte selected[3];
const byte EncoderButtons[3] = {5,6,7};
byte debounceEncoder[3];
byte litUp[3][7];
unsigned long lastBlinkTime;
byte blinkoff;
