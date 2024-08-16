# PocketFT8Xcvr41
An implementation of Charles Hill's Palm-Sized Pocket FT8 Transceiver updated for Teensy 4.1 with SMD components on a 4-Layer board

# Version 1.00 (Under development)

# Attribution and License
* Copyright (C) 2021, Charles Hill
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba: https://github.com/kgoba/ft8_lib
* Enhancements by Lars Petersen, OZ1BXM
* PC Board by Jim Conrad, KQ7B (retired engineer)

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Please use this software at your own risk

# V1.00 Features
* FT8 Message Transmit and Receive
* Small Size, 4.0 X 2.8"
* TCXO
* 100 mW power output @ 50 ohm load
* 1 uVolt Receiver Sensitivity
* Powered by a single USB-A source (e.g. portable consumer power bank)
* Si4735 SSB Receiver & Si5351 Transmit FSK Clock, GVA-84+ MMIC
* SD Card Contact Logging
* Adafruit 320 X 480 Resistive Color Touch Screen

# DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.

In order to optimize both program storage and processing speed requirements so that the Teensy 3.6 is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

# Motivation
Charles Hill's concept of a portable FT8 transceiver is perfect for SOTA and the U.S. Idaho mountains (my home).  Rather than starting with a clean slate, I chose to start with a compact implementation of Pocket FT8 with minimal revision, largely confined to that necessary for the Teensy 4.1 (as the 3.6 is no longer available) and a 4-layer board.  The idea is to use Pocket FT8 as a foundation for incremental improvements by evaluating what changes are truly merited.

An alternative approach, previously investigated as YASDR, might be to construct an FT8 radio hat for a Raspberry 5 supporting the comprehensive wsjtx/etc natively.  This archived approach offers considerable flexibility but with substantially higher power requirements (I'm getting too old to pack heavy batteries in the mountains;).

# Manifest
* BenchTests:  Arduino sketches for incremental tests of the hardware
* Bibliography:  "The shoulders of giants"
* Investigations:  Code and simulations exploring supporting technologies
* KiCad:  Just a folder where KiCad likes to place the BOM output file
* PocketFT8XcvrFW:  Arduino firmware sketch for the Pocket FT8 transceiver
* PocketFT8XcvrHW:  KiCad 8 project files for the PCB

# ToDo
* Bench testing/rework to replicate Charlie's Pocket FT8 functionality
* Add harmonic trap to the low-pass filter
* Evaluate on-the-air performance
* Add code to support a GPS module for time/date/location
* Evalutate 20m and 40m versions for SOTA operation
* Consider higher power (e.g. 1..2 watts) Class-D finals with BS170s
* Evaluate SI4735 receiver simplicity vs Tayloe Detector performance
* Find a DAC with a 3.3V reference for full-screen touch
* USB-C power

# Status and Next Steps
* August 15, 2024:  PCB gerbers and pos files submitted to PCBWay for fabrication and assembly of the V1.00 lab prototype.  Developed several bench tests for evaluating assembled boards.  Completed initial work to port Charlie's code to this hw implementation (e.g. use of an SI5351C driven by a TCXO, etc).
