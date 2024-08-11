/*
NAME
  Test07 -- Si4735 Communication and Initialization

DESCRIPTION
  Verifies that Teensy can communicate with and initialize the Si4735 radio
  receiver.

EXERCISED
  + RESET pin
  + I2C connectivity with SI4735
  + Ability to initialize the SI4735 

NOTE
  + This test doesn't actually setup the SI4735 to receive anything.  It
  merely ensures Teensy can communicate with an alive/well SI4735 on the PCB.

REFERENCES
  1. https://github.com/Rotron/Pocket-FT8/blob/main/Project%20Manual_1.0.pdf
  2. https://oz1bxm.dk/notes/pocket-FT8.html

ATTRIBUTION
  KQ7B and Charlie [1]

*/



#include <SI4735.h>
#include "patch_full.h"  // SSB patch for whole SSBRX full download


#define AM_FUNCTION 1
#define RESET_PIN 20
#define USB 2


SI4735 si4735;

uint16_t currentFrequency;
long et1 = 0, et2 = 0;
const uint16_t size_content = sizeof ssb_patch_content;  // see ssb_patch_content in patch_full.h or patch_init.h;



void setup() {

  //Initialize the Arduino world and let console know we're starting
  Serial.begin(9600);
  Serial.println("Starting...");
  delay(100);

  // Gets and sets the Si47XX I2C bus address
  int16_t si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  if (si4735Addr == 0) {
    Serial.println("Si473X not found!");
    Serial.flush();
    while (1)
      ;
  } else {
    Serial.print("The Si473X I2C address is 0x");
    Serial.println(si4735Addr, HEX);
  }

  //Load the SSB patch
  delay(10);
  Serial.println("SSB patch is loading...");
  et1 = millis();
  loadSSB();
  et2 = millis();
  Serial.print("SSB patch was loaded in: ");
  Serial.print((et2 - et1));
  Serial.println("ms");
  delay(10);
  si4735.setTuneFrequencyAntennaCapacitor(1);  // Set antenna tuning capacitor for SW.
  delay(10);
  si4735.setSSB(18000, 18400, 18100, 1, USB);

  delay(10);
  currentFrequency = si4735.getFrequency();
  si4735.setVolume(50);
  Serial.print("CurrentFrequency = ");
  Serial.println(currentFrequency);
}

// the loop function runs over and over again forever
void loop() {
  delay(100);
}

//Installs SSB patch code in SI4735
void loadSSB()
{
  si4735.queryLibraryId(); // Is it really necessary here? I will check it.
  si4735.patchPowerUp();
  //si4735.setPowerUp(1, 1, 1, 0, 1, SI473X_ANALOG_DIGITAL_AUDIO);
  //si4735.radioPowerUp();
  delay(50);
  si4735.downloadPatch(ssb_patch_content, size_content);
  // Parameters
  // AUDIOBW - SSB Audio bandwidth; 0 = 1.2KHz (default); 1=2.2KHz; 2=3KHz; 3=4KHz; 4=500Hz; 5=1KHz;
  // SBCUTFLT SSB - side band cutoff filter for band passand low pass filter ( 0 or 1)
  // AVC_DIVIDER  - set 0 for SSB mode; set 3 for SYNC mode.
  // AVCEN - SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default).
  // SMUTESEL - SSB Soft-mute Based on RSSI or SNR (0 or 1).
  // DSP_AFCDIS - DSP AFC Disable or enable; 0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
  //si4735.setSSBConfig(bandwidthIdx, 1, 0, 1, 0, 1);
  si4735.setSSBConfig(2, 1, 0, 1, 0, 1);  //2 = 3 kc bandwidth
}

