// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#ifndef _ADAFRUIT_TOUCHSCREEN_H_
#define _ADAFRUIT_TOUCHSCREEN_H_
#include <stdint.h>
#include "MCP342x.h"

//Define number of samples acquired from the ADC to determine each X and Y coordinate
#define NUMSAMPLES 5

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) || defined(TEENSYDUINO) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega4809__)
typedef volatile uint8_t RwReg;
#elif defined(ARDUINO_STM32_FEATHER)
typedef volatile uint32 RwReg;
#elif defined(NRF52_SERIES) || defined(ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_STM32)
typedef volatile uint32_t RwReg;
#else
typedef volatile uint32_t RwReg;
#endif



//Object that encapsulates the X,Y, and Z/pressure measurements for a touch event
class TSPoint {
public:
  TSPoint(void);
  TSPoint(int16_t x, int16_t y, int16_t z);

  bool operator==(TSPoint);
  bool operator!=(TSPoint);

  int16_t x,  //X-Coordinate
    y,        //Y-Coordinate
    z;        //Z-Coordinate (nominally the "pressure")
};


//Object that controls and keeps state for a touch screen
class TouchScreen {
public:
  /**
   * @brief Construct a new Touch Screen object
   *
   * @param xp X+ pin. Must be an analog pin
   * @param yp Y+ pin. Must be an analog pin
   * @param xm X- pin. Can be a digital pin
   * @param ym Y- pin. Can be a digital pin
   * @param rx The resistance in ohms between X+ and X- to calibrate pressure
   * sensing
   */
  TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx);

  uint16_t pressure(void);
  TSPoint getPoint();
  int16_t pressureThreshhold;  //Pressure threshold for `isTouching`

private:
  enum TouchState { IDLE,
                    GETX,
                    GETY };    //getPoint() state variable data type
  uint8_t _yp, _ym, _xm, _xp;  //The resistive touchscreen DIO pins
  uint16_t _rxplate;           //The resistance (Ohms) of the X-Axis touch plate
  enum TouchState _state;      //getPoint() state variable
  uint32_t _adcStartTime;      //Microsecond when ADC started a conversion
  uint32_t _allowedDuration;   //Microseconds allowed to complete a conversion
  int _samples[NUMSAMPLES];    //ADC samples acquired to determine coordinate value
  int _nSamplesAcquired;       //Number of samples acquired for a coordinate
  int _minimumSample;          //Smallest plausible ADC sample

  void setupToSampleXCoordinate();
  void setupToSampleYCoordinate();
  void startConversion(MCP342x::Channel);
  bool conversionIsFinished();
  long getConversionResult();
};

#endif