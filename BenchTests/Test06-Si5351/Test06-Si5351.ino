/**
NAME
  Test06-Si5351 -- Test the Si5351 clock generator

DESCRIPTION
  Initializes the Si5351 and programs CLK0 and CLK2.  If successful,
  CLK0 (XCLK, the transmitter clock) will output a 14100 kHz signal, and
  CLK2 (RCLK, the receiver clock) will output a 14200 kHz signal.
  Both signals should be confirmed with a frequency counter.

EXERCISED,
  + I2C bus
  + Si5351 connectivity
  + TCXO and Si5351 operation

NOTE
  + This is the first use of the TCXO whose 25.000 mHz signal
  appears on its pin 3 and on the Si5351 CLKIN pin 6
  + The CLK0 XCLK signal appears on Si5351 pin 13, C20 and U3 pin 1
  + The CLK2 RCLK signal appears on Si5351 pin 9 and Si4735 pin 19
  + The PTT signal is not exercised and consequently not the RF chain
  + V2.00 hardware moved the SI5351 chip to the Wire1 bus from Wire


REFERENCES


ATTRIBUTION
  KQ7B 

**/

#include "si5351.h"
#include <Wire.h>

// Set I2C bus to use: Wire, Wire1, etc.
#define WIRE Wire1

Si5351 si5351;

void setup() {
  bool err; 

  Serial.begin(9600);
  delay(100);
  Serial.println("Starting...");
  delay(100);

  //while(!Serial.available()) {  Serial.print("> "); delay(1000);}
  //String s = Serial.readString();
  //Serial.println("\nContinuing...\n");

  //Initialize SI5351 (load cap won't actually be used with CLKIN) for Pocket FT8
  Serial.println("\nInitialize the SI5351");
  bool found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25000000UL, 0);
  if (!found) {
    Serial.println("Error:  SI5351 not found");
    while (1)
      ;  //Hang here
  }

  //Configure SI5351 PLLs to use external CLKIN rather than XTAL
  Serial.println("\n Configuring PLLA for CLKIN...");
  si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_CLKIN);
  //si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_CLKIN);

  //Configure CLK0 for 10 mHz
  Serial.println("\n Set freq of CLK0...");
  err = si5351.set_freq(1400000000UL, SI5351_CLK0);
  if (err) {
    Serial.println("Error:  set_freq(...,CLK0");
    while (1)
      ;
  }


  err = si5351.set_freq(1810000000UL, SI5351_CLK2);
  if (err) {
    Serial.println("Error:  set_freq(...,CLK2");
    while (1)
      ;
  }

  Serial.printf("Test complete\n");

}


void loop() {

  //Serial.println("Loop...");


/*
  //Output SI5351 status every 5 seconds
  si5351.update_status();
  Serial.print("SYS_INIT: ");
  Serial.print(si5351.dev_status.SYS_INIT);
  Serial.print("  LOL_A: ");
  Serial.print(si5351.dev_status.LOL_A);
  Serial.print("  LOL_B: ");
  Serial.print(si5351.dev_status.LOL_B);
  Serial.print("  LOS: ");
  Serial.print(si5351.dev_status.LOS);
  Serial.print("  REVID: ");
  Serial.println(si5351.dev_status.REVID);
*/

  delay(1000);

}
