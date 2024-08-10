# PocketFT8Xcvr41
An implementation of Charles Hill's Palm-Sized Pocket FT8 Transceiver updated for Teensy 4.1 with SMD peripherals

# Version 1.00 (Under development)

# Attribution and License
* Copyright (C) 2021, Charles Hill
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba: https://github.com/kgoba/ft8_lib
* Enhancements by Lars Petersen, OZ1BXM
* PC Board by Jim Conrad, KQ7B

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Please use this software at your own risk

# Features
* FT8 Message Transmit and Receive
* Small Size, 4.0 X 2.8"
* 100 mW power output @ 50 ohm load
* 1 uVolt Receiver Sensitivity
* Powered by a USB-A source (e.g. portable power bank)
* Si4735 SSB Receiver & Si5351 Transmit FSK Clock, GVA-84+ MMIC
* SD Card Contact Logging
* 320 X 480 Resistive  Color Touch Screen

# DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.

In order to optimize both program storage and processing speed requirements so that the Teensy 3.6 is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

# Motivation
I (KQ7B) liked Charles Hill's concept of a portable FT8 transceiver for the Idaho mountains (my home).  Rather than starting with a clean slate, I chose to investigate a Pocket FT8 with minimal revisions, largely confined to those necessary for the Teensy 4.1 (as the 3.6 is no longer available), and to fab a "real" PC Board.  The idea is to use this as a foundation for incremental improvements after evaluating what changes are truly merited.

An alternative approach, previously investigated as YASDR, might be to construct an FT8 radio hat for a Raspberry 5 supporting the comprehensive wsjtx/etc natively but with substantially higher power requirements for the RP5.

# Investigations
* PC Board artwork and fabrication
* Bench testing/rework
* Add harmonic trap to the low-pass filter
* Evaluate on-the-air performance
* Add code to support the GPS module for time/date/location
* Evalutate 20m version
* Consider alternatives for higher power (e.g. 1..2 watts) Class-D operation
* SI4735 receiver performance complexity vs Tayloe Detector approach
* Find a DAC with a 3.3V reference for full-screen touch

# Status and Next Steps
* August, 2024:  PCB design is complete but not fully reviewed by PCB fab.  The original firmware sketch is compiling for Teensy 4.1 but is not yet tested.  Next Step:  Develop tests to exercise the Version 1.00 hardware.
