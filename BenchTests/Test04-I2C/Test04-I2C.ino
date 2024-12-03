/*
NAME
  Test04-I2C -- Enumerate devices on the I2C bus

DESCRIPTION
  Prints a table of all responding I2C device addresses.  If it works, you should 
  see table entries for the MCP3422 (0x68), SI4735 (0x11 or 0x63)  and  the
  SI5351 (0x60).  The code has been revised to scan both Wire and Wire2 I2C busses.

EXERCISED
  + I2C bus accessed through Wire
  + I2C bus accessed through Wire1
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

// Set I2C bus to use: Wire, Wire2, etc.
#define WIRE Wire
#define WIRE2 Wire2

void setup() {

  byte error, address;
  int nDevices;


  WIRE.begin();
  WIRE2.begin();

  Serial.begin(9600);
  while (!Serial)
    delay(10);

  Serial.println("\nScanning SDA/SCL for devices...");

  nDevices = 0;

  // The i2c_scanner uses the return value of
  // the Write.endTransmisstion to see if
  // a device acknowledged an address.
  for (address = 1; address < 127; address++) {

    //Scan for device at address
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    if (error == 0) {
      Serial.printf("Device found on SDA/SCL at 0x%02x\n", address);
      nDevices++;
    } else if (error == 4) {
      Serial.printf("Error on addr 0x%02x, errno=%d\n", address, error);
    }
  }
  printf("Found %d devices on SDA/SCL\n",nDevices);


  Serial.println("\nScanning SDA2/SCL2 for devices...");

  nDevices = 0;

  // The i2c_scanner uses the return value of
  // the Write.endTransmisstion to see if
  // a device acknowledged an address.
  for (address = 1; address < 127; address++) {

    //Scan for device at address
    WIRE2.beginTransmission(address);
    error = WIRE2.endTransmission();

    if (error == 0) {
      Serial.printf("Device found on SDA2/SCL2 at 0x%02x\n", address);
      nDevices++;
    } else if (error == 4) {
      Serial.printf("Error on SDA2/SCL2 addr 0x%02x, errno=%d\n", address, error);
    }
  }
  printf("Found %d devices on SDA2/SCL2\n", nDevices);
}


void loop() {
  delay(10);  //Nothing to do
}
