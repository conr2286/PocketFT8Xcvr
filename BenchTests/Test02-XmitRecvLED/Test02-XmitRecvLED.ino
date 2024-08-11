/*
NAME
  Test02-XmitRecvLED --- Blink the PocketFT8Xcvr XMIT LED

DESCRIPTION
  This is the first test (well... after smoke) of the PocketFT8Xcvr PCB.

EXERCISED
  It confirms the transceiver's Transmit/Receive switching circuit responds
  to the Teensy GPIO PTT pin by blinking the XMIT LED.

NOTE
  Currently, Charlie's PocketFT8 code uses the same pin (13) for PTT 
  as the Teensy 4.1 on-board LED.  This test is more-or-less superfluous
  unless somebody changes the PTT pin assignment.

ATTRIBUTION
  Adapted by KQ7B from public domain work by Scott Fitzgerald, Arturo Guadalupi, and Colby Newman

*/

//Exercise the Push-to-Talk Pin
#define PTT_Pin 13


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(PTT_Pin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(PTT_Pin, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(PTT_Pin, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}
