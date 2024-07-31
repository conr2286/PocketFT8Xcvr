# PocketFT8Xcvr41
An implementation of Charles Hill's Palm-Sized Pocket FT8 Transceiver updated for Teensy 4.1

##Attribution
* Copyright (C) 2021, Charles Hill
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba: https://github.com/kgoba/ft8_lib
* Enhancements by Lars Petersen, OZ1BXM
* PC Board by Jim Conrad, KQ7B

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Please use this software at your own risk

##Features

* FT8 Message Transmit and Receive

* Small Size, 3.5” X 2.75” X 1.125” (in Charles' original implementation)

* 100 mW power output @ 50 ohm load

* 1 uVolt Receiver Sensitivity

* Single 5 volt power input, battery or wall wart

* Silicon Labs Technology, Si4735 SSB Receiver & Si5351 Transmit FSK Clock, GVA-84+ MMIC

* SD Card Contact Logging

* 320 X 480 Resistive  Color Touch Screen

##DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.

In order to optimize both program storage and processing speed requirements so that the Teensy 3.6 is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

##Motivation
I (KQ7B) liked Charles Hill's concept of a portable FT8 transceiver.  Rather than starting from a clean slate, I chose to implement a Pocket FT8 with minimal revisions, largely confined to those necessary for the Teensy 4.1 (as the 3.6 is no longer available), and to fab a "real" PC Board.  The idea is to use this as a foundation for incremental improvements after evaluating what changes are merited.

An alternative approach might be to construct an FT8 radio shield for a Raspberry 5.

##Investigations
* PC Board artwork and fabrication
* Evaluate on-the-air performance
** Evaluate power consumption
** Evaluate performance of the 100 mW final
** SI4735 receiver
* Consider use of a different DAC with a 3.3V reference

##Status
August, 2024:  Schematic capture is complete but not yet reviewed.  The original firmware sketch is compiling for Teensy 4.1 but is not yet tested.
