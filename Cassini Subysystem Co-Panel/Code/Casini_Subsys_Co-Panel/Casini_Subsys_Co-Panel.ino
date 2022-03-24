 /*
 Nathan Ford 
 2021
 CMSC Casini Gauge Co-panel
 */

#include <Adafruit_NeoPixel.h>

#define SECTIONS 6
#define BACKLIGHTPIN 13
#define VERSIONNUMBER "Casini Subsytem Co-Panel Version 1.2 (Release)"
const byte switchPins[SECTIONS][5] = {{22,23,24,25,26},{27,28,29,30,31},{32,33,34,35,36},{37,38,39,40,41},{42,43,44,45,46},{A8,A9,A10,A11,A12}};
const byte multiplyPins[SECTIONS][2] = {{8,9},{10,11},{14,15},{16,17},{18,19},{20,21}};
const byte scalePins[SECTIONS] = {2,3,4,5,6,7};
const int  valueChange[6][5] = {{28,-20,-5,32,-11}, {35,8,-20,30,17}, {60,-15,20,-15,30}, {-3,5,10,15,20}, {49,36,25,16,9}, {-15,-20,-32,-10,90},};
const float  multiplyValue[6][2] = {{0,0},{1.5,.5},{0,0},{0,0},{0,0},{-.5,1.25}};
const byte LightIndex[6]  = {0,3,1,4,2,5};
int x;
int y;
int i;
byte section;
unsigned long lastPixelUpdate = 0;

Adafruit_NeoPixel backlights(SECTIONS, BACKLIGHTPIN, NEO_GRB + NEO_KHZ800);
void setup() {
  backlights.begin();// initiallise backlights
  backlights.clear();
  backlights.setBrightness(128);
  backlights.show();
  for(x = 0; x < SECTIONS; x++){ // set all those switches to inputs
    for(y = 0; y < 5; y++){
      pinMode(switchPins[x][y], INPUT_PULLUP);
    }
    for(y = 0; y < 2; y++){
      pinMode(multiplyPins[x][y], INPUT_PULLUP);
    }
  }
  Serial.begin(9600); // begin serial
  Serial.println(VERSIONNUMBER);
}

void loop() {
  if(millis() - lastPixelUpdate > 10){ // if we havent updated for 10 ms, update! if we have, then just chill till we do (we will update 100 times per second)
    lastPixelUpdate = millis();
    updateAll();
  }
}

void updateAll(){
  for(section = 0; section < SECTIONS; section++){ // for each gauge do the following:
    short scaleValue = 0; // start with a blank slate
    for(i = 0; i < 5; i++){ // read switches and incrament scale value accordingly
      if(!digitalRead(switchPins[section][i])){
        scaleValue += valueChange[section][i];
      }
    }
    calcNewVal(section,scaleValue); // do fancy math and tricks according to each gauges specified way
    if(scaleValue < 0){ // we cant display negatives on our scales
      scaleValue = 0;
    }
    else if(scaleValue > 100){// nor can we go larger than 100
      scaleValue = 100;
    }
    short tempVal = 7.65 * scaleValue; // convert our scale of 0 - 100, to a one between 0- 765 (for backlight color calculations)
    backlights.setPixelColor(LightIndex[section],backlights.Color(getRGB(0, tempVal),getRGB(1, tempVal),getRGB(2, tempVal))); // set backlight
    scaleValue = round((scaleValue/100.0)*255);// convert 0-100 to 0-255 for analog output for the gauges so we get 0-5v output
    analogWrite(scalePins[section], scaleValue); // write to gauge
  }
  backlights.show(); // after all gauges have been updated, dont forget to push all the new backlight colors!
}

void calcNewVal(byte _section, short &_SV){ // magical calulations for the special middle switches based on sections
  if(_section == 1 || _section == 5){ // gauges 1 and 5 are ones that just do simple multiplications to the value
    for(i = 0; i < 2; i++){
      if(!digitalRead(multiplyPins[_section][i])){
        _SV = _SV * multiplyValue[_section][i];
      }
    }
  }
  else if(_section == 2){ // one simple multiply, but then second switch we do some -5 units per each switch up
    if(!digitalRead(multiplyPins[_section][0])){
      _SV = _SV * .75;
    }
    if(!digitalRead(multiplyPins[_section][1])){
      byte switchesUp = 0;
      for(i = 0; i < 5; i++){
        if(!digitalRead(switchPins[_section][0])){
          switchesUp++;
        }
      }
      _SV += -(switchesUp * 5);
    }
  }
  else if(_section == 4){ //  this is the fancy square root switch one
    byte switchesToGo = 0;
    for(i = 0; i < 2; i++){
      if(!digitalRead(multiplyPins[_section][i])){
        switchesToGo++;
      }
    }
    if(switchesToGo > 0){
      int switchFocus = 0;
      while(switchesToGo > 0 && switchFocus < 5){
        if(!digitalRead(switchPins[_section][switchFocus])){
          _SV += -valueChange[_section][switchFocus];
          _SV += pow(valueChange[_section][switchFocus],.5);
          switchesToGo += -1;
        }
        switchFocus++;
      }
    }
  }
  else if(_section == 0){ // doubles certain switches based on the two middle ones
    if(!digitalRead(multiplyPins[_section][0])){
      if(!digitalRead(switchPins[_section][0])){
        _SV += valueChange[_section][0];
      }
      if(!digitalRead(switchPins[_section][2])){
        _SV += valueChange[_section][2];
      }
      if(!digitalRead(switchPins[_section][3])){
        _SV += valueChange[_section][3];
      }
    }
    if(!digitalRead(multiplyPins[_section][1])){
      if(!digitalRead(switchPins[_section][1])){
        _SV += valueChange[_section][1];
      }
      if(!digitalRead(switchPins[_section][4])){
        _SV += valueChange[_section][4];
      }
    }
  }
  else if(_section == 3){ // this gauge just adds or subtracts big values based on special switches
    if(!digitalRead(multiplyPins[_section][0])){
      _SV +=50;
    }
    if(!digitalRead(multiplyPins[_section][1])){
      _SV += -65;
    }
    
  }
}



byte getRGB(byte _sel, int _slider){ // rgb liniar color "wheel" calculator
  if(_sel == 0){ //red value calculations
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
  else if(_sel == 1){ //green value calculations
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
  else{//blue value calculations
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
