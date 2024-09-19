# PocketFT8Xcvr41
An implementation of Charles Hill's Palm-Sized Pocket FT8 Transceiver updated for Teensy 4.1 with SMD components on a 4-Layer board

# Versions
* 1.00 Original submission to PCBWay
* 1.01 Resubmit to PCBWay using SI5351C-B-GM for unobtanium SI5351C-B-GM1 package

# Attribution and License
* Copyright (C) 2021, Charles Hill
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba: https://github.com/kgoba/ft8_lib
* Enhancements by Lars Petersen, OZ1BXM
* PC Board by Jim Conrad, KQ7B (retired engineer)

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Please use this software at your own risk

# Features
* FT8 Message Transmit and Receive
* Small Size, 4.0 X 2.8"
* TCXO
* 100 mW power output @ 50 ohm load (well...)
* 1 uVolt Receiver Sensitivity
* Powered by a single USB source (e.g. portable consumer power bank)
* Si4735 SSB Receiver & Si5351 Transmit FSK Clock, GVA-84+ MMIC
* SD Card Contact Logging
* Adafruit 320 X 480 Resistive Color Touch Screen

# DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.

In order to optimize both program storage and processing speed requirements so that the Teensy 3.6 is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.  For Teensy 4.1, the revised sample rate required replacement of the core AudioStream.h file with the AudioStream_32k.h file (from PocketFT8FW sketch folder --- see Issue #12).

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

# Motivation
Charles Hill's concept of a portable FT8 transceiver is ideal for POTA/SOTA in the U.S. Idaho mountains (my home).  The entire transceiver fits in a compact enclosure, the power demand is acceptable, and it can operate from a USB power block familiar to backpackers.  Rather than start with a clean slate, I chose to start with a compact implementation of Pocket FT8 with minimal revision, largely confined to that necessary for the Teensy 4.1 (as the 3.6 is no longer available) and implemented on a single 4-layer board.  The idea is to reproduce it, identify and resolve its shortcomings in subsequent revisions, and keep it highly portable.

An alternative approach, previously investigated as YASDR, might be to construct an FT8 radio hat for a Raspberry 5 supporting the comprehensive wsjtx/etc natively.  This archived approach offers considerable flexibility but with substantially higher power requirements (I'm getting too old to pack heavy batteries in the mountains;).

# Manifest
* BenchTests:  Arduino sketches for incremental tests of the hardware
* Bibliography:  "The shoulders of giants"
* Investigations:  Code and simulations exploring future technologies
* PocketFT8XcvrFW:  Arduino 2.0 firmware sketch for the Pocket FT8 transceiver
* PocketFT8XcvrHW:  KiCad 8 files for the PCB

# ToDo
* Bench testing/rework to replicate Charlie's Pocket FT8 functionality
* Consider a harmonic trap in the low-pass filter
* Also consider a Chebyshev low-pass filter
* Evaluate on-the-air performance
* Support a GPS module for time/date/location
* Investigate 20m and 40m versions for POTA/SOTA operation
* Investigate higher power (e.g. 1..2 watts) Class-D finals with BS170s operating at 5V
* Evaluate the SI4735 receiver's simplicity vs benefits of a Tayloe Detector --- is it really worth it?
* USB-C power

# Versions
* V1.00 PCB gerbers and pos files submitted to PCBWay.
* V1.01 Switched from the unobtanium QFN16 to the QFN

