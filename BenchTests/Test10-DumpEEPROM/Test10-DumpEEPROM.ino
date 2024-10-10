/*
NAME
  Test10-DumpEEPROM

DESCRIPTION
  Dumps contents of EEPROM

EXERCISED

NOTE

REFERENCES



ATTRIBUTION
  KQ7B 


*/

#include <EEPROM.h>


int EEPROMReadInt(int);


void setup() {

  Serial.begin(9600);
  while (!Serial)
    delay(10);
  Serial.println("Starting...\n");

  for (int adr = 0; adr <= 10; adr++) {
    int x = EEPROMReadInt(adr);
    Serial.printf("EEPROM %d: %d\n", adr, x);
  }
}


  void loop() {

    delay(5000);
  }


  int EEPROMReadInt(int address) {
    uint16_t byte1 = EEPROM.read(address);
    uint16_t byte2 = EEPROM.read(address + 1);
    uint16_t internal_value = (byte1 << 8 | byte2);

    return (int)internal_value - 32768;
  }
