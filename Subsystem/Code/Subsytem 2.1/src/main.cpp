#include <Arduino.h>
/*
CMSC SUBSYSTEM REPAIR PANEL
DESIGNED AND PROGRAMMED BY NATHAN FORD 2020 - UPDATED 2021
*/
// -- set this unique for each panel, or bad vodoo happens --
const String PanelName("PanelTest");
//--------------------------------------------------------------

#define DEBUGPRINTS false  //If system should send Debug Reports
#define MAKEPARSEFRIENDLY false //for the web page, should it look nice, or make it easy for thorium to use
#include "SevenSegmentTM1637.h"
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_ImageReader.h>
#include <SPI.h>

Encoder myEnc(20, 21);
#define PIXEL_PIN 8
#define NUM_PIXELS 19
#define speakerPin 2
#define incorrectBuzzFreq 100
#define VersionNumber "Subsystem Version 2.0 (Alpha) (Prototype)" //ETHERNET SERVER WOOT WOOT
byte mac[6] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
  };
const byte SectionLights[] = {16,17,18};
const byte PIN_CLK = 4;
const byte PIN_DIO = 5;
const byte Section1Button = 9;
const byte Section2Button = 10;
const byte Section3Button = 11;
const byte GridPins[4][4] = {{25,24,23,22},{26,27,28,29}};
const byte BinaryPins[8] = {30,31,32,33,34,35,36,37};
const byte JackPins[10] = {38,39,40,41,42,43,44,45,46,47};
const byte SDCardSSPin = 48;
const byte TFTScreenSSPin = 53;
const String Space = " ";
const byte ColorNums[] = {7,3,4,5,6,2,3,4,3,5,3,2};
const byte ProfileConversion[][7] = {
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
const byte Colors[7][3] = {
  {0,0,0},
  {255,0,0},
  {255,255,0},
  {0,255,0},
  {0,255,255},
  {179,120,211},
  {255,255,255}
};
const byte buttonNums[4][4] = { 
  {13,9,5,1},
  {14,10,6,2},
  {15,11,7,3},
  {16,12,8,4}
};


ImageReturnCode stat;
byte  buttonValues[4][4] = {
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};
byte prev[4][4] = {
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};
int jackConnections[5][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
String lines;
String inString;
int num;
int rotary;
int connections[5][2];
int colorProfile;
int startColors[16];
int endColors[16];
String result;
String filePrefix;
String ext = ".txt";
String picext = ".bmp";
String A = "a";
String B = "b";
String picPath = "/Pictures/";
byte tempScan;
byte x;
byte y;
byte i;
byte count;
long counter;
byte switches[8] = {0,0,0,0,0,0,0,0};
long oldPos = 0;
long pos = 0;
long newPos = 0;
long oldnewPos = 0;
short dialerValue = 0;
 byte section = 0;
byte lookingAt;
bool opened = false;
bool debugMode = false;
bool prevdebug = false;
bool lastSubmitCorrectOrIncorrect;
unsigned long lastJackCheck;
unsigned long lastActivity;
short lastCompletedSubsystem = -1;


#define USE_SD_CARD
#define SD_CS   48 // SD card select pin
#define TFT_CS 53 // TFT select pin
#define TFT_DC  49 // TFT display/command pin
#define SD_SPEED 2 // how many Mhz the SD card interface will run on, if you get artifacting, reduce this.

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

Adafruit_ILI9341     tft    = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;


SevenSegmentTM1637    display(PIN_CLK, PIN_DIO);
Adafruit_NeoPixel pixels(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
File myFile;

void section1();
void section2();
void section3();
void checkJackStatus();
void checkButtonStatus();
void updateButtons();
void readFile();
void sendData();
void reset();
byte findIndex(const byte list[], byte valueWanted);
void checkForLastUpateRestart();
void communicateWithEthernetController();

void setPinModes(){
  pinMode(speakerPin, OUTPUT);
  pinMode(Section1Button, INPUT_PULLUP);
  pinMode(Section2Button, INPUT_PULLUP);
  pinMode(Section3Button, INPUT_PULLUP);
  for(i = 0; i < 4; i++){
    pinMode(GridPins[0][i], OUTPUT);
  }
  for(i = 0; i < 4; i++){
    pinMode(GridPins[1][i], INPUT_PULLUP);
  }
  for(i = 0; i < 10; i++){
    pinMode(JackPins[i], INPUT_PULLUP);
  }
  for(i = 0; i < 8; i++){
    pinMode(BinaryPins[i], INPUT_PULLUP);
  }
  
}
void setup(void) {
  // put your setup code here, to run once:
  setPinModes();
  Serial.begin(9600);
  Serial.println(VersionNumber);
  #if DEBUGPRINTS
  Serial.println("DEBUG PRINTS ENABLED"); //this line is only compiled if debug scripts is set to true
  #endif
  display.begin();
  display.setBacklight(100);
  display.print("INIT");
  pixels.begin();
  pixels.clear();
  pixels.setBrightness(200);
  pixels.show();
  while(!Serial);
  tft.begin();
  if(!SD.begin(SD_CS, SD_SCK_MHZ(SD_SPEED))) { //This is where the infamous NOSD error comes from, this only occurs if there is a problem initializing the sd card
    Serial.println(F("SD begin() failed"));
    display.print("NOSD");
    for(;;);
  }
  stat = reader.drawBMP("/Pictures/logo.bmp", tft, 0, 0); //draws ship logo stored on the sd card
  display.clear(); //display and tft can be easily mixed up, display is the 7 segment display and tft is the screen
  oldPos = myEnc.read();
  result = Space + Space + Space + newPos; //formats the number nice
  display.print(result);
  
}
void loop() {
  communicateWithEthernetController();
  if(!debugMode){
    if(section == 0){ //section selection
      section1();
    }
    else if(section == 1){
      if(millis() - lastJackCheck > 4900){
        lastJackCheck = millis();
      }
      section2();
    }
    else if(section == 2){
      section3();
    }
    else{
      Serial.println("Oh Poop, section variable got corrupted");
    }
  }
  else{ //this is the jack connection tester
    if(!digitalRead(JackPins[5])){
      if(!prevdebug){
        tone(speakerPin, 440);
        pixels.setPixelColor(SectionLights[1], pixels.Color(0, 255, 0));
        pixels.show();
        prevdebug = true;
      }
    }
    else{
      if(prevdebug){
        noTone(speakerPin);
        prevdebug = false;
      }
      pixels.setPixelColor(SectionLights[1], pixels.Color(255, 255, 0));
      pixels.show();
    }
    delay(10);
  }
  checkForLastUpateRestart(); // check for inactivity and if they have been gone for more than 10 min restart
}
void incorrect(byte section) { //this is the beeping and red light you get when you get a puzzle incorrect
  section = section-1;
  lastSubmitCorrectOrIncorrect = 0;
  for(i = 0; i < 3; i++){
    tone(speakerPin, incorrectBuzzFreq);
    pixels.setPixelColor(SectionLights[section], pixels.Color(255, 0, 0));
    pixels.show();
    delay(750);
    noTone(speakerPin);
    pixels.setPixelColor(SectionLights[section], pixels.Color(0, 0, 0));
    pixels.show();
    delay(750);
  }
}

void readSwitches(){
  for(i = 0; i < 8; i++){
    switches[i] = digitalRead(BinaryPins[i]); //check rocker status and store in memory
  }
  count = 0;
  for(i = 0; i < 8; i++){
    if(switches[i] == 0){
      count += round(pow(2,i)); // change the binary sequence into an actual number
    }
  }
}

void section1() {
  //check to debug mode (section 2 and 3 buttons pressed at the same time)
  if(!digitalRead(Section2Button) && !digitalRead(Section3Button)){
    Serial.println("Entering Debug Mode");
    debugMode = true; 
    pinMode(JackPins[0], OUTPUT);
    digitalWrite(JackPins[0], LOW);
    for(int lights3 = 0; lights3 < 3; lights3++){
      pixels.setPixelColor(SectionLights[lights3], pixels.Color(255, 255, 0)); //make buttons yellow so user knows they entered it
    }
    pixels.show();
    return;
  }
  tempScan = digitalRead(Section1Button);
  if(tempScan == 0){ // if section 1 button is pressed
    lastActivity = millis();
    pixels.setPixelColor(SectionLights[0], pixels.Color(255, 255, 255)); //set to white because it takes a second to read from sd, and it tells the user the panel is doing something
    pixels.show();
    readSwitches();
    readFile(); // read sd card file named after the number that the switches equal
    if(opened){ // if file exists
      if(rotary == newPos){ // if rotary number matches solution
        pixels.setPixelColor(SectionLights[0], pixels.Color(0, 255, 0)); // green light
        pixels.show();
        tone(speakerPin, 784); //good tone 1
        delay(100);
        tone(speakerPin, 1046); // good tone 2
        delay(200);
        noTone(speakerPin);
        filePrefix = count; // saves file prefix so we can find the images
        lastSubmitCorrectOrIncorrect = 1; // tell serial adapter that they passed this puzzle (outdated) (original way we were going to integrate, and will revise here later on for thorium web server)
        section = 1; // increment the section number
        result = picPath + filePrefix + A + picext; //get filename and path for next puzzle prompt photo
        char file[result.length() + 1]; // creates string array
        strcpy(file, result.c_str());  //changes filename from string to char array, sd libraries are very picky about this
        stat = reader.drawBMP(file, tft, 0, 0); // writes new photo to screen
        opened = false; // resets var for next run
      }
      else{ //if rotary number does not match solution file
        incorrect(1);
      }
    }
    else{ //if no such file exists on the sd card it must be wrong
      delay(928); //pretend we are reading from the file, otherwise it instantly tells you incorrect, and creates inconsistancy of experience
      incorrect(1); //incorrect 'em
    }
    
  }
  pos = myEnc.read(); // get new rotary position
  counter = counter + pos - oldPos;
  if(counter >= 4){ //between every "click" on the rotary encoder there is really an incrementation of 4, kind of weird, i know, but this makes it one click per incrament
    newPos += 1;
    counter -= 4;
  }
  else if(counter <= -4){ // decrament same as incrament, but reverse
    newPos -= 1;
    counter += 4;
  }
  if(newPos < 0){ // can't go less than 0
    newPos = 0;
  }
  else if(newPos > 255){ // nor more than 255
    newPos = 255;
  }
  if (newPos != oldnewPos) { // if position was changed since last time
    oldnewPos = newPos; // remember for next time
    if(newPos < 10){ // if it is single digit, put 3 spaces
      result = Space + Space + Space + newPos;
    }
    else if(newPos<100){ // double digit 2 spaces
      result = Space + Space + newPos;
    }
    else { //  triple digit 1 space
      result = Space + newPos;
    }
    display.print(result); // update display with new number
  }
  oldPos = pos;
}

void section2(){
  tempScan = digitalRead(Section2Button);
  if(tempScan == 0){ // if button was pressed
    lastActivity = millis();
    checkJackStatus(); //check the jack status
    //check if same as solution
    num = 0;
    for(x = 0; x < 5; x++){
      for(y = 0; y < 2; y++){
        #if DEBUGPRINTS
        Serial.print(jackConnections[x][y]);
        Serial.print("\t");
        Serial.print(connections[x][y]);
        Serial.print("\t");
        #endif
        if(jackConnections[x][y] != connections[x][y]){
          num++;
        }
      }
      #if DEBUGPRINTS
      Serial.print("\n");
      #endif
    }
    if(num == 0){ // if there are no differences, get correct sequence underway
      pixels.setPixelColor(SectionLights[1], pixels.Color(0, 255, 0));
      pixels.show();
      tone(speakerPin, 784); // the sound of success
      delay(100);
      tone(speakerPin, 1046);
      delay(200);
      noTone(speakerPin);
      lastSubmitCorrectOrIncorrect = 1;
      section = 2;
      result = picPath + filePrefix + B + picext; // get new image filename
      char file[result.length() + 1];
      i = 0;
      for(x = 0; x < 4; x++){
        for(y = 0; y < 4; y++){
          buttonValues[y][3-x] = findIndex(ProfileConversion[colorProfile],startColors[i]); // set grid starting positions
          i++;
        } 
      }
      strcpy(file, result.c_str()); 
      stat = reader.drawBMP(file, tft, 0, 0); // draw us the puzzle prompt
      checkJackStatus(); //read jack state for ethernet, it is probaby wondering what is happening since we ignored it while drawing image
    }
    else{
      incorrect(2);
    }
  }
  delay(5);// just vibe, no need to check like 1000000 times a second for that button status to be pushed, we will only check 200 times a second ;)
}

void checkJackStatus(){
  for(x = 0; x < 10; x++){ // make sure all jacks are set to pullup position, and reset jack connections array
    pinMode(JackPins[x],INPUT_PULLUP);
    jackConnections[x][0] = 0;
    jackConnections[x][1] = 0;
  }
  i = 0;
  for(x = 0; x < 10; x++){ //go through each jack as a test point
      pinMode(JackPins[x], OUTPUT);
      digitalWrite(JackPins[x], LOW); // set it to ground
      for(y = 0; y < 10; y++){ //check all other pins to see if they are grounded
        if(x != y){// make sure it's not itself
          tempScan = digitalRead(JackPins[y]);
          if(tempScan == 0){ // if it is grounded, there must be a connection!
            if(i == 0){ // if this is the first element in the connections array, just add it
              jackConnections[i][0] = x+1;
              jackConnections[i][1] = y+1;
              i++;
            }
            else{ // if not, check that it hasn't been added already (other side of cable will also show conneciton)
              for(num = 0; num < i+1; num++){
                if(round(x + 1) == round(jackConnections[num][1]) && round(y + 1) == round(jackConnections[num][0])){
                  tempScan = 1;
                }
              }
              if(tempScan == 0){ // if it is new, add it already!
                jackConnections[i][0] = x+1;
                jackConnections[i][1] = y+1;
                i++;
              }
            }
          }
        }
      }
      digitalWrite(JackPins[x], HIGH); // set pin to a high state, so it isnt grounded anymore
      pinMode(JackPins[x], INPUT_PULLUP); // set it to pullup
      if(i == 5){
        break;// if array is full, (5 connections), exit the loop, because we only have 10 pins, and so if we have 5 pairs that is all possible connections. (unless there is an abomination 3+ connection somewhere)
      }
    }
}

void section3(){ // the fabled section 3 *Dramatic Mucic*
  updateButtons(); // update button colors
  checkButtonStatus(); // read if any have been pressed
  tempScan = digitalRead(Section3Button);
  num = 0;
  i = 0;
  if(tempScan == 0){ // if button has been pushed
    lastActivity = millis();
    tempScan = 1;
    for(x = 0; x < 4; x++){
      for(y = 0; y < 4; y++){
        if(endColors[i] != ProfileConversion[colorProfile][buttonValues[y][3-x]]){ // compare button status to the solution
          num++;
        }
        i++;
      }
    }
    if(num != 0){ // if there is even a single error, tell the user they are incorrect
      incorrect(3);
    }
    else{
      pixels.setPixelColor(SectionLights[2], pixels.Color(0, 255, 0)); //set light to green
      pixels.show();
      lastSubmitCorrectOrIncorrect = 1;
      sendData(); // give user feedback and reset
    }
  }
}

void checkButtonStatus(){
  for(x = 0; x < 4; x++){ // for the keen of eye, very similar to the jack connection tester, except it writes pin states into an array
    digitalWrite(GridPins[0][x],LOW);
    for(y = 0; y < 4; y++){
      tempScan = digitalRead(GridPins[1][y]);
      if(tempScan == 0 && prev[x][y] != 0){ // if button is pressed
        prev[x][y] = 0;
        buttonValues[x][y] += 1; // incrament state counter
        if(buttonValues[x][y] >= ColorNums[colorProfile]){ // if cycled through them all
          buttonValues[x][y] = 0; // set back to 0
        }
      }
      prev[x][y] = tempScan;
    }
    digitalWrite(GridPins[0][x],HIGH);
    delay(5);
  }
}

void updateButtons(){
  for(x = 0; x < 4; x++){
    for(y = 0; y < 4; y++){
      lookingAt = buttonNums[x][y];
      pixels.setPixelColor(lookingAt-1, pixels.Color(Colors[ProfileConversion[colorProfile][buttonValues[x][y]]][0],Colors[ProfileConversion[colorProfile][buttonValues[x][y]]][1],Colors[ProfileConversion[colorProfile][buttonValues[x][y]]][2]));
    } //set each grid button to the proper color for the state it is in using the profile conversion and color conversion arrays
  }
  pixels.show();
}

void readFile(){
  myFile = SD.open(count + ext); // open solution file
  if (myFile) { // file exists
    opened = true;
    myFile.seek(0); // go to start
    i = 0;
    while(myFile.available()){ // while there is file left to explore
      i++; // incrament counter
      if(i == 1){
        rotary = myFile.parseInt(); // if first, grab proper rotary position
      }
      else if(i >= 2 && i<= 11){ // if 2nd to 11th, store all that glorious jack data
        if(i % 2 == 0){
          connections[(i-2)/2][0] = myFile.parseInt();
        }
        else{
          connections[(i-3)/2][1] = myFile.parseInt();
        }
      }
      else if(i == 12){ // grab that color profile
        colorProfile = myFile.parseInt() - 1;
      }
      else if(i > 12 && i <= 28){ //  steal those starting button states
        startColors[i-13] = myFile.parseInt();
      }
      else if(i > 28 && i <= 44){ // prepare the button solution states
        endColors[i-29] = myFile.parseInt();
      }
      else{ //  if there is other garbage thrown in our holy files, just ignore it
        myFile.parseInt();
      }
    }
    myFile.close(); // close the file
  } else {
    // if the file didn't open, nothing is done, and section 1 will know, and will let the user know that they were wrong
  }
 }

 void sendData(){
  delay(100);
  tone(speakerPin, 784); // good noise
  delay(100);
  tone(speakerPin, 1046);
  delay(200);
  noTone(speakerPin);
  stat = reader.drawBMP("/Pictures/success.bmp", tft, 0, 0); // tell the user they have defeated that subsystem readjustment
  lastSubmitCorrectOrIncorrect = 1;
  delay(2500); // give them time to read their success and feel true joy
  reset();
 }
void reset(){
  lastCompletedSubsystem = filePrefix.toInt();
  display.print("INIT");
  pixels.clear();
  pixels.show();
  stat = reader.drawBMP("/Pictures/logo.bmp", tft, 0, 0); //draws ship logo stored on the sd card
  display.clear(); //display and tft can be easily mixed up, display is the 7 segment display and tft is the screen
  oldPos = myEnc.read();
  result = Space + Space + Space + newPos; //formats the number nice
  display.print(result);
  section = 0; // reset section variable
}
 byte findIndex(const byte list[], byte valueWanted){ // function to find a specific value in an array
  for(byte j = 0; j < 7; j++){
    if(valueWanted == list[j]){
      return(j);
      break;
    }
  }
  return 0;
 }

void checkForLastUpateRestart(){
  if(section != 0 && millis() - lastActivity >= 600000) {//if inactive for 10 min and not on base screen
    lastActivity = millis();
    incorrect(section+1);
    reset();
  }
}

void communicateWithEthernetController(){
  if(Serial.available() >= 6){ //if we have a complete (or almost complete) message waiting
    String request = Serial.readStringUntil('\n');
    request.replace("\r", "");
    if(request.equals("Initialize")){
      Serial.println("OK");
    }
    else if(request.equals("Identify")){
      Serial.println(PanelName + " " + VersionNumber);
    }
    else if(request.equals("Update")){
      if(section == 0){
        readSwitches();
      }
      else if(section == 1){
        checkJackStatus();
      }

      Serial.print("$u "); //the command denoting that this is an update

      Serial.print(section);
      Serial.print(' ');
      Serial.print(oldnewPos);
      Serial.print(' ');
      Serial.print(count);
      Serial.print(' ');
      Serial.print(lastSubmitCorrectOrIncorrect);
      Serial.print(' ');
      Serial.print(lastCompletedSubsystem);
      Serial.print(' ');

      Serial.flush();

      for(x = 0; x < 5; x++){
        for(y = 0; y < 2; y++){
          Serial.print(jackConnections[x][y]);
          Serial.print(' ');
        }
      }
      for(x = 0; x < 5; x++){
        for(y = 0; y < 2; y++){
          Serial.print(connections[x][y]);
          Serial.print(' ');
        }
      }

      Serial.flush();


      for(int x = 3; x >= 0; x--){
        for(int y = 0; y < 4; y++){
          Serial.print(ProfileConversion[colorProfile][buttonValues[y][x]]);
          Serial.print(' ');
        }
      }
      Serial.flush();
      for(i = 0; i < 16; i++){
        Serial.print(endColors[i]);
        if(i != 15){
          Serial.print(' ');
        }
        else{
          Serial.println();
        }
      }
    }
  }
}