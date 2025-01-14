/**
NAME
  Test07-Si5351 -- Test the entire transmitter's RF chain

DESCRIPTION
  Initializes the Si5351 and programs CLK0 and CLK2.  If successful,
  CLK0 (XCLK, the transmitter clock) will output a 14100 kHz signal.
  Both signals should be confirmed with a frequency counter at the
  antenna connector.

EXERCISED,
  + I2C bus
  + Si5351 connectivity
  + TCXO and Si5351 operation
  + PTT and transmitter's RF chain
  + V2.00 hardware implements the complementary XMIT and RECV signals in firmware
  + V2.00 hardware moved the SI5351 from the Wire to Wire1 bus

NOTE
  + This is the first use of the TCXO whose 25.000 mHz signal
  appears on its pin 3 and on the Si5351 CLKIN pin 6
  + The CLK0 XCLK signal appears on Si5351 pin 13, C20 and U3 pin 1
  + The CLK2 RCLK signal appears on Si5351 pin 9 and Si4735 pin 19

REFERENCES


ATTRIBUTION
  KQ7B 

**/

#include "si5351.h"
#include <Wire.h>

// Set I2C bus to use: Wire, Wire1, etc.
#define WIRE Wire2

//Exercise the Push-to-Talk Pin
#define PIN_PTT 14
#define PIN_RCV 15

Si5351 si5351;

void setup() {
  bool err;

  Serial.begin(9600);
  delay(100);
  printf("Starting...\n");

  //Initialize SI5351 (load cap won't actually be used with CLKIN) for Pocket FT8
  bool found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25000000, 0L);
  if (!found) {
    printf("Error:  SI5351 not found\n");
    while (1)
      ;  //Hang here
  }

  //Configure SI5351 PLLs to use external CLKIN rather than XTAL
  si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_CLKIN);  //KQ7B V1.00 uses cmos CLKIN, not a XTAL
  si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_CLKIN);  //All PLLs using CLKIN
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(3276800ULL, SI5351_CLK2);  //Receiver's PLL clock (so sad)
  si5351.output_enable(SI5351_CLK2, 1);      //Enable receiver clk (TP3 on PCB)

  //Configure xmit CLK0 for 14.1 mHz
  err = si5351.set_freq(1410000000ULL, SI5351_CLK0);
  if (err) {
    printf("Error:  set_freq(...,CLK0\n");
    while (1)
      ;
  }
  si5351.output_enable(SI5351_CLK0, 1);  //Enable transmitter clk (TP2 on PCB)


  //Turn on the transmitter's RF chain
  pinMode(PIN_RCV, OUTPUT);
  pinMode(PIN_PTT, OUTPUT);
  digitalWrite(PIN_RCV, LOW);   //~RCV
  digitalWrite(PIN_PTT, HIGH);  //XMT
}


void loop() {

  //Output SI5351 status every 10 seconds
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

  delay(10000);
}
