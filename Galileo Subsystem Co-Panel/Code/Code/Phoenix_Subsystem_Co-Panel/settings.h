#include "arduino.h"
//What light numbers (0-11) will be effected by each switch 
//values less than 0 (value < 0) means no value, ie: 2,4,5,-1 it will only effect pins 2,4, and 5.
const byte LightsEffected[2][12][4] = {{
  {0,5,7,11},
  {1,2,10,11},
  {0,9,10,11},
  {3,7,9,10},
  {2,6,9,11},
  {1,3,8,9},
  {1,2,5,8},
  {0,5,6,7},
  {2,-1,-1,-1},
  {4,-1,6,-1},
  {4,5,9,1},
  {3,1,-1,11}},{
  
  {0,1,2,5},
  {2,6,-1,10},
  {4,-1,-1,8},
  {3,7,10,3},
  {4,11,6,10},
  {5,6,9,11},
  {0,-1,3,7},
  {0,-1,-1,6},
  {1,4,7,10},
  {2,3,6,8},
  {3,7,10,11},
  {1,-1,6,11}}};
//Values to be added to all lights in the list
const byte colorAdd[2][12][3] = {{
 {200,0,0},
 {50,100,0},
 {0,0,50},
 {175,50,150},
 {0,170,0},
 {0,0,220},
 {50,80,10},
 {55,50,0},
 {120,0,50},
 {10,50,50},
 {50,20,140},
 {50,0,50}},{
 
 {50,0,100},
 {0,50,150},
 {0,220,30},
 {210,20,50},
 {50,50,0},
 {150,150,150},
 {10,70,0},
 {30,10,0},
 {160,0,50},
 {0,10,200},
 {50,50,0},
 {80,0,170}}};
 
 //this is the amount of possible notes you want to have, the max is sadly 256 notes. beats per minute, pretty much exactly what it sounds like
#define sheetMusicLength 30
#define BeatsPerMinute 50.0

#define Ab -1
#define A  0
#define As 1
#define Bb 1
#define B 2
#define C 3
#define Cs 4
#define Db 4
#define D 5
#define Ds 6
#define Eb 6
#define E 7
#define F 8
#define Fs 9
#define Gb 9
#define G 10
#define Gs 11

#define Wn 4
#define Hn 2
#define Qn 1
#define En .5
#define Sn .25
// this is where you define the notes for the sheet music, remember, no duplicate notes per columb. insert key will be your best friend :)
//also, these are actually all numbers, just for simplicity i created an octive of constants for you to use :) A = 0 = 440HZ
const short sheetmusic[2][sheetMusicLength] = {
  {Ab,A ,As,B ,C ,Cs,D ,Ds,E ,F ,Fs, G,Gs,E , E,B ,F ,E ,D ,E ,D ,C ,Cs,Eb,Ab,B ,D ,Ds,F ,B },
  {Ab,A ,As,B ,C ,Cs,D ,Ds,E ,F ,Fs, G,Gs,E , E,B ,F ,E ,D ,E ,D ,C ,Cs,Eb,Ab,B ,D ,Ds,F ,B }
};
// this is what switch will enable each note:
const byte SwitchPerButton[2][sheetMusicLength] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 1, 3, 5, 3, 1, 3, 5, 1, 4, 7, 6, 2, 0, 2, 6, 7},
  { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 1, 3, 5, 3, 1, 3, 5, 1, 4, 7, 6, 2, 0, 2, 6, 7}
};
// this is counting quarter notes as 1 beat
const float NoteDuration[2][sheetMusicLength] = {
  {Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Wn,1 ,Qn,En,En,En,Sn,Sn,En,En, 1, 1,.5,.5,.5,1 ,.5,1 },
  {Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Qn,Wn,1 ,Qn,En,En,En,Sn,Sn,En,En, 1, 1,.5,.5,.5,1 ,.5,1 }
};
