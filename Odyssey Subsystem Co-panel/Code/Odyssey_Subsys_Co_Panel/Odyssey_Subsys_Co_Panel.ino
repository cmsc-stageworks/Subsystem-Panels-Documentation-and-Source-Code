#include <Adafruit_NeoPixel.h>

#define BIG_RED_BUTTON_PIN A2
#define BIG_RED_BUTTON_LIGHT A3
#define BAR_PIN1 11
#define BAR_PIN2 12
#define PYRMID_PIN1 13
#define PYRMID_PIN2 6
#define BAR_COUNT 62
#define PYRMID_COUNT 6
#define DIAG_PIN 5
#define DIAG_COUNT 4
#define RGBBUTTON_PIN 7
#define RGBBUTTON_COUNT 3
#define dataPin 2
#define latchPin 3
#define clockPin 4
#define PYRMID_DELAY 150

byte PyrmidAnimationPath[3][6] = {{5,2,0,4,1,3},{5,4,3,2,1,0},{5,3,1,0,4,2}};

byte bigRedStat = 0;
byte bigRedDebounce;
byte PyrmidLightColors[2][6][3];
unsigned long LastPyrmidUpdate;
const byte PyrmidSwitches[2][3] = {{27,25,23},{26,24,22}};
byte PyrmidSwitchStates[2][3];
byte PyrmidAnimationPos[2][3];
byte MaxCapacity[2] = {100,100};
int invertedAnimationPos[2][2];
byte invertedDirection[2][2];
const byte middleSliderPins[] = {A0,A1};
byte sliderPercent[2];
byte sliderPowerPins[2] = {31,30};
byte sliderPolarityReverse[2] = {29,28};
byte sideColors[2][3] = {{0,255,255},{255,255,0}};
byte LightGrid[20];
byte debounce[20];
const byte middleButtonPins[20] = {14,15,16,17, 47, 45, 43, 41, 46, 44, 42, 40, 39, 37, 35, 33, 38, 36, 34, 32};
unsigned long diagCounter = 0;
byte diagnosticRunning = 0;
unsigned long flashClock = 0;
unsigned long timeSinceDiagnosticStart = 0;
byte currDiagRunning = 0;
byte lightAlt = 0;
short flashDelay = 500;
bool completedDiag = false;
byte diagLights[4] = {0,0,0,0};
byte runThroughDiag = 0;
byte middleColor[3] = {0,0,0};
const byte middleRGBButtonPins[3] = {8,9,10};
unsigned int lastRGBUpdate = 0;
byte debounceMiddleRGB[3] = {0,0,0};
unsigned long lastSliderUpdate;

Adafruit_NeoPixel barGraph1(BAR_COUNT, BAR_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel barGraph2(BAR_COUNT, BAR_PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pyrmidLight1(PYRMID_COUNT, PYRMID_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pyrmidLight2(PYRMID_COUNT, PYRMID_PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel diagnosticLights(DIAG_COUNT, DIAG_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel middleRGB(RGBBUTTON_COUNT, RGBBUTTON_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel* bars[2] = {&barGraph1,&barGraph2};
Adafruit_NeoPixel* pyrmidLights[2] = {&pyrmidLight1,&pyrmidLight2};

void setup() {
  // put your setup code here, to run once:
  pinMode(BIG_RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BIG_RED_BUTTON_LIGHT, OUTPUT);
  for(int i = 0; i <  2; i++){
    (*pyrmidLights[i]).begin();
    (*pyrmidLights[i]).clear();
    (*pyrmidLights[i]).show();
    (*pyrmidLights[i]).setBrightness(10);
    (*bars[i]).begin();
    (*bars[i]).clear();
    (*bars[i]).show();
    (*bars[i]).setBrightness(10);
    pinMode(middleSliderPins[i], INPUT);
    pinMode(sliderPowerPins[i], INPUT_PULLUP);
    pinMode(sliderPolarityReverse[i], INPUT_PULLUP);
    for(int x = 0; x < 3; x++){
      pinMode(PyrmidSwitches[i][x], INPUT_PULLUP);
    }
  }
  diagnosticLights.begin();
  diagnosticLights.clear();
  diagnosticLights.show();
  diagnosticLights.setBrightness(100);
  middleRGB.begin();
  middleRGB.clear();
  middleRGB.show();
  middleRGB.setBrightness(100);
  for(int i = 0; i < 20; i++){
    pinMode(middleButtonPins[i], INPUT_PULLUP);
  }
  for(int i = 0; i < 3; i++){
    pinMode(middleRGBButtonPins[i], INPUT_PULLUP);
  }
  pinMode(dataPin,OUTPUT);
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  updateLights();
  Serial.begin(9600);
}

void loop() {
  checkButtons();
  Diagnostics();
  RGBMiddle();
  Sliders();
  RunPyrmids();
  updateLights();
}

void checkButtons(){
  for(int i = 4; i < 20; i++){
    if(!debounce[i] && !digitalRead(middleButtonPins[i])){
      LightGrid[i] = !LightGrid[i];
    }
    debounce[i] = !digitalRead(middleButtonPins[i]);
  }
  byte test = !digitalRead(BIG_RED_BUTTON_PIN);
  if(test && !bigRedDebounce)
    bigRedStat = !bigRedStat;
  bigRedDebounce = test;
}

/*
 * byte PyrmidAnimationPos[2][3];
byte PyrmidAnimationPath[3][6] = {{0,1,2,3,4,5},{5,4,3,2,1,0},{0,1,3,2,4,5}}
 */
void RunPyrmids(){
  if(millis()-LastPyrmidUpdate >= PYRMID_DELAY){
    LastPyrmidUpdate = millis();
    for(byte side = 0; side < 2; side++){
      for(byte x = 0; x < 6; x++){
        for(byte y = 0; y < 3; y++){
          PyrmidLightColors[side][x][y] = 0;
        }
      }
      for(byte x = 0; x < 3; x++){
        PyrmidSwitchStates[side][x] = !digitalRead(PyrmidSwitches[side][x]);
        PyrmidAnimationPos[side][x]++;
        if(PyrmidAnimationPos[side][x] >= 6){
          PyrmidAnimationPos[side][x] = 0;
        }
        if(PyrmidSwitchStates[side][x]){
          PyrmidLightColors[side][PyrmidAnimationPath[x][PyrmidAnimationPos[side][x]]][x] += 255;
        }
      }
      (*pyrmidLights[side]).clear();
      for(byte x = 0; x < 6; x++){
        (*pyrmidLights[side]).setPixelColor(x, (*pyrmidLights[side]).Color(PyrmidLightColors[side][x][0],PyrmidLightColors[side][x][1],PyrmidLightColors[side][x][2]));
      }
      (*pyrmidLights[side]).show();
    }
  }
}


void Sliders(){
  if(millis()-lastSliderUpdate > 50){
    lastSliderUpdate = millis();
    for(byte side = 0; side < 2; side++){
      if(!digitalRead(sliderPowerPins[side])){
        sliderPercent[side] = round((analogRead(middleSliderPins[side])-10)/1000.0*100.0);
        if(sliderPercent[side]>150){
          sliderPercent[side] = 0;
        }
        else if(sliderPercent[side]>150){
          sliderPercent[side] = 100;
        }
        byte inverted = !digitalRead(sliderPolarityReverse[side]);
        byte SliderValue;
        byte CapacityValue;
        if(MaxCapacity[side] - sliderPercent[side] >=0){
          SliderValue = round(sliderPercent[side]/100.0*BAR_COUNT/2);
        }
        else{
          SliderValue = round(MaxCapacity[side]/100.0*BAR_COUNT/2);
        }
        CapacityValue = SliderValue;
        (*bars[side]).clear();
        for(int i = 0; i <  BAR_COUNT/2; i++){
          if(SliderValue >= i+1){
            (*bars[side]).setPixelColor(-i+(BAR_COUNT/2-1), barGraph1.Color(sideColors[inverted][0],sideColors[inverted][1],sideColors[inverted][2]));
          }
          if(CapacityValue >= i+1){
            (*bars[side]).setPixelColor(-i+BAR_COUNT-1, barGraph1.Color(sideColors[inverted][0],sideColors[inverted][1],sideColors[inverted][2]));
          }
        }
        if(inverted){
          (*bars[side]).setPixelColor(-invertedAnimationPos[side][0]+BAR_COUNT/2-1, barGraph1.Color(sideColors[0][0],sideColors[0][1],sideColors[0][2]));
          (*bars[side]).setPixelColor(-invertedAnimationPos[side][1]+BAR_COUNT-1, barGraph1.Color(sideColors[0][0],sideColors[0][1],sideColors[0][2]));
          for(int i = 0; i < 2; i++){
            if(invertedDirection[side][i]){
              invertedAnimationPos[side][i]--;
            }
            else{
              invertedAnimationPos[side][i]++;
            }
            if(invertedAnimationPos[side][i] < 0){
              invertedAnimationPos[side][i] = 0;
              invertedDirection[side][i] = 0;
            }
          }
          if(invertedAnimationPos[side][0] >= SliderValue){
            invertedAnimationPos[side][0] = SliderValue-1;
            invertedDirection[side][0] = 1;
          }
          if(invertedAnimationPos[side][1] >= CapacityValue){
            invertedAnimationPos[side][1] = CapacityValue-1;
            invertedDirection[side][1] = 1;
          }
        }
        if(SliderValue <= 0){
          (*bars[side]).clear();
        }
        (*bars[side]).show();
      }
      else{
        (*bars[side]).clear();
        (*bars[side]).show();
      }
    }
  }
}

void RGBMiddle(){
    if(bigRedStat){
      digitalWrite(BIG_RED_BUTTON_LIGHT, HIGH);
    }
    else{
      digitalWrite(BIG_RED_BUTTON_LIGHT, LOW);
    }
    for(int i = 0; i < 3; i++){
      byte temp = !digitalRead(middleRGBButtonPins[i]);
      if(temp && !debounceMiddleRGB[i]){
        middleColor[i] = !middleColor[i];
      }
      debounceMiddleRGB[i] = temp;
    }
    if(millis() - lastRGBUpdate >= 50){
      for(int i = 0; i < 3; i++){
        if(middleColor[i]){
          middleRGB.setPixelColor(i, middleRGB.Color(middleColor[0]*255, middleColor[1]*255, middleColor[2]*255));  
        }
        else{
          middleRGB.setPixelColor(i, middleRGB.Color(0,0,0));
        }
      }
      middleRGB.show();
      lastRGBUpdate = millis();
    }
}







void RunDiag(byte _diagSys){
    if(currDiagRunning == 0){
      currDiagRunning = _diagSys+1;
      timeSinceDiagnosticStart = millis();
      flashClock = millis();
      lightAlt = 1;
      diagnosticLights.setPixelColor(_diagSys , diagnosticLights.Color(255*lightAlt, 255*lightAlt, 0));
      diagnosticLights.show();
      flashDelay = 500;
    }
    else{
      if(millis()-flashClock > flashDelay){
        lightAlt = !lightAlt;
        diagnosticLights.setPixelColor(_diagSys , diagnosticLights.Color(255*lightAlt, 255*lightAlt, 0));
        diagnosticLights.show();
        flashClock = millis();
        flashDelay -= 50;
        if(flashDelay < 100){
          flashDelay = 100;
        }
      }
      if(millis()-timeSinceDiagnosticStart > 5000){
        randomSeed(millis());
        byte verdict = random(3);
        if(verdict == 0){
          diagnosticLights.setPixelColor(_diagSys , diagnosticLights.Color(255,0, 0));
          diagLights[_diagSys] = 1;
        }
        else if(verdict == 1){
          diagnosticLights.setPixelColor(_diagSys , diagnosticLights.Color(0, 255, 0));
          diagLights[_diagSys] = 2;
        }
        else{
          diagnosticLights.setPixelColor(_diagSys , diagnosticLights.Color(0,0,255));
          diagLights[_diagSys] = 3;
        }
        diagnosticLights.show();
        completedDiag = true;
        currDiagRunning = 0;
      }
    }
  }



void Diagnostics(){
  if(diagnosticRunning == 0){
    if(!digitalRead(middleButtonPins[0])){
      diagnosticLights.clear();
          diagnosticLights.show();
        LightGrid[0] = 1;
        if(diagCounter == 0){
          diagCounter = millis();
        }
        else if(millis()-diagCounter >= 5000){
          diagnosticRunning = 1;
        }
    }
    else if(!digitalRead(middleButtonPins[1])){
      diagnosticLights.clear();
          diagnosticLights.show();
        LightGrid[1] = 1;
        if(diagCounter == 0){
          diagCounter = millis();
        }
        else if(millis()-diagCounter >= 5000){
          diagnosticRunning = 2;
        }
    }
    else if(!digitalRead(middleButtonPins[2])){
      diagnosticLights.clear();
          diagnosticLights.show();
        LightGrid[2] = 1;
        if(diagCounter == 0){
          diagCounter = millis();
        }
        else if(millis()-diagCounter >= 5000){
          diagnosticRunning = 3;
        }
    }
    else if(!digitalRead(middleButtonPins[3])){
      diagnosticLights.clear();
          diagnosticLights.show();
        LightGrid[3] = 1;
        if(diagCounter == 0){
          diagCounter = millis();
        }
        else if(millis()-diagCounter >= 5000){
          diagnosticRunning = 4;
        }
    }
    else{
      LightGrid[0] = 0;
      LightGrid[1] = 0;
      LightGrid[2] = 0;
      LightGrid[3] = 0;
      diagCounter = 0;
    }
  }
  else{
    diagCounter = 0;
    if(diagnosticRunning == 1){
      RunDiag(0);
      if(completedDiag){
        diagnosticRunning = 0;
        completedDiag = false;
      }
    }
    else{
      RunDiag(runThroughDiag);
      if(completedDiag){
        completedDiag = false;
        runThroughDiag++;
        if(runThroughDiag == diagnosticRunning){
          diagnosticRunning = 0;
          runThroughDiag = 0;
        }
      }
    }
  }
}








void updateLights(){
  byte tempOuts[3] = {0,0,0};
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, tempOuts[2]);
  shiftOut(dataPin, clockPin, MSBFIRST, tempOuts[1]);
  shiftOut(dataPin, clockPin, MSBFIRST, tempOuts[0]);
  digitalWrite(latchPin, HIGH);
  for(int i; i < 24; i++){
    if(LightGrid[i]){
      if(i < 8){
        bitSet(tempOuts[0], i);
      }
      else if(i < 16){
        bitSet(tempOuts[1], i-8);
      }
      else{
        bitSet(tempOuts[2], i-16);
      }
    }
  }
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, tempOuts[2]);
  shiftOut(dataPin, clockPin, MSBFIRST, tempOuts[1]);
  shiftOut(dataPin, clockPin, MSBFIRST, tempOuts[0]);
  digitalWrite(latchPin, HIGH);
}
