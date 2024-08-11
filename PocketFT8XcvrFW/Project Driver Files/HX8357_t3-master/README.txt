This is a version of Paul Stoffregen's highly-optimized ILI9341_t3 TFT library for the Adafruit HX8357D 3.5" TFT display (https://www.adafruit.com/product/2050). The extra speed is acheived by optimizing the SPI transactions and using hardware SPI only. This port runs about 2X as fast as the original Adafruit HX8357 display library on the Teensy 3.5, as shown below.

/* This Library (on Teensy 3.5 120Mhz, Fast optimization) */
HX8357 Test!
Display Power Mode: 0xCE
MADCTL Mode: 0xE0
Pixel Format: 0x2
Image Format: 0x0
Self Diagnostic: 0xE0
Benchmark                Time (microseconds)
Screen fill              449409
Text                     109286
Lines                    117080
Horiz/Vert Lines         36600
Rectangles (outline)     20353
Rectangles (filled)      1087322
Circles (filled)         147947
Circles (outline)        120168
Triangles (outline)      25393
Triangles (filled)       356333
Rounded rects (outline)  49729
Rounded rects (filled)   1183395
Done!

/* Old Library (on Teensy 3.5 120Mhz, Fast optimization) */
Display Power Mode: 0xCE
MADCTL Mode: 0xE0
Pixel Format: 0x2
Image Format: 0x0
Self Diagnostic: 0xE0
Benchmark                Time (microseconds)
Screen fill 		     N/A
Text                     407743
Lines                    1352147
Rectangles (outline)     63736
Rectangles (filled)      N/A
Circles (filled)         N/A
Circles (outline)        579367
Triangles (outline)      N/A
Triangles (filled)       1117238
Rounded rects (outline)  150835
Rounded rects (filled)   2447902
Done!

Discussion regarding the original optimized version for the ILI9341:

http://forum.pjrc.com/threads/26305-Highly-optimized-ILI9341-%28320x240-TFT-color-display%29-library

------------------------------------------

This is a library for the Adafruit HX8357 display products

This library works with the Adafruit 3.5" Touch Shield V2 (SPI)
  ----> https://www.adafruit.com/product/2050
 
Check out the links above for our tutorials and wiring diagrams.
These displays use SPI to communicate, 4 or 5 pins are required
to interface (RST is optional).

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistributionanm825