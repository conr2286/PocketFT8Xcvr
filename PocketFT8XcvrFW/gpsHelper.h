#include <TinyGPSPlus.h>
#include "pins.h"

//Define the default timeout (seconds)
#define GPS_TIMEOUT 60      

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
  double flat;
  double flng;

  //Publicly accessible methods
  bool obtainGPSfix();  //Returns true and initializes member vars if it obtains a GPS fix

  /**
  ** GPShelper Constructor
  **
  ** @param baudRate GPS serial connection baud rate
  **
  **/
  GPShelper(unsigned baudRate) {
    timeoutSeconds = GPS_TIMEOUT;
    SerialGPS.begin(baudRate);
  }

  //Our private implementation variables
private:
  unsigned timeoutSeconds;  //Maximum number of seconds we  wait for GPS to obtain a fix
  TinyGPSPlus gps;          //Our chosen interface to the GPS
};