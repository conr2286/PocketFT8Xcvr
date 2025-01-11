#include <Adafruit_GPS.h>

#pragma once
#include "pins.h"


// what's the name of the hardware serial port?
#define GPSSerial Serial1




class GPShelper {

public:

  //This public member records if we have ever obtained a successful fix with the data below
  bool validFix = false;

  //These are the publicly accessible member variables whose values are valid only if we have obtained a fix
  unsigned year;
  unsigned month;
  unsigned day;
  unsigned hour;
  unsigned minute;
  unsigned second;
  unsigned milliseconds;
  double flat;
  double flng;

  //Publicly accessible methods
  GPShelper(unsigned gpsBaudRate);
  bool obtainGPSfix(unsigned timeoutSeconds,void (*gpsAcquiringFix)());  //Returns true and assigns member var values if it obtains a GPS fix


 
  //Our private implementation variables
private:


};