#include <TinyGPSPlus.h>
#include "pins.h"

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
  bool obtainGPSfix(unsigned timeoutSeconds);  //Returns true and assigns member var values if it obtains a GPS fix

  /**
  ** GPShelper Constructor
  **
  ** @param baudRate GPS serial connection baud rate
  **
  **/
  GPShelper(unsigned baudRate) {
    SerialGPS.begin(baudRate);
  }

  //Our private implementation variables
private:
  TinyGPSPlus gps;          //Our chosen interface to the GPS


};