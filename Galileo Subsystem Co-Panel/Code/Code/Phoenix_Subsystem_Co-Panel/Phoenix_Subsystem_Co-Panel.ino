/*
Nathan Ford 3-3-21
Phoenix Subsystem Co-Panel
Designed for Christa McCauliffe Space Center

*/
#include "PhoenixGlobalVars.h"
#include "settings.h"
#define VERSIONNUMBER "Pheonix Subsystem Co-Panel Version 1.2 (Release)"



/*
-----------------------------------------------------------------------------------------------------------------------------------------------
|   modifiable variables are in settings.h                                                                                            |
-----------------------------------------------------------------------------------------------------------------------------------------------
*/



void startPixel(Adafruit_NeoPixel &_P){ // function to start neopixel strands
  _P.begin();
  _P.show();
  _P.setBrightness(10);
}
void setPinModes(){ // sets if each pin is input, output, or input with a pullup resistor
  pinMode(MIXCOLORSWITCH, INPUT_PULLUP);
  pinMode(colorSliderPins[0], INPUT);
  pinMode(colorSliderPins[1], INPUT);
  for(x = 0; x < 12; x++){
    pinMode(SwitchPins[x][0], INPUT_PULLUP);
    pinMode(SwitchPins[x][1], INPUT_PULLUP);
  }
  for(x = 0; x < 3; x++){
    pinMode(colorLockButtonPins[0][x], INPUT_PULLUP);
    pinMode(colorLockButtonPins[1][x], INPUT_PULLUP);
    pinMode(colorLockButtonLights[0][x], OUTPUT);
    digitalWrite(colorLockButtonLights[0][x], LOW);
    pinMode(colorLockButtonLights[1][x], OUTPUT);
    digitalWrite(colorLockButtonLights[1][x], LOW);
  }
  for(x = 0; x < 4; x++){
    for(y = 0; y< 2; y++){
      pinMode(buttonLightPins[x][y], OUTPUT);
      digitalWrite(buttonLightPins[x][y], LOW);
    }
  }
}

void setup() {
  startPixel(circle1);
  startPixel(circle2);
  startPixel(bar1);
  startPixel(bar2);
  setPinModes();
  CheckForBarUpdates();
  Serial.begin(9600);
  Serial.println(VERSIONNUMBER);
  for(int i = 0; i < 2; i++){
    speakers[i].begin(speakerPins[i]);
  }
}

void loop() {
  timeCurr = millis();
  CheckForBarUpdates();
  checkToUpdateColorWheels();
  playNotes();
}
/*
unsigned long lastSoundStarted[2];
short nextSoundDueToPlay[2];
const short sheetmusic[2][sheetMusicLength]
const byte SwitchPerButton[2][sheetMusicLength]
const float NoteDuration[2][sheetMusicLength]
*/
void playNotes(){ // funciton for playing the next notes
  for(byte side = 0; side < 2; side++){ // both sides need it
    if(millis() - lastSoundStarted[side] >= nextSoundDueToPlay[side]){ // has it been long enough yet?
      lastSoundStarted[side] = millis(); // record this time
      nextSoundDueToPlay[side] = NoteDuration[side][notekeeper[side]] * round(60.0/BeatsPerMinute * 1000); // calculate how long this note will play for
      notekeeper[side]++;// increment counter and keep in range
      if(notekeeper[side] >= sheetMusicLength){
        notekeeper[side] = 0;
      }
      resumeSpeakers(side); // start the sounds
    }
  }
}
void resumeSpeakers(byte side){ // play notes
  int noteNum = sheetmusic[side][notekeeper[side]]; // get note number
    float toneFreq = BASE; // set base freqency
    if(noteNum >=0){ // if it is above an A
      for(int i = 0; i < noteNum; i++){ // calculate the freqency of the note
        toneFreq = toneFreq * pow(2,1.0/12.0);
      }
    }
    else{ // calculate frequency backwards
      for(int i = 0; i < noteNum; i--){
        toneFreq = toneFreq / pow(2,1.0/12.0);
      }
    }
    if(digitalRead(SwitchPins[SwitchPerButton[side][notekeeper[side]]][side])){ // if that note is enabled
      speakers[side].play(toneFreq); // play it
    }
}

void checkToUpdateColorWheels(){
  if(millis() - lastColorWheelUpdate > 150){
    lastColorWheelUpdate = millis();
    //for(int i = 0; i < 2; i++){
      //speakers[i].stop();
    //}
    if(!digitalRead(MIXCOLORSWITCH)){ // if circles are enabled
      if(circlesMoving == false){
        rotateDelay = 750;
      }
      circlesMoving = true;
    }
    else{
      circlesMoving = false; // if not stop them
    }
    circle1.clear();
    circle2.clear();
    if(!circlesMoving){ // update colors if not in motion
      updateCircles(0);
      updateCircles(1);
    }
    else{
      if(timeCurr - LastRotate >= rotateDelay){ // if they are moving, move them in a circle
        rotateCircles();
        LastRotate = timeCurr;
        if(rotateDelay > 100){
          rotateDelay -= 50;
        }
      }
    }
  }
}
void rotateCircles(){
  circleOffset++; // begin the spin
  if(circleOffset > 11){ // if made full loop, set back to 0
    circleOffset = 0;
  }
  for(int side = 0; side < 2; side++){ // mirror both sides
    for(x = 0; x < 3; x++){ // get those 3 colors used in each ring
      if(circleLockedNums[side][x]){ // if a color is not locked in
        for(y = 0; y < 4; y++){ // set the 4 pixels on each ring to blank
          if(side == 0){ // if first side, yes i know i should have used pointers, but, since i am putting these comments in afterwords, i dont really feel like changing it XD
            circle1.setPixelColor(((x + circleOffset)%3 + y*3)%12,circle1.Color(0,0,0));
          }
          else{
            circle2.setPixelColor(((x + circleOffset)%3 + y*3)%12,circle1.Color(0,0,0));
          }
        }
      }
      else{
        for(y = 0; y < 4; y++){ // since a color is locked in, set the 4 pixels on each ring to it
          if(side == 0){
            circle1.setPixelColor(((x + circleOffset)%3 + y*3)%12,circle1.Color(circleColors[side][x][0],circleColors[side][x][1],circleColors[side][x][2]));
          }
          else{
            circle2.setPixelColor(((x + circleOffset)%3 + y*3)%12,circle2.Color(circleColors[side][x][0],circleColors[side][x][1],circleColors[side][x][2]));
          }
        }
      }
    }
  }
  circle1.show();
  circle2.show();
}

void updateCircles(byte side){
  int sliderValue = analogRead(colorSliderPins[side]); // get that slider value
  sliderValue = round(sliderValue/1024.0*765.0); // round it to a useable value
  byte sliderColor[3] = {getRGB(0,sliderValue),getRGB(1,sliderValue),getRGB(2,sliderValue)}; // get those colors based on our color wheel
  for(x = 0; x < 3; x++){
    circleLockedNums[side][x] = digitalRead(colorLockButtonPins[side][x]); // check if we need to lock a value in
    if(circleLockedNums[side][x]){ // if it isn't locked
      circleColors[side][x][0] = sliderColor[0]; //  set colors to what slider converts to
      circleColors[side][x][1] = sliderColor[1];
      circleColors[side][x][2] = sliderColor[2];
      if(side == 0){ // blank those pixels
        for(y = 0; y < 4; y++)
          circle1.setPixelColor(x + y*3,circle1.Color(0,0,0));
      }
      else{
        for(y = 0; y < 4; y++)
          circle2.setPixelColor(x + y*3,circle2.Color(0,0,0));
      }
    }
    else {
      if(side == 0){ // if it is locked set to preset color
        for(y = 0; y < 4; y++)
          circle1.setPixelColor(x + y*3,circle1.Color(circleColors[side][x][0],circleColors[side][x][1],circleColors[side][x][2]));
      }
      else{
        for(y = 0; y < 4; y++)
          circle2.setPixelColor(x + y*3,circle2.Color(circleColors[side][x][0],circleColors[side][x][1],circleColors[side][x][2]));
      }
    }
  }
  if(side == 0){
    circle1.show();
  }
  else{
    circle2.show();
  }
}

byte getRGB(byte _sel, int _slider){ // liniar color wheel converter
  if(_sel == 0){ //red calculations
    if(0 <= _slider and _slider <= 255){
      return 255-_slider;
    }
    else if(255 < _slider and _slider < 510){
      return 0;
    }
    else{
      return _slider-510;
    }
  }
  else if(_sel == 1){ //green calculations
    if(0 <= _slider and _slider <= 255){
      return(_slider);
    }
    else if(255 <= _slider and _slider <= 510){
      return(510-_slider);
    }
    else{
      return 0;
    }
  }
  else{//blue calculations
    if(255 <= _slider and _slider <= 510){
      return(_slider - 255);
    }
    else if(510 <= _slider and _slider <= 765){
      return(765-_slider);
    }
    else{
      return 0;
    }
  }
}

void CheckForBarUpdates(){
  if(millis() - lastBarCheck > 150){
    lastBarCheck = millis();
    //for(int i = 0; i < 2; i++){
      //speakers[i].stop();
    //}
    pos[0] = Rotary1.read(); // read encoders
    pos[1] = Rotary2.read();
    for(int i = 0; i < 2; i++){
      if(pos[i] - oldPos[i] >= 4){
        TruePos[i]++;
        oldPos[i] += 4;
      }
      if(oldPos[i] - pos[i] >= 4){
        TruePos[i]--;
        oldPos[i] -= 4;
      }
      checkEvry[i] = checkEvry[i] + 5*(TruePos[i]); // calc new delay values based on rotary encoders
      TruePos[i] = 0;
    }
    if(checkEvry[0] < 10){ //make sure delays are in bounds
      checkEvry[0] = 10;
    }
    else if(checkEvry[0] > 1000){
      checkEvry[0] = 1000;
    }
    if(checkEvry[1] < 10){
      checkEvry[1] = 10;
    }
    else if(checkEvry[1] > 1000){
      checkEvry[1] = 1000;
    }
    if(millis() - lastBarUpdate[0]> checkEvry[0]){ // delay and progress animation cycle
      BarOffset[0]++;
      if(BarOffset[0] > 21){
        BarOffset[0] = 0;
      }
      lastBarUpdate[0] = timeCurr;
    }
    if(millis() - lastBarUpdate[1] > checkEvry[1]){ // delay and progress animation cycle
      BarOffset[1]++;
      if(BarOffset[1] > 21){
        BarOffset[1] = 0;
      }
      lastBarUpdate[1] = timeCurr;
    }
    if(millis() - lastPixUpdate > lightUpdate){ // if we need to update lights
      lastPixUpdate = millis();
      UpdateBars(0);
      UpdateBars(1);
    }
  }
}
void UpdateBars(byte side){
    for(x = 0; x < 12; x++){
      for(y = 0; y < 3; y++)
        currColor[side][x][y] = 0; // blank it
    }
    for(x = 0; x < 12; x++){
      if(!digitalRead(SwitchPins[x][side])){ //  if buttons are pressed
        if(x > 7){
          digitalWrite(buttonLightPins[x-8][side], HIGH); // (try to) turn on their lights
        }
        for(y = 0; y < 4; y++){
          if(LightsEffected[side][x][y] >= 0){
            for(int i = 0; i < 3; i++)
              currColor[side][LightsEffected[side][x][y]][i] += colorAdd[side][x][i];
          }
        }
      }
      else if(x > 7){
        digitalWrite(buttonLightPins[x-8][side], LOW); // if not pushed, turn it off
      }
    }
    for(x = 0; x < 12; x++){
      if(side == 0){ // set bar pixels
        bar1.setPixelColor((x + BarOffset[side])%22, bar1.Color(currColor[side][x][0]%255,currColor[side][x][1]%255,currColor[side][x][2]%255));
        bar1.setPixelColor((x + 12 + BarOffset[side])%22, bar1.Color(currColor[side][x][0]%255,currColor[side][x][1]%255,currColor[side][x][2]%255));
      }
      else{
        bar2.setPixelColor((x + BarOffset[side])%22, bar1.Color(currColor[side][x][0]%255,currColor[side][x][1]%255,currColor[side][x][2]%255));
        bar2.setPixelColor((x + 12 + BarOffset[side])%22, bar1.Color(currColor[side][x][0]%255,currColor[side][x][1]%255,currColor[side][x][2]%255));
      }
    }
    if(side == 0)
      bar1.show();
    else
      bar2.show();
}
