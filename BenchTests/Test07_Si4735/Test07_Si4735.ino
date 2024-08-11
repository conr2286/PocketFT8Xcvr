/*
NAME
  Test07 -- Si4735 Communication and Initialization

DESCRIPTION
  Verifies that Teensy can communicate with and initialize the Si4735 radio
  receiver.

EXERCISED

NOTE

REFERENCES
  1. https://github.com/Rotron/Pocket-FT8/blob/main/Project%20Manual_1.0.pdf
  2. https://oz1bxm.dk/notes/pocket-FT8.html

ATTRIBUTION
  KQ7B and Charlie [1]

*/



#include <Wire.h>



void setup() {

  //Initialize the Arduino world and let console know we're starting
  Serial.begin(9600);
  Serial.println("Starting...");
  delay(100);

}

// the loop function runs over and over again forever
void loop() {
  delay(100);
}
