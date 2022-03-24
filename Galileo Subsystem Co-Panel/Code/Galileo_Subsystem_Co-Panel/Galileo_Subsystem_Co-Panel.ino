//credits, variable declairations and constants in the header

//---------------------------------------------------------------------------------------------------------------------------------


// FOR CHANGING SETTINGS PLEASE REFER TO GalileoSubys.h, it is not in this file, just don't want you getting lost and confused :)
// and thank you for working on this panel, you guys really are the ones who make it fun!


//---------------------------------------------------------------------------------------------------------------------------------

#include "GalileoSubsys.h"
#define VERSIONNUMBER "Galileo Subsystem Co-Panel Version 1.0 (Release)"

void setup() {
  Serial.begin(9600);// start serial and print version number
  Serial.println(VERSIONNUMBER);
  for(i = 0; i < 3; i++){// set inputs and outputs
    pinMode(EncoderButtons[i], INPUT_PULLUP);
  }
  for(i = 0; i < 15; i++){
    pinMode(jackPlugDetectPins[i],INPUT_PULLUP);
    pinMode(jackPlugCheckStartPin + i,INPUT_PULLUP);
  }
  for(x = 0; x < 4; x++){
    pinMode(buttonGridPins[0][x], OUTPUT);
    pinMode(buttonGridPins[1][x], INPUT_PULLUP);
    digitalWrite(buttonGridPins[0][x], HIGH);
  }
  pinMode(dataPin,OUTPUT);
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  rgbLights.begin(); // start the lights and set brightness
  rgbLights.clear();
  rgbLights.show();
  rgbLights.setBrightness(50);
}

void loop() {
  gridButtonInputChecks();
  diceUpdate();
  updateCircleSelections();
  jacks();
  updateLights();
  updateShiftRegister();
  programLightsUpdate();
}

void programLightsUpdate() {
  for(int columb = 0; columb <  3; columb++ ) { // for each column
    if(programLightsPosition[columb] < 0 && gridStat[0][columb]){ // check if the button has been pressed
      programLightsPosition[columb] = 0; // activate program
      shiftRegisterOut[columb] = 1; // turn on button light
      lastProgramUpdate[columb] = millis();
    }
    else if(programLightsPosition[columb] >= 0){ // if program is running
      bool desiredSwitchState = digitalRead(programSwitchPins[columb * 5 + programLightsPosition[columb]]); // get if that rocker is enabled
      if(!desiredSwitchState){ // if it isn't
        while(!desiredSwitchState &&  programLightsPosition[columb] <= 5){// search and find the next available rocker
          programLightsPosition[columb]++;
          if(programLightsPosition[columb] < 5)
            desiredSwitchState = digitalRead(programSwitchPins[columb * 5 + programLightsPosition[columb]]);
        }
      }
      if(programLightsPosition[columb] < 5){ // if we didn't go out of bounds of the rockers (that could only mean we finished program or couln't find any switches on)
        if(!programFlash[columb][programLightsPosition[columb]]){ // if that switch is a non twinkly program command
          for(i = 0; i < 3; i++){ // set the neopixel to that switches color
            PixelStates[ProgramNeopixelStart + (columb)][i] = programColor[columb][programLightsPosition[columb]][i];
          }
          if(millis() - lastProgramUpdate[columb] > PROGRAMLIGHTSDELAY){ // if enough time has elapsed
            lastProgramUpdate[columb] = millis();
            programLightsPosition[columb]++; // increment to next switch
          }
        }
        else {
          for(i = 0; i < 3; i++){ // set the neopixel to that switches color + enable or disable it based on flashbit (tells if it is on or off atm)
            PixelStates[ProgramNeopixelStart + (columb)][i] = programColor[columb][programLightsPosition[columb]][i] * flashbit[columb];
          }
          if(millis() - lastProgramUpdate[columb] > PROGRAMFLASHDELAY){ // if we have finished one flash on/off
            numFlashProg[columb]++; // increase recorded number of flashes
            lastProgramUpdate[columb] = millis(); 
            flashbit[columb] = !flashbit[columb]; // set flashbit to opposite
            if(numFlashProg[columb] >= PROGRAMLIGHTSDELAY/PROGRAMFLASHDELAY){ // if enough time has elapsed (we flashed enough times)
              numFlashProg[columb] = 0; // reset all vars
              flashbit[columb] = true;
              programLightsPosition[columb]++; // increment to next switch
            }
          }
        }
      }
    }
    if(programLightsPosition[columb] >= 5){ // if we went through all switches
      programLightsPosition[columb] = -1; // set program running to fasle
      shiftRegisterOut[columb] = 0; // turn off the light to the button
      for(i = 0; i < 3; i++){ // clear the neopixel
        PixelStates[ProgramNeopixelStart + (columb)][i] = 0;
      }
    }
  }
}

void diceUpdate(){
  for(byte side = 0; side < 2; side++){// do for both sides
    byte gridIndex = (1-side)*2+1; // get the grid inex on the button grid
    byte variantNumber = 0; // initialise variable
    for(byte Bit = 0; Bit < 4; Bit++){ // convert that binary we got from the button grid, and turn it into a number we can use
      if(gridStat[gridIndex][LRAdapt[Bit]]){ // if switch is on
        bitSet(variantNumber, Bit); // turn on a bit in the byte we have here
      }
    }
    // next here we set the lights on or off based on the variant number and the preset cases
    shiftRegisterOut[buttonShiftRegisterOffset + side * 4] = DiceLightStat[side][variantNumber][1]; 
    shiftRegisterOut[buttonShiftRegisterOffset + side * 4 + 1] = DiceLightStat[side][variantNumber][0];
    shiftRegisterOut[buttonShiftRegisterOffset + side * 4 + 2] = DiceLightStat[side][variantNumber][6];
    shiftRegisterOut[buttonShiftRegisterOffset + side * 4 + 3] = DiceLightStat[side][variantNumber][5];
    for(x = 0; x < 3; x++){ // set the neopixels to the correct state for that variant number
      for(y = 0; y < 3; y++){
        PixelStates[DieNeopixelStart + x + side * 3][y] = DiceColors[variantNumber][y] * DiceLightStat[side][variantNumber][2 + x];
      }
    }
  }
}

void gridButtonInputChecks(){ // this is how we drive 15 inputs with only 8 inputs
  for(x = 0; x < 4; x++){ // for each row
    pinMode(buttonGridPins[0][x], OUTPUT); // set row to low
    digitalWrite(buttonGridPins[0][x], LOW);
    //delayMicroseconds(1); // delay for a millisecond to apply (i dont think this is necissary, but i dont have panel next to me so if it acts weird un comment this)
    for(y = 0; y < 4; y++){ // for each column
      if(!digitalRead(buttonGridPins[1][y])){ // if that column is low (button has been pressed)
        gridStat[x][y] = true; // set it to true
      }
      else{ // if not set it to false
        gridStat[x][y] = false;
      }
    }
    digitalWrite(buttonGridPins[0][x], HIGH);
    //delayMicroseconds(1); // delay for a millisecond to apply (i dont think this is necissary, but i dont have panel next to me so if it acts weird un comment this)
    pinMode(buttonGridPins[0][x], INPUT_PULLUP);
  }
}

void jacks(){
  if(currJackCommand == 0){ // if we arent running a diagnostic or transfering data
    testIfPlugged(); // test to see if they are plugged
    testInterPinConnections(); // test to see where they are connected
    renderJacks(); // turn on lights
    if(gridStat[2][0]){ // if bypass has been activated
      currJackCommand = 1; // set command
      jackCommandStart = millis();
    }
    else if(gridStat[2][1]){ // if diagnostic has been activated
      currJackCommand = 2;// set command
      jackCommandStart = millis();
      jackAnimationTracker = 0; // reset animaiton tracker
    }
    else if(gridStat[2][2]){ // if data transfer has been initiated
      currJackCommand = 3;// set command
      jackCommandStart = millis();
      jackAnimationTracker = 0;// reset animaiton tracker
      randomSeed(millis()); // seed the random with milliseconds the arduino has been on
      jackAnimationExit = random(8,32); // generate random length of how long it will take to transfer data
      for(int i = 0; i < 15; i++){ // reset all pixels to off
        PixelStates[jackNeopixelStartOffset + i][0] = 0;
        PixelStates[jackNeopixelStartOffset + i][1] = 0;
        PixelStates[jackNeopixelStartOffset + i][2] = 0;
      }
    }
    else if(gridStat[2][3]){ // if a system has need to be reinitallised
      currJackCommand = 4;// set command
      jackCommandStart = millis();
    }
  }
  else if(currJackCommand == 1){ //Bypass
    currJackCommand = 0; // set command back to nothing
    for(int i = 0; i < 15; i++){ // find all second plugged connections and set them to bypassed, persistant until reinitallised
      if(jackIsPlugged[i] && jackConnectedTo[i] != -1 && jackWasPlugged[i] > jackWasPlugged[jackConnectedTo[i]]){
        bypassed[i] = true;
      }
    }
  }
  else if(currJackCommand == 2){ // Diagnostic
    if(millis() - jackCommandStart >= jackAnimationTracker * 500){
      if(jackAnimationTracker > 0){ // if this isnt the first pixel, reset pixel behind
        PixelStates[jackNeopixelStartOffset + (jackAnimationTracker - 1)%15][0] = 0;
        PixelStates[jackNeopixelStartOffset + (jackAnimationTracker - 1)%15][1] = 0;
        PixelStates[jackNeopixelStartOffset + (jackAnimationTracker - 1)%15][2] = 0;
      }
      else{
        for(int i = 0; i < 15; i++){ // if this is first time, clear all the pixels
          PixelStates[jackNeopixelStartOffset + i][0] = 0;
          PixelStates[jackNeopixelStartOffset + i][1] = 0;
          PixelStates[jackNeopixelStartOffset + i][2] = 0;
        }
      }
      // set current pixel to blue
      PixelStates[jackNeopixelStartOffset + jackAnimationTracker % 15][0] = 0;
      PixelStates[jackNeopixelStartOffset + jackAnimationTracker % 15][1] = 0;
      PixelStates[jackNeopixelStartOffset + jackAnimationTracker % 15][2] = 255;
      jackAnimationTracker++; // increment pixel
    }
    if(jackAnimationTracker >= 45){ // if we went through all jacks 3 times
      jackAnimationTracker = 0; // reset back to default
      currJackCommand = 0;
      for(int i = 0; i < 15; i++){ // set second plugged jacks to white, that is persistant until unplugged
        if(jackIsPlugged[i] && jackConnectedTo[i] != -1 && jackWasPlugged[i] > jackWasPlugged[jackConnectedTo[i]]){
          jackSpecialColor[i] = 1;
        }
      }
    }
  }
  else if(currJackCommand == 3){ // Transfer Data
    if(millis() - jackCommandStart > jackAnimationTracker * 500){ // if it has been half a second
      if(jackAnimationTracker % 3 == 0){ // if it is 1st frame
        for(int i = 0; i < 15; i++){// clear pixel state of second plug
          if(jackIsPlugged[i] && jackConnectedTo[i] != -1 && jackWasPlugged[i] > jackWasPlugged[jackConnectedTo[i]]){
            PixelStates[jackNeopixel[i]][0] = 0;
            PixelStates[jackNeopixel[i]][1] = 0;
            PixelStates[jackNeopixel[i]][2] = 0;
          }  // set first plug to blue
          else if(jackIsPlugged[i] && jackConnectedTo[i] != -1 && jackWasPlugged[i] < jackWasPlugged[jackConnectedTo[i]]){
            PixelStates[jackNeopixel[i]][0] = 0;
            PixelStates[jackNeopixel[i]][1] = 0;
            PixelStates[jackNeopixel[i]][2] = 255;
          }
        }
      }
      if(jackAnimationTracker % 3 == 1){ // if it is second frame
        for(int i = 0; i < 15; i++){// set second plug to blue
          if(jackIsPlugged[i] && jackConnectedTo[i] != -1 && jackWasPlugged[i] > jackWasPlugged[jackConnectedTo[i]]){
            PixelStates[jackNeopixel[i]][0] = 0;
            PixelStates[jackNeopixel[i]][1] = 0;
            PixelStates[jackNeopixel[i]][2] = 255;
          }   // clear the first plug light
          else if(jackIsPlugged[i] && jackConnectedTo[i] != -1 && jackWasPlugged[i] < jackWasPlugged[jackConnectedTo[i]]){
            PixelStates[jackNeopixel[i]][0] = 0;
            PixelStates[jackNeopixel[i]][1] = 0;
            PixelStates[jackNeopixel[i]][2] = 0;
          }
        }
      }
      if(jackAnimationTracker % 3 == 2){ // if it is 3rd frame
        for(int i = 0; i < 15; i++){ // clear all pixels
          PixelStates[jackNeopixel[i]][0] = 0;
          PixelStates[jackNeopixel[i]][1] = 0;
          PixelStates[jackNeopixel[i]][2] = 0;
        }
      }
      jackAnimationTracker++; // increment frame counter
    }
    if(jackAnimationTracker >= 3 * jackAnimationExit){ // if we have gone through all the animations required
      currJackCommand = 0;// reset vars
      jackAnimationTracker = 0;
    }
  }
  else if(currJackCommand == 4){ // Reinitiallise
    currJackCommand = 0; // reset vars
    for(int i = 0; i < 15; i++){ // remove bypassing from second plugged jack
      if(jackIsPlugged[i] && jackConnectedTo[i] != -1 && jackWasPlugged[i] > jackWasPlugged[jackConnectedTo[i]]){
        bypassed[i] = false;
      }
    }
  }
  else{ // if we have bad command, just reset it, this should never happen, but who knows what's possible when there are both hackers and small children involved
    Serial.print("Error, jack command corrupted, Resetting [=====>] : ");
    currJackCommand = 0;
    Serial.println("Reset.");
  }
}

void renderJacks(){
  for(int i = 0; i < 15; i++){ // go through each jack
    if(bypassed[i]){ // if it is bypassed set it to red
      PixelStates[jackNeopixel[i]][0] = 255;
      PixelStates[jackNeopixel[i]][1] = 0;
      PixelStates[jackNeopixel[i]][2] = 0;
    }
    else if(jackIsPlugged[i] && jackConnectedTo[i] == -1){// if only one side is plugged
      jackSpecialColor[i] = 0;  // reset diagnostic light to regular if it was in debug mode
      PixelStates[jackNeopixel[i]][0] = 255; // set to yellow
      PixelStates[jackNeopixel[i]][1] = 255;
      PixelStates[jackNeopixel[i]][2] = 0;
    }
    else if(!jackIsPlugged[i]){ // if it is unplugged
      for(y = 0; y < 3; y++){ // clear color on jacks
        PixelStates[jackNeopixel[i]][y] = 0;
      }
      jackSpecialColor[i] = 0; // if it was white from diagnostic clear
    }
    else{
      if(jackSpecialColor[i] == 1){ // if diagnostic light, set to white
        PixelStates[jackNeopixel[i]][0] = 255;
        PixelStates[jackNeopixel[i]][1] = 255;
        PixelStates[jackNeopixel[i]][2] = 255;
      }
      else if(jackWasPlugged[i] < jackWasPlugged[jackConnectedTo[i]]){// if it was plugged in first , set to green
        PixelStates[jackNeopixel[i]][0] = 0;
        PixelStates[jackNeopixel[i]][1] = 255;
        PixelStates[jackNeopixel[i]][2] = 0;
      }
      else if(jackWasPlugged[i] > jackWasPlugged[jackConnectedTo[i]]){ // if it was plugged in second, set to light blue (my favorite color, thats why it is in every panel)
        PixelStates[jackNeopixel[i]][0] = 0;
        PixelStates[jackNeopixel[i]][1] = 255;
        PixelStates[jackNeopixel[i]][2] = 255;
      }
      else{ // if we have weird problem, set to purple (looks pretty cool actually)
        PixelStates[jackNeopixel[i]][0] = 255;
        PixelStates[jackNeopixel[i]][1] = 0;
        PixelStates[jackNeopixel[i]][2] = 255;
      }
    }
  }
}

void testInterPinConnections(){ 
  for(i = 0; i < 15; i++){
    jackConnectedTo[i] = -1; // clear all old data
  }
  for(x = 0; x < 15; x++){ // go through each jack
    pinMode(jackPlugCheckStartPin + x, OUTPUT); // set jack low
    digitalWrite(jackPlugCheckStartPin + x, LOW);
    for(y = 0; y < 15; y++){ // go through each jack to test for conneciton
      if(!digitalRead(jackPlugCheckStartPin + y)){ // if it is also low!
        if(y > x){ // if second plug is higher up (just so we dont record each connection twice because it has two sides)
          jackConnectedTo[x] = y; // set connections to eachother
          jackConnectedTo[y] = x;
          if(!jackIsPlugged[x]){ // if it wasn't plugged for some reason (not pushed in all the way for example)
            jackIsPlugged[x] = true; // set plugged to true
            jackWasPlugged[x] = millis(); // record this time as the time it was plugged
          }
          if(!jackIsPlugged[y]){ // if it wasn't plugged for some reason (not pushed in all the way for example)
            jackIsPlugged[y] = true; // set plugged to true
            jackWasPlugged[y] = millis(); // record this time as the time it was plugged
          }
        }
      }
    }
    digitalWrite(jackPlugCheckStartPin + x, HIGH); // se that jack high again and an input
    pinMode(jackPlugCheckStartPin + x, INPUT_PULLUP);
  }
}

void testIfPlugged(){
  for(x = 0; x < 15; x++){ // set each jack dected pin low
    pinMode(jackPlugCheckStartPin + x, OUTPUT);
    digitalWrite(jackPlugCheckStartPin + x, LOW);
  }
  delayMicroseconds(1); // wait a millisecond to apply
  for(y = 0; y < 15; y++){ // go through each jack
    if(digitalRead(jackPlugDetectPins[y])){ // if it is high no connecion to detect pin, meaning something was plugged!
      if(!jackIsPlugged[y]){ // if it wasn't plugged before
        jackIsPlugged[y] = true; // set it to plugged
        jackWasPlugged[y] = millis(); // record this time as time plugged
      }
    }
    else{ // if not plugged set its state to false
      jackIsPlugged[y] = false;
    }
    digitalWrite(jackPlugCheckStartPin + y, HIGH); // set that detect pin High agian
  }
  delayMicroseconds(1); // delay a microsecond to apply
  for(x = 0; x < 15; x++){
    pinMode(jackPlugCheckStartPin + x, INPUT_PULLUP); // set dectedt pins to inputs again.
  }
}

void updateCircleSelections(){
  if(millis() - lastBlinkTime > BLINKDELAY){ // if it is time to change blink state (used for cursor/pixel blink)
    blinkoff = !blinkoff; // set it to opposite
    lastBlinkTime = millis();
  }
  for(y = 0; y < 21; y++){// reset all pixel states for the circles
    PixelStates[y][0] = 0;
    PixelStates[y][1] = 0;
    PixelStates[y][2] = 0;
  }
  for(x = 0; x < 3; x++){ // for each circle
    pos[x] = (*RotaryEncoders[x]).read();// read raw in from encoder
    count[x] += pos[x] - oldPos[x]; // find the change, and store it in count
    if(count[x] >= 4){ // if it has increased by 4 (one click on potentiometer wheel)
      int temp = (byte)(count[x] / 4); // find out how many times it is divisible by
      selected[x] += temp; // add those to selecion var
      selected[x] = selected[x] % 7; // keep selecion var in bounds
      count[x] = count[x] % 4; // subtract from count what we took from it and added to selected
    }
    else if(count[x] <= -4){ // if it had decrased by 4 (one click)
      int temp = (int)(count[x] / 4); // get change
      if(abs(temp) > abs(selected[x])){ // if selected is smaller than amount to subtract
        selected[x] = 6; // set selected to max value
        count[x] = count[x] + 4; // only increae count by one click, because we only moved by one click
      }
      else{ // if selected is bigger
        selected[x] += temp; // bring it on
        selected[x] = selected[x] % 7; // keep selected in range
        count[x] = count[x] % 4; // add to count what we took from it and subtarcted from selected
      }
    }
    PixelStates[selected[x]+7*x][1] = 255;// draw cursor
    PixelStates[selected[x]+7*x][0] = 255;
    oldPos[x] = pos[x]; // set old pos to pos
    if(!digitalRead(EncoderButtons[x])){ // if button has been pressed
      if(!debounceEncoder[x]){ // and it is the first we have heard of it
        litUp[x][selected[x]] = !litUp[x][selected[x]]; // change the state of the current pixel
      }
      debounceEncoder[x] = 1; // say we've heard about this button press
    }
    else{ // if not pressed
      debounceEncoder[x] = 0; // record that down
    }
    for(y = 0; y < 7; y++){ // for each pixel
      if(y != selected[x]){ // if it isn't the current selection
        PixelStates[y+7*x][x] = 255*litUp[x][y]; // set pixel state based on if it is lit or not
      }
      else{// if it is on the current selection
        if(litUp[x][y]){ // if it is lit up
          if(blinkoff){ // if it is the cursors turn to show (changed back and forth at the top of this function)
            PixelStates[y+7*x][x] = 0;// blank color of pixel
            PixelStates[y+7*x][1] = 255;//show cursor
            PixelStates[y+7*x][0] = 255;
          }
          else{ // if it is the pixel's turn
            PixelStates[y+7*x][0] = 0;//clear cursor
            PixelStates[y+7*x][1] = 0;
            PixelStates[y+7*x][x] = 255;//show pixel
          }
        }
      }
    }
  }
}

void updateLights(){
  if(millis() - lastLightUpdate >= 20){ // if it is time to update lights again
    lastLightUpdate = millis();
    for(i = 0; i < 60; i++){// go through each pixel and update it
      rgbLights.setPixelColor(i, rgbLights.Color(PixelStates[i][0],PixelStates[i][1],PixelStates[i][2]));
    }
    rgbLights.show(); // write to pixels
  }
}

void updateShiftRegister(){ 
  if(millis()-lastShiftRegisterUpdate >= 20){ // if it is time to update registers agian
   lastShiftRegisterUpdate = millis();
   byte leds = 0; // define some temp vars
   byte led2 = 0;
   for(i = 0; i <  16; i ++){ // go through each input and write current states to binary we can send out
    if(shiftRegisterOut[i]){
      if(i < 8){
        bitSet(leds,i);
      }
      else{
        bitSet(led2,i-8);
      }
    }
   }
   digitalWrite(latchPin, LOW); // tell the chips we are talking to them
   shiftOut(dataPin, clockPin, MSBFIRST, led2); // write second registers info
   shiftOut(dataPin, clockPin, MSBFIRST, leds); // write first registers info
   digitalWrite(latchPin, HIGH); // tell them they have recived all there is to recive
  }
}
