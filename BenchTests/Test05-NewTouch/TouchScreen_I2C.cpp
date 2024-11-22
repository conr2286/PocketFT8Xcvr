/**
** SYNOPSIS
**  TouchScreen_I2C --- Adafruit's resistive touchscreen library revisited
**
** NOTES
**  This derivative of the resistive touchscreen library implements a non-blocking
**  getPoint() method.  When invoked repeatedly from loop(), getPoint() returns
**  immediately with z (pressure) set to zero when touch data is not yet available.
**  The design is constructed around a state machine driven by calls to getPoint().
**
** ATTRIBUtION
**  (C) ladyada / adafruit, "Code under MIT License"
**  Unknown, Implementation for the MCP342X A/D Converters
**  Jim Conrad, Non-blocking derivative of the above for a specified I2C bus
*/


//Ignore unused variable warnings in legacy code
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "Arduino.h"
#include "pins_arduino.h"
#include <Wire.h>
#include "MCP342x.h"

//Define which I2C bus hosts the MCP342x
#define MCP342X_WIRE Wire2

#include "DEBUG.h"

#ifdef __AVR
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif
#include "TouchScreen_I2C.h"

// increase or decrease the touchscreen oversampling. This is a little different
// than you make think: 1 is no oversampling, whatever data we get is
// immediately returned 2 is double-sampling and we only return valid data if
// both points are the same 3+ uses insert sort to get the median value. Adafruit
// found 2 is precise yet not too slow and recommends sticking with it.  However,
// the MCP342X re-implementation has been tested with 5 samples.
#define NUMSAMPLES 5

//Build an instance of the MCP342x A/D Converter
uint8_t address = 0x69;  //Original I2C address was 0x68 but MCP342X V1.01 chip reports 0x68
MCP342x adc = MCP342x(address);

//Build the resulting object returned by getPoint()
TSPoint result;      //Valid results are returned from here
TSPoint incomplete;  //Invalid results are returned from here

//TSPoint constructor.  The instance members are the X/Y touch coordinates and Z, the so-called
//touch "pressure" (careful examination of code discloses that term's loose usage;).
TSPoint::TSPoint(void) {
  x = y = z = 0;
}
/**
 * @brief Construct a new TSPoint::TSPoint object
 *
 * @param x0 The point's X value
 * @param y0 The point's Y value
 * @param z0 The point's Z value ("pressue")
 */
TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}
/**
 * @brief Check if the current point is **not** equivalent to another point
 *
 * @param p1 The other point being checked for equivalence
 * @return `true` : the two points are equivalent
 * `false`: the two points are **not** equivalent
 */
bool TSPoint::operator==(TSPoint p1) {
  return ((p1.x == x) && (p1.y == y) && (p1.z == z));
}
/**
 * @brief Check if the current point is **not** equivalent to another point
 *
 * @param p1 The other point being checked for equivalence

 * @return `true` :the two points are **not** equivalent
 * `false`: the two points are equivalent
 */
bool TSPoint::operator!=(TSPoint p1) {
  return ((p1.x != x) || (p1.y != y) || (p1.z != z));
}






/**
 * @brief Insertion sort newElement into existing[] of nElements
 *
 * @param newElement  This element will be inserted
 * @param existing  Existing array sorted ascending
 * @param nElements  Number of elements, potentially 0, in existingArray
 *
 * The array is maintained in ascending, sorted order.
 *
 * Warning:  There is no check for buffer overflow.
 *
**/
void insertionSort(int newElement, int existing[], int nElements) {

  int p, q;

  //Insert into an empty array?
  if (nElements == 0) {
    existing[0] = newElement;  //Insert at first index
    return;                    //That was easy!
  }

  //Locate p at the index of the smallest existing element whose value is >= newElement
  for (p = 0; p < nElements; p++) {
    if (existing[p] >= newElement) break;
  }

  //Are we appending newElement after the current end of existing[]?
  if (p == nElements) {
    existing[p] = newElement;  //Append newElement following last existing[] element
    return;                    //That was easy too!
  }

  //Move elements in existingArray to create a hole for newElement at existing[p]
  for (q = nElements; q >= p; q--) {
    existing[q] = existing[q - 1];  //Move an element of existingArray
  }

  //Insert newElement into the hole opened above
  existing[p] = newElement;  //p locates the hole

}  //insertionSort()





/**
 * @brief Measure the X, Y, and pressure and return a TSPoint with the
 * measurements
 *
 * @return TSPoint The derived X, Y, and Z pressure values
 *
 * getPoint() does not block.  If the touchscreen has not experienced
 * a touch event, getPoint() returns a TSPoint with z==0 to indicate
 * no touch event has occurred.
 *
 * Because raw resistive touchscreen coordinate sample values are eratic,
 * we collect multiple samples of both the X and Y coordinates and 
 * return their median values to reject the noise.
 *
 * We operate the ADC at 240 samples/second and collect a total of
 * 10 samples requiring a total of ~42 mS.  Because we are non-blocking,
 * much of that time is available for loop() to do other work.
 *
 * Somewhere in the history of this code, the coordinate system appears to have
 * been rotated in the getPoint() method's implementation.  Adafruit's
 * resistive touchscreen has four pins, X+ and X- for one axis and Y+ and Y-
 * for the other.  To read the X-coordinate of a touch point, we normally
 * ground one X pin, connect the other to Vcc, float both Y pins, and
 * sample the voltage on the Y.  Likewise, reading a Y-coordinate would
 * ground a Y pin, connect the other to Vcc, float both X pins, and sample
 * the voltage on the X.  You can observe this in Adafruit's github touchscreen
 * code, but will find the axis rotated below.  This may have arisen in the
 * original Pocket FT8 code though I'm not sure why as the coordinates could
 * be easily exchanged without the rotation.
 */
TSPoint TouchScreen::getPoint(void) {

  uint8_t err;
  MCP342x::Config status;
  long conversionResult;

  //The common result of most repeated calls to getPoint() is no touch data available
  result.x = result.y = result.z = 0;  //Pressure==0 implies no touch event data available

  //Repeated calls to getPoint() from loop() drive the state machine to configure the
  //resistive touchscreen pins, acquire/interpret X and Y coordinate data, ignore erratic
  //samples, and eventually return a valid result.
  switch (_state) {

    //The state machine is currently IDLE as we have not begun capturing touch samples
    case IDLE:
      _state = GETX;                       //Config state machine to acquire X-Coordinate samples
      setupToSampleXCoordinate();          //Sets the DIO ports to sample the touchscreen's X-Coordinates
      startConversion(MCP342x::channel2);  //Start ADC to acquire first sample of an X-Coordinate
      _nSamplesAcquired = 0;               //No samples acquired yet
      return incomplete;                   //Return the incomplete result
      break;

    //The state machine is currently acquiring X-coordinate samples (ADC is running)
    case GETX:

      //If the ADC is ready, retrieve an X-Coordinate sample
      if (conversionIsFinished()) conversionResult = getConversionResult();
      else return incomplete;  //Return the incomplete result as we're no where near finished

      //If a touch event occurred, then insert conversionResult into sorted samples[]
      if (conversionResult >= _minimumSample) {
        insertionSort(conversionResult, _samples, _nSamplesAcquired++);
      } else {
        _state = IDLE;          //This sample appears to be erratic noise
        _nSamplesAcquired = 0;  //Discard sample of an untouched screen
        return incomplete;      //Return an incomplete result
      }

      //Have we acquired sufficient samples to deduce the X-Coordinate's value?
      if (_nSamplesAcquired >= NUMSAMPLES) {
        result.x = _samples[NUMSAMPLES / 2];  //Record the median X-Coordinate sample value from sorted samples[]
        _state = GETY;                        //Config state machine to acquire Y-Coordinate samples
        setupToSampleYCoordinate();           //Set the DIO ports to sample resistive touchscreen Y-Coordinates
        startConversion(MCP342x::channel1);   //Start ADC to acquire first sample of a Y-Coordinate
        _nSamplesAcquired = 0;                //No Y-Coordinate samples acquired yet
      } else {
        startConversion(MCP342x::channel2);  //Start another X-Coordinate sample
      }
      return incomplete;  //Return an incomplete result
      break;


    //The state machine is currently acquiring Y-coordinate samples (ADC is running)
    case GETY:

      //If the ADC is ready, retrieve a Y-Coordinate sample
      if (conversionIsFinished()) conversionResult = getConversionResult();
      else return incomplete;  //Return an incomplete result

      //If a touch event occurred, then insert conversionResult into sorted samples[]
      if (conversionResult >= _minimumSample) {
        insertionSort(conversionResult, _samples, _nSamplesAcquired++);
      } else {
        _state = IDLE;          //Touch event was apparently too quick for us
        _nSamplesAcquired = 0;  //Discard sample of an untouched screen
        return incomplete;      //Return an incomplete result
      }

      //Have we acquired sufficient samples to deduce the Y-Coordinate's value?
      if (_nSamplesAcquired >= NUMSAMPLES) {
        result.y = _samples[NUMSAMPLES / 2];  //Record the median Y-Coordinate sample value from sorted samples[]
        _state = IDLE;                        //We have completed a touch event and return to IDLE
        result.z = result.x + result.y;       //This is how Adafruit calculated the so-called "pressure"
        return result;                        //Return a valid result
      } else {
        startConversion(MCP342x::channel1);  //Start another Y-Coordinate sample
      }
      return incomplete;
      break;


    //The state machine is damaged beyond all recognition:  punt
    default:
      _state = IDLE;  //Attempt repair by faking IDLE
      _nSamplesAcquired = 0;
      delay(100);     //Let the ADC finish whatever it's doing

  }  //switch(_state)
  return incomplete;

}  //getPoint()

/**
 * @brief Configure digital pins to acquire Y-Coordinate samples from resistive touchscreen
 *
 * Warning:  the axis are rotated (see description of getPoint())
**/
void TouchScreen::setupToSampleYCoordinate() {
  pinMode(_yp, INPUT);  //Float Y+
  pinMode(_ym, INPUT);  //Also Y-
  pinMode(_xp, OUTPUT);
  pinMode(_xm, OUTPUT);

  digitalWrite(_xm, HIGH);  //Set X- to Vcc
  digitalWrite(_xp, LOW);   //Ground X+
  delayMicroseconds(20);    //Fast ARM chips need to allow voltages to settle
}  //setupToSampleYCoordinate()


/**
 * @brief Configure digital pins to acquire X-Coordinate samples from resistive touchscreen
 *
 * Warning:  the axis are rotated (see description of getPoint())
 *
**/
void TouchScreen::setupToSampleXCoordinate() {
  pinMode(_xp, INPUT);  //Float X+
  pinMode(_xm, INPUT);  //Also X-
  pinMode(_yp, OUTPUT);
  pinMode(_ym, OUTPUT);

  digitalWrite(_ym, LOW);   //Ground Y-
  digitalWrite(_yp, HIGH);  //Set Y+ at Vcc
  delayMicroseconds(20);    // Fast ARM chips need to allow voltages to settle
}  //setupToSampleXCoordinate()

/**
 * @brief Start an ADC conversion on the specified channel
 *
 * @param channel The specified ADC channel
 *
 * The ADC is started for a single conversion but we don't block for nor acquire the resulting value.
 * We record the starting time later used to determine when the conversion should be finished without
 * blocking.
 *
 * Note that the conversion resolution is coordinated with conversionIsFinished()
**/
void TouchScreen::startConversion(MCP342x::Channel channel) {
  int err = adc.convert(channel, MCP342x::oneShot, MCP342x::resolution12, MCP342x::gain1);
  _adcStartTime = micros();
}  //startConversion()


/**
 * @brief Returns true if ADC conversion is complete, false if not
 *
 * We are using the expected duration to determine when the conversion should be complete.
 *
 * Note that conversion resulution is coordinated with startConversion() as the
 * actual conversion time is dependent upon the resolution.
 *
**/
bool TouchScreen::conversionIsFinished() {


  //Return true if we've waited long enough for conversion to complete
  if (micros() - _adcStartTime > _allowedDuration) return true;

  //Else return false
  return false;

}  //conversionIsFinished()


/**
 * @brief Retrieve conversion result from ADC
 *
**/
long TouchScreen::getConversionResult() {
  long adcResult;
  MCP342x::Config adcStatus;

  //Retrieve the result
  MCP342x::error_t err = adc.read(adcResult, adcStatus);
  ASSERT(!err);
  return adcResult;

}  //getConversionResult()



/**
 * @brief Constructor initializes a TouchsScreen object
 *
**/
TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym,
                         uint16_t rxplate = 0) {

  //Record the touchscreen's DIO pin numbers
  _yp = yp;
  _xm = xm;
  _ym = ym;
  _xp = xp;

  //Record the resistance of the X axis plate
  _rxplate = rxplate;

  //IDLE the getPoint() state machine
  _state = IDLE;

  //[Re-]Initialize this I2C bus
  MCP342X_WIRE.begin();

  //Reset all MCP342X chips on this I2C bus
  MCP342x::generalCallReset();
  delay(1);  // MC342x needs 300us to settle, wait 1ms

  //Record the duration of a 12-bit conversion
  _allowedDuration = adc.resolution12.getConversionTime() + 100;  //microseconds

  pressureThreshhold = 10;
  _minimumSample = pressureThreshhold;
}


// /**
//  * @brief Read the touch event's Z/pressure value
//  *
//  * @return int the Z measurement
//  */
// uint16_t TouchScreen::pressure(void) {
//   // Set X+ to ground
//   pinMode(_xp, OUTPUT);
//   digitalWrite(_xp, LOW);

//   // Set Y- to VCC
//   pinMode(_ym, OUTPUT);
//   digitalWrite(_ym, HIGH);

//   // Hi-Z X- and Y+
//   digitalWrite(_xm, LOW);
//   pinMode(_xm, INPUT);
//   digitalWrite(_yp, LOW);
//   pinMode(_yp, INPUT);

//   int z1 = analogRead(_xm);
//   int z2 = analogRead(_yp);

//   if (_rxplate != 0) {
//     // now read the x
//     float rtouch;
//     rtouch = z2;
//     rtouch /= z1;
//     rtouch -= 1;
//     rtouch *= readTouchX();
//     rtouch *= _rxplate;
//     rtouch /= 1024;

//     return rtouch;
//   } else {
//     return (1023 - (z2 - z1));
//   }
// }
