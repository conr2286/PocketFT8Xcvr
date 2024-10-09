/*
**
** NAME
**  pins.h --- Define usage of all GPIO pins in this one place
**
** NOTES
**  If you change processors, displays, etc, you may need to reevaluate the GPIO pin assignments
**
**  The V2.0 boards control the RCV and XMT signals using firmware thru PIN_RCV and PIN_PTT.  
**  This firmware implementation is compatible with both V1.X and V2.0 boards.
**
** HISTORY
**  10/08/2024 Teensy 4.1 pin assignments by KQ7B
**
**/

//The Adafruit 480x320 display pins
#define PIN_CS    10
#define PIN_DC    9
#define PIN_RST   8
#define PIN_MOSI  11
#define PIN_DCLK  13          //Teensy 4.1
#define PIN_MISO  12

//I2C pins
#define PIN_SCL   19
#define PIN_SDA   18

//SI4735 pins
#define PIN_RESET 20

//Transmit/Receive pins
#define PIN_PTT   14            //Teensy 4.1
#define PIN_RCV   15            //FW implementation of ~PTT (i.e. RECV) in V2.0 boards

//Adafruit resistive touchscreen pins
#define PIN_YP 38               // must be an analog pin, use "An" notation!
#define PIN_XM 37               // must be an analog pin, use "An" notation!
#define PIN_YM 36               // can be a digital pin
#define PIN_XP 39               // can be a digital pin

//GPS pins
#define PIN_GPSTX 0             //TX data from GPS
#define PIN_GPSRX 1             //RX data from GPS

