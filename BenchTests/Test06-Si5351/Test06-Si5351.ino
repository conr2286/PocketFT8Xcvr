/*
NAME
  Test06-Si5351 -- Test the Si5351 clock generator

DESCRIPTION
  Initializes the Si5351 and programs CLK0 and CLK2.  If successful,
  CLK0 (RCLK, the receiver clock) will output a ### kHz signal, and
  CLK2 (XCLK, the transmitter clock) will output a 14100 kHz signal.
  Both signals can be confirmed with a frequency counter.

EXERCISED
  + I2C bus
  + Si5351 connectivity
  + TCXO and Si5351 operation

NOTE
  + This is the first use of the TCXO whose 25.000 mHz signal
  appears on its pin 3 and on Si5351 pin 6.
  + The CLK0 signal appears on Si5351 pin 13 and Si4735 pin 19.
  + The CLK1 signal appears on Si5351 pin 10, C20, and GVA-84+ pin 1.
  + The PTT signal is not exercised and consequently not the RF chain

REFERENCES


ATTRIBUTION
  KQ7B 

*/


#include <Wire.h>

// Set I2C bus to use: Wire, Wire1, etc.
#define WIRE Wire

void setup() {
  WIRE.begin();

}


void loop() {
 
  delay(5000);           // wait 5 seconds for next scan
}
