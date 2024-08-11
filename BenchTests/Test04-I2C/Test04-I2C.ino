/*
NAME
  Test04-I2C -- Enumerate devices on the I2C bus

DESCRIPTION
  Prints a table of all responding I2C device addresses.  If it works, you should 
  see table entries for the MCP3422 (0x68), SI4735 (0x11 or 0x63)  and  the
  SI5351 (0x60).

EXERCISED
  + I2C bus
  + MCP3422 connectivity
  + SI4735 connectivity
  + SI5351 connectivity

NOTE

REFERENCES
  1. https://learn.adafruit.com/scanning-i2c-addresses/arduino
  2. https://playground.arduino.cc/Main/I2cScanner/


ATTRIBUTION
  KQ7B with major leverage from [1] 
  MIT License, (C) 2023 Carter Nelson for Adafruit Industries
  Modified from https://playground.arduino.cc/Main/I2cScanner/

*/

// SPDX-FileCopyrightText: 2023 Carter Nelson for Adafruit Industries
//
// SPDX-License-Identifier: MIT
// --------------------------------------
// i2c_scanner
//
// Modified from https://playground.arduino.cc/Main/I2cScanner/
// --------------------------------------

#include <Wire.h>

// Set I2C bus to use: Wire, Wire1, etc.
#define WIRE Wire

void setup() {
  WIRE.begin();

  Serial.begin(9600);
  while (!Serial)
     delay(10);
  Serial.println("\nI2C Scanner");
}


void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000);           // wait 5 seconds for next scan
}
