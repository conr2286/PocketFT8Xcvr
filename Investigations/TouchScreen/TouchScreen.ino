/*
NAME
  TouchScreen Explorer

DESCRIPTION
  Investigates properties/behaviors of the Adafruit 320x480 resistive touchscreen,
  especially behaviors helpful to understanding erratic (bouncing, dropout, noise...)
  coordinate readings.

EXERCISED
  + X plate resistance
  + Idle (no touch) noise on X plate
  + Touch event noise (dropouts, erratic coordinates, etc)
  + Duration of a touch event

NOTE
  + From the Pocket FT8 schematic, we know the Teensy 4.1 YP and XM pins
  connect to the touchpad Y+ and X- pins through 510 ohm resistors.  This
  hardware dependency is used in some calculations below.
  + Pocket FT8 operates the MCP342x ADC with 12 bits at 240 samples/second.  The 
  range of possible readings from the ADC should be 0..2047.
  + We consider a touch event to begin when the ADC returns a value greater
  than or equal to the so-called MINPRESSURE.
  + We note a touch event begins, and continue recording samples into a FIFO
  ring buffer for 1.9 seconds

REFERENCES

ATTRIBUTION
  KQ7B, MIT License

*/



#include <Wire.h>
#include "HX8357_t3n.h"
#include "MCP342x.h"

// These are the four touchscreen pins used by Pocket FT8 hardware
#define YP 38  // must be an analog pin, use "An" notation!
#define XM 37  // must be an analog pin, use "An" notation!
#define YM 36  // can be a digital pin
#define XP 39  // can be a digital pin

// Define the value of the resistors connecting the Teensy 4.1 YP and XM pins
// to the touchpad's Y+ and X- pins.
#define RYP 510  //Ohms
#define RXM 510  //Ohms

//Define Vcc
#define VCC 3.6  //Volts

// // Touchpad calibrarion as investigated on V1.01 hardware
// #define TS_MINX 123
// #define TS_MINY 104
// #define TS_MAXX 1715
// #define TS_MAXY 1130

//Other touchpad constants replicated from button.cpp in PocketFT8XcvrFW
#define MINPRESSURE 120
#define PENRADIUS 3

//Number of samples taken during one phase of the investigation
#define NSAMPLES 240  //About one seconds worth of samples

//Define the maximum possible ADC sampled voltage
#define maxADCreading 2.047  //"She can't do no more, Jim"

//Build the display object using pin numbers from Charlie's Pocket FT8 code
HX8357_t3n tft = HX8357_t3n(10, 9, 8, 11, 13, 12);  //Teensy 4.1 moved SCK to dig pin 13

//Build an instance of the MCP342x A/D Converter
uint8_t address = 0x69;  //Original I2C address was 0x68 but MCP342X V1.01 chip reports 0x68
MCP342x adc = MCP342x(address);

//Build an array for storing ADC samples
float samples[NSAMPLES];  //About one seconds

/*
** RingBfr -- A FIFO ring buffer of long integers
**
** Someday:  rewrite this class as a template for arbitrary types and buffer sizes
*/
class RingBfr {

public:

  /**
 * RingBfr constructor
**/
  RingBfr() {
    nextIn = nextOut = count = 0;
  }  //RingBfr()


  /**
 * @brief Reset a ring buffer to its initial conditions
 *
**/
  void reset() {
    nextIn = nextOut = count = 0;
  }  //reset()


  /**
 * Increment a ring buffer index, wrapping around to 0 if necessary
 *
 * @param  index The value to be advanced
 *
 * @return The advanced index
 *
 * The returned index value will always lie within 0..(bfrSize-1).  Advancement
 * is independent of the count of existing entries in the buffer.
 *
**/
  int advance(int index) {
    index++;                          //Increment a buffer index
    if (index >= bfrSize) index = 0;  //Wrap around to beginning?
    return index;
  }  //advance()


  /**
 * Retard (backup) a ring buffer index, wrapping around if necessary
 *
 * @param index The value to be retarded
 *
 * @return The retarded index value
 *
 * The returned value will always lie within 0..(bfrSize-1) even if the index parameter
 * lies outside that range.
 *
**/
  int retard(int index) {
    if (index >= bfrSize) index = bfrSize;  //Repair invalid index
    index--;                                //Backup the index
    if (index < 0) index = bfrSize - 1;     //Deal with wraparound
    return index;
  }  //retard()


  /**
 * Getter for the nextIn member
 *
 * @return Value of the nextIn member
 *
**/
  int getNextIn() {
    return nextIn;
  }


  /**
 * Getter for the bfrSize member
 *
 * @return The maximum number entries that the buffer can hold
 *
**/
  int getBfrSize() {
    return bfrSize;
  }


  /**
 * Add a value to the end of a FIFO ring buffer
 *
 * @param  newValue The value to be added
 * 
 * @return Updated count of entries in buffer or -1 if the buffer was found already full
 *
 * Adds newValue to the end of the ring buffer, advances nextIn, and increments the count.
 * Will not overwrite an existing buffer entry.
 *
**/
  int addNext(long newValue) {
    if (count >= bfrSize) return -1;  //Check for full buffer
    buffer[nextIn] = newValue;        //Add newValue to end of the buffer
    nextIn = advance(nextIn);         //Advance the nextIn buffer index
    count++;                          //Increment count of buffered values
    return count;                     //Number of buffered values
  }                                   //addNext()


  /**
 * Add a value to the end of a ring buffer, overwriting old values when full
 *
 * @param  newValue The value to be added
 * 
 * @return Updated count of entries in buffer (0..bfrSize)
 *
 * Similar to addNext() but overwrites the oldest entry when buffer overflows.
 * Maintains nextOut indexing the oldest, non-overwritten entry so that buffer
 * continues to work as a FIFO in the sense that the oldest buffered value
 * will be the first out.
 *
**/
  int overwrite(long newValue) {
    buffer[nextIn] = newValue;  //Add newValue to end of the buffer
    nextIn = advance(nextIn);   //Advance the nextIn buffer index
    count++;                    //Increment count of entries
    if (count > bfrSize) {      //Did buffer overflow?
      nextOut = nextIn;         //Yes, move nextOut to index of oldest entry
      count = bfrSize;          //Maintain buffer at its capacity
    }
    return count;  //1..bfrSize
  }


  /**
 * Retrieve the next value from a ring buffer
 *
 * @param &value Retrieved value
 *
 * @return #elements remaining in buffer or -1 if buffer was already empty
 *
 * Retrieves the next entry, advances the nextOut index, and decrements the count
 *
**/
  int getNext(long &value) {

    if (count == 0) return -1;   //Check for empty buffer
    value = buffer[nextOut];     //Retrieve the next value from buffer
    nextOut = advance(nextOut);  //Advance the nextOut index
    count--;                     //Decrement count of remaining entries
    return count;                //Number of buffered entries remaining
  }                              //getNext()


  /**
 * Getter for the count of buffered entries
 *
**/
  int getCount() {
    return count;
  }


  /**
 * Set nextOut to the specified value
 *
 * @param index The specified new value
 *
 * @return Updated nextOut if success, else -1 if index lies outside the buffer's range
 *
 * The index must lie in the range, 0..(bfrSize-1)
 *
**/
  int setNextOut(unsigned index) {
    if (index >= bfrSize) return -1;  //Check for invalid index
    nextOut = index;                  //It's valid
    return nextOut;                   //Return valid index
  }


//Private members
private:
  int nextIn;                      //Index of where next added value will be placed
  int nextOut;                     //Index of where next removed value will be found
  int count;                       //Number of values in the buffer
  static const int bfrSize = 480;  //Maximum number of buffer entries
  long buffer[bfrSize];            //Container for the actual buffered data entries

};  //class RingBfr





//Build the ring buffer object
RingBfr bfr;

//Initialization and one-time touchscreen discovery
void setup() {

  MCP342x::Config status;
  long value;
  float sum, v2, rXplate;

  //Initialize the Arduino world and let console know we're starting
  Serial.begin(9600);
  delay(100);
  Serial.println("Starting...");

  //Initialize the display
  tft.begin(HX8357D);
  tft.fillScreen(HX8357_BLACK);
  tft.setTextColor(HX8357_YELLOW);
  tft.setRotation(3);  //PocketFT8FW uses 3
  tft.setTextSize(2);
  delay(100);


  //Setup to measure the X-Plate resistance
  Serial.println("Measuring X-Plate resistance");
  pinMode(XM, OUTPUT);
  pinMode(XP, OUTPUT);
  pinMode(YM, INPUT);      //Float Y plate connections
  pinMode(YP, INPUT);      //Float Y plate connections
  digitalWrite(XM, HIGH);  //Connect XM to Vcc through RXM
  digitalWrite(XP, LOW);   //Ground XP side of the plate

  //Acquire ~1 second of samples of the voltage appearing on the touchpad's X- pin for the resistance calculation
  int i;
  for (i = 0; i < NSAMPLES; i++) {
    adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution12, MCP342x::gain1, 100000, value, status);
    samples[i] = value;
  }

  //Calculate the average value of the voltage on touchpad's X- pin
  sum = 0.0;
  for (i = 0; i < NSAMPLES; i++) sum += samples[i];
  v2 = (sum / NSAMPLES) * maxADCreading;  //Average value of voltage appearing on touchpad's X- pin

  //Now we can calculate the resistance of the X-Plate
  rXplate = (RXM * v2) / (VCC - v2);
  printf("X-Plate resistance = %f Ohms\n", rXplate);

  //Calculate the stdev (noise) of the X-Plate voltage measurements
  float sumDevSquared = 0.0;
  for (i = 0; i < NSAMPLES; i++) {
    float deviation = samples[i] - v2;
    sumDevSquared += (deviation * deviation);
  }
  float stdev = sqrt(sumDevSquared / NSAMPLES);
  printf("X-Plate stdev (noise) = %f volts\n", stdev);

  //Setup to instrument a touch event
  printf("Tap (touch) the screen");
  bfr.reset();  //Reset the ring buffer

}  //setup()





int eventStart;    //Records index of where touch event begins in ring buffer
int plotStart;     //Records index of where plot begins in ring buffer
int nSamples = 0;  //Number of samples acquired during touch event

// loop() runs over and over again forever, recording touchscreen samples in a ring buffer.
// When a touch event begins, loop() records the starting ring buffer index and 480
// (2 seconds) more samples from the touchscreen, and finally plots the touchscreen voltage
// samples to illustrate noise/dropouts/etc.
void loop() {

  long value;
  MCP342x::Config status;
  bool touchEventInProgress = false;

  //Get the next value (0..2047) from the ADC
  adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot, MCP342x::resolution12, MCP342x::gain1, 100000, value, status);

  //Add the ADC value to the ring buffer, overwriting the oldest entry when the buffer eventually fills
  bfr.overwrite(value);  //Always record the sampled value in the ring buffer

  //Did we receive the first valid sample of a new touch event?
  if (touchEventInProgress == false && value >= MINPRESSURE) {

    //Yes, a touch event has begun
    eventStart = bfr.retard(bfr.getNextIn());  //Record index of where touch event starts in ring buffer
    touchEventInProgress = true;               //State variable indicating touch event has begun

    //Start the plot either 24 or getCount() entries behind the eventStart index
    plotStart = eventStart;                 //Index of where touch event begins
    int nRetard = min(bfr.getCount(), 24);  //Number of entries to retard plotStart (accounting for bfr.count)
    for (int n = 1; n <= nRetard; n++)      //Loop retards plotStart 0.1 second of samples
      plotStart = bfr.retard(plotStart);    //Back the index one position
    bfr.setNextOut(plotStart);              //Set bfr.nextOut to index of where plot will begin
  }

  //Should we tally this sample as part of a touch event?
  if (touchEventInProgress)
    nSamples++;  //Yes, this sample is part of a touch event

  //Was this the final sample in this touch event?
  if (nSamples == (480 - 24)) {  //We want to acquire 1.9 seconds of data after the event starts

    //Yes, stop acquiring samples.  We have 0.1 second of data prior to event and 1.9 during the event
    touchEventInProgress = false;  //Setup for next touch event
    nSamples = 0;                  //Reset tally for next event

    //Plot the acquired raw ADC voltage samples of the touch event
    for (int i = 0; i < bfr.getCount(); i++) {
      bfr.getNext(value);                              //Retrieve value from FIFO ring buffer
      value = 319.0 * float(value) / 2047.0;           //Scale value to fit on the 320x480 display's Y axis
      tft.drawPixel(i, (int16_t)value, HX8357_WHITE);  //Plot this point
    }
  }

}  //loop()
