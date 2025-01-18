#include "arduino.h"
#include <Tone.h>
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>
#define ENC_CLK1 18
#define ENC_DIO1 19
#define ENC_CLK2 20
#define ENC_DIO2 21
#define lightUpdate 10

#define CIRCLE_COUNT 12
#define CIRCLE1_PIN 10
#define CIRCLE2_PIN 11
#define BAR_COUNT 22
#define BAR1_PIN 8
#define BAR2_PIN 9
#define BASE 440

#define MIXCOLORSWITCH A2

const byte speakerPins[2] = {12,13}; 
const byte colorSliderPins[2] = {A0,A1};
const byte colorLockButtonPins[2][3] = {{7,6,5},{4,3,2}};
const int colorLockButtonLights[2][3] = {{A8,A9,A10},{A11,A12,A13}};
byte circleColors[2][3][3];
byte circleLockedNums[2][3];
const byte SwitchPins[12][2] = {{23,39},{25,41},{27,43},{29,45},{31,47},{33,49},{35,14},{37,15},{22,38},{24,40},{26,42},{28,44}};
const int buttonLightPins[4][2] = {{30,17},{32,16},{34,46},{36,48}}; 
int currColor[2][12][3];

unsigned long timeCurr;
unsigned long lastColorWheelUpdate;
unsigned long lastBarCheck;
unsigned long lastBarUpdate[2];
long oldPos[2];
long pos[2];
short TruePos[2];
int checkEvry[2] = {100,100};
int x;
int y;
byte BarOffset[2];
bool circlesMoving = false;
byte circleOffset;
unsigned long LastRotate;
int rotateDelay = 100;
unsigned long lastPixUpdate = 0;
byte notekeeper[2];
unsigned long lastSoundStarted[2];
short nextSoundDueToPlay[2];

Tone speakers[2];
Adafruit_NeoPixel circle1(CIRCLE_COUNT, CIRCLE1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel circle2(CIRCLE_COUNT, CIRCLE2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bar1(BAR_COUNT, BAR1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bar2(BAR_COUNT, BAR2_PIN, NEO_GRB + NEO_KHZ800);

Encoder Rotary1(ENC_CLK1,ENC_DIO1);
Encoder Rotary2(ENC_CLK2,ENC_DIO2);
