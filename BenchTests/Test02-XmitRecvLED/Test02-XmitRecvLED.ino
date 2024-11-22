/*
NAME
  Test02-XmitRecvLED --- Blink the PocketFT8Xcvr XMIT LED

DESCRIPTION
  This test exercises the PTT signal pin's ability to place the rig
  in its transmit mode

EXERCISED
  It confirms the transceiver's Transmit/Receive switching circuit responds
  to the Teensy GPIO PTT pin by blinking the XMIT LED.

NOTE
  Teensy 3.6 used digital pin 13 in the original Pocket FT8 implementation;
  Teensy 4.1 necessitated a change to pin 14.

ATTRIBUTION
  Adapted by KQ7B from public domain work by Scott Fitzgerald, Arturo Guadalupi, and Colby Newman

*/


//Transmit/Receive pins
#define PIN_PTT 14  //Teensy 4.1
#define PIN_RCV 15  //FW implementation of ~PTT (i.e. RECV) in V2.0 boards


// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600);
  Serial.println("Starting...");
  delay(100);

  //V2.00 hardware uses firmware to derive the XMIT and RECV complementary signals
  pinMode(PIN_PTT, OUTPUT);
  pinMode(PIN_RCV, OUTPUT);
  digitalWrite(PIN_PTT, LOW);
  digitalWrite(PIN_RCV, HIGH);

  delay(5000);

}

// the loop function runs over and over again forever
void loop() {
  Serial.println("Looping...");
  digitalWrite(PIN_RCV, LOW);
  digitalWrite(PIN_PTT, HIGH);  // turn the LED on (HIGH is the voltage level)

  delay(5000);                  // wait for a second
  digitalWrite(PIN_PTT, LOW);   // turn the LED off by making the voltage LOW
  digitalWrite(PIN_RCV,HIGH);
  delay(5000);                  // wait for a while
}
