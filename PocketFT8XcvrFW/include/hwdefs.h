#pragma once
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
**  V2.X and V3.X boards use an MCP342X external 12-bit ADC to read the touchscreen signals.
**
**  V4.X boards eliminate the MCP342X and read the touchscreen through ADC1 A16 and A17 while
**  the Teensy 4.1 audio pipeline uses ADC2 and A2.
**
** HISTORY
**  10/08/2024 Teensy 4.1 pin assignments by KQ7B
**  03/14/2026 Version 4.X boards move the touchscreen to Teensy 4.1's A16 and A17 pins
**
**/

// Define the hardware version number (whole integral values only)
// #define HW_VERSION 2        //Version 2.X boards (working lab prototype)
// #define HW_VERSION 3  // Version 3.X boards (never mfg'd due to tariffs)
#define HW_VERSION 4  // Version 4.X boards (internal touchscreen ADC)

// The Adafruit 480x320 display pins
#define PIN_CS 10
#define PIN_DC 9
#define PIN_RST 8
#define PIN_MOSI 11
#define PIN_DCLK 13  // Teensy 4.1
#define PIN_MISO 12

// I2C pins
#define PIN_SCL 19   // Wire
#define PIN_SDA 18   // Wire
#define PIN_SCL2 24  // Wire2
#define PIN_SDA2 25  // Wire2

// I2C bus definitions for the Wire objects
#define WIRE_RCV Wire   // The SI4735's private bus
#define WIRE_ETC Wire2  // Everything (SI5351, MCP342x) else

// Serial bus definitions for optional GPS
#define SerialGPS Serial1

// SI4735 pins
#define PIN_RESET 20  // Resets the Si4735
#define PIN_AF A2     // Si4735 received audio connected to A2 via Pin 16

// Transmit/Receive pins
#define PIN_PTT 14  // Teensy 4.1
#define PIN_RCV 15  // FW implementation of ~PTT (i.e. RECV) in V2.0 boards

// Adafruit resistive touchscreen pins (External ADC HW option)
#define PIN_YP 38   // must be an analog output pin
#define PIN_XM 37   // must be an analog output pin
#define PIN_YDM 36  // Any digital pin
#define PIN_XDP 39  // Any digital pin

#if HW_VERSION >= 4
// Version 4.X hardware internal Teensy 4.1 ADC pins (Note:  Versions 2.X and 3.x
// use an external ADC read via I2C rather than an internal ADC) for reading the
// floating touchscreen pins (Schematic signals XD- and YD+).  Note:  You can
// patch a V3.X PCB to make these signal connections for the internal ADC
#define PIN_XDM A16  // Y-Coord analog input signal sampled from display's floating XM pin (A16)
#define PIN_YDP A17  // X-Coord analog input signal sampled from display's floating YP pin (A17)
#endif

// GPS pins (Teensy 4.1 Serial1)
#define PIN_GPSTX 0  // TX data from GPS
#define PIN_GPSRX 1  // RX data from GPS
#define PIN_PPS 2    // PPS pulse

// Auxilliary pins for future expansion (Require V3.00 PCB)
#define PIN_AN13 27  // A13
#define PIN_D27 27   // D27
#define PIN_D28 28   // D28
#define PIN_D29 29   // D29
#define PIN_D30 30   // D30
#define PIN_D31 31   // D31

// The ADC calibration data for V4.X hardware is using a 3.3V reference to Vcc
// And analog.c is using a 10-bit resolution rather than 12
#if HW_VERSION >= 4
#define TS_MINX (137 >> 2)
#define TS_MINY (126 >> 2)
#define TS_MAXX (2182 >> 2)
#define TS_MAXY (1392 >> 2)
#define MINPRESSURE (120 >> 2)  // I don't think the pressure stuff is working as expected
#define PENRADIUS 3
#else
// This is calibration data for the MC342x ADC to the screen coordinates
// using 510 Ohm resistors to reduce the sensors' driven voltage to Y+ and X-
// where the ADC reference voltage is 2.048 volts
#define TS_MINX 123
#define TS_MINY 104
#define TS_MAXX 1715
#define TS_MAXY 1130
#define MINPRESSURE 120  // I don't think the pressure stuff is working as expected
#define PENRADIUS 3
#endif
