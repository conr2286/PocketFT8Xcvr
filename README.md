# Pocket FT8 Revisited
An SMD derivative of Charles Hill's Palm-Sized Pocket FT8 Transceiver

# Attribution
* Copyright (C) 2021, Charles Hill (W5BAA), https://github.com/Rotron/Pocket-FT8 
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba (YL3JG): https://github.com/kgoba/ft8_lib
* SN74ACT244 PA by Barb (WB2CBA), https://github.com/WB2CBA/DX-FT8-FT8-MULTIBAND-TABLET-TRANSCEIVER 
* PC Board and enhancements by Jim Conrad (KQ7B), https://github.com/conr2286/PocketFT8Xcvr 
* Adafruit, PJRC and other Arduino libraries

# MIT License
* Copyright (c) 2018 Kārlis Goba
* Copyright (C) 2019 Ricardo Lima Caratti
* Copyright (C) 2021, Charles Hill (W5BAA)
* Copyright (C) 2024, Jim Conrad (KQ7B)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Features
* Single band, FT8 Tranceiver
* Supports 160-15 meters (tested on 40m and 20m)
* Small Size, 4.0 X 2.8", 4-layer board
* Powered by a single 5V USB source (e.g. portable USB power block) delivering ~375 mA
* Si4735 SSB Receiver
* TCXO stabilized Si5351 Clock
* SN74ACT244 PA delivering ~250 mW
* ADIF Contact Logging to SD card
* Adafruit 320 X 480 Resistive Color Touch Screen
* Station configuration via SD file, config.json
* Adafruit Ultimate GPS-derived location and UTC time

# DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.  To optimize program storage and processing speed so the Teensy is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.  For Teensy 4.1, the revised sample rate required replacement of the core AudioStream.h file with the AudioStream6400.h file in the PocketFT8FW sketch folder.

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

# Motivation and Related Projects
About 1966, the ARRL Handbook published a receiver design known as the Junior Miser's Dream challenging us to achieve so much with so little.  Today, Charley Hill's concept for a self-contained FT8 transceiver, ideal for portable operation, again achieves so much with so little.  The entire transceiver fits in a small enclosure, powered by a USB power block.  Pocket FT8 Revisited is a derivative of Charley's concept.

Unlike the uSDX multi-mode designs, Pocket FT8 focuses entirely on FT8.  Yes, yes, I know.  That's why I have other radios.  Pocket FT8 fits in the pocket of my backpack, doesn't require an external key, microphone, headphone, cell phone, or computer, and offers a completely self-contained transceiver for a contest-like "QSO."  It's clean.

An alternative approach, previously investigated as YASDR, is to construct an FT8 radio hat for a Raspberry 5 supporting the comprehensive wsjtx/etc natively.  This archived project offers considerable flexibility but with higher power requirements (I'm getting too old to pack heavy batteries in the mountains;).  This approach is worth revisiting in the future, especially for POTA.

Charley and Barb have also progressed with another derivative having a multiband FT8 transceiver, DX FT8, at https://github.com/WB2CBA/DX-FT8-FT8-MULTIBAND-TABLET-TRANSCEIVER, replacing the SI4735 with a Tayloe receiver.

# Manifest
* BenchTests:  Arduino sketches for incremental tests of the hardware
* Bibliography:  "..shoulders of giants"
* Investigations:  Code and simulations exploring technologies for this project
* PocketFT8XcvrFW:  The transceiver firmware sources
* Extras:  Additional files required to build the firmware
* RFFilters:  Investigations into various filter designs
* PocketFT8XcvrHW:  KiCad 8 files for the PCB
* Mfg:  Files associated with PCB manufacturing
* Extras:  Contains Teensy AudioStream.h modified for 6400 samples/second
* Schematics:  PDFs of hardware schematics
* SpectralPurity:  Results of investigations into transmitter output

# Versions
* V1.00 PCB gerbers and pos files submitted to PCBWay.
* V1.10 Transmitter works.  Receiver occasionally decodes FT8 messages.  See Issue #23 on github.
* V2.00 A new PCB moves the SI5351 and MCP342x to Wire1 to avoid I2C noise (Issue #23) hampering the SI4735

# TODO
Pocket FT8 is a work-in-progress.  While the V2.00 hardware and firmware will complete QSOs, it lacks features you might find elsewhere.
* Hashed FT8 callsigns
* Free text messages (much of the code is present but not integrated)
* Extensive testing --- I'm hoping PlatformIO will eventually provide some relief with its unit testing support
* FT8 contesting and DXpedition features found in wsjtx
* Improve the GPS connector and add the PPS trace to PCB

# Design Notes
* Versions of Pocket FT8 Revisited have been tested on 40M at KQ7B.
* The receiver is based upon the SI4735 chip offering a compact, low-power approach for most HF bands  Following up on a tip from the SI4735's application notes, the chip was found to be sensitive to noise from other I2C traffic on the same bus as the SI4735.  Disabling the touchscreen I2C in firmware resolved the issue, greatly improving reception.  The V2.00 boards address this issue by moving the touchscreen (MCP342X) and SI5351 traffic from the Wire to the Wire1 I2C bus, isolating the SI4735 on Wire.  Likewise, the V2.00 boards pay more attention to signal routing adjacent to noise-sensitive circuitry.
* The V1.10 transmitter PA employed a MMIC a la Charles Hill's original design.  The V2.00 boards replaced the MMIC with the readily available SN74ACT244 line driver which delivers more power (an idea borrowed from Barb in the DX FT8) with similar current draw.
* Many clock designs use a TCXO driving one of the SI5351's crystal pins and often in conflict with the SI5351 application notes recommendations.  Many designers have commented (not always of one voice;) on this approach.  Following up on a now-misplaced tip re. phase noise, this design employs a low jitter TCXO driving the SI5351's CLKIN pin.  Following up on yet another misplaced tip regarding the SI5351's phase noise when driving heavy loads, the V2.00 boards buffer the XCLK signal from the SI5351.  All this may be overkill but it was easy to implement.
* The V2.00 boards and the original Pocket FT8 project use an external MCP342x ADC to poll the touchscreen.  With a non-blocking ADC library, the Teensy 4.1 could likely use the second internal ADC but this idea has not yet been tested (it can be achieved using a patch to the V2.00 boards).  Alternative approaches include using a touchscreen controller.
* The V1.10 LP filter lacked rejection of received signals below the operating frequency.  Spectral investigations into the found surprisingly strong signals in the 500..7000 kHz range having the potential to limit the receiver's performance.  The V2.00 board filter plugins offer a chance to explore a bandpass design to protect the receiver's front end.  This has not yet been tested.
* The V2.00 boards support the Adafruit Ultimate GPS Breakout board.  The firmware supports using the PPS digital interrupt signal to determine when the GPS acquires a fix; however, the V2.00 PCB requires a patch wire to connect the PPS pin to Teensy Digital Pin 2, and the GPS connector has the PPS pin in an awkward location for a ribbon cable.  This will hopefull be fixed in the V3.00 hardware.  The patch is easy to solder and I recommend it.

# Boards and Parts Availability
Pocket FT8 Revisited is not a kit.  If you seek a kit-like experience, check-out QRP Labs, the uSDX follow-ons, or the DX FT8 kit as Pocket FT8 construction demands some engineering skills and tools including KiCAD, the Arduino IDE, and Visual Studio Code with PlatformIO extensions.  This project is more about tinkering than operating.  That said, sometimes I have spare boards for one version or another.

# Building the Firmware
1. Install Arduino 2.X IDE
2. Using Arduino 2.X, install support for the Teensy 4.1 board
3. Install Visual Studio Code and PlatformIO
4. Using PlatformIO, install support for the Teensy 4.1 platform
5. Using PlatformIO, install support for the required libraries (documented in the PocketFT8XcvrFW folder)
6. Copy Extras/AudioStream6400.h into
 .../Library/Arduino15/packages/teensy/hardware/avr/1.59.0/cores/teens4 asArduinoStream.h --- This file redefines AUDIO_SAMPLE_RATE_EXACT enabling Teensy 4.1 to operate the audio stream at 6400 samples/second (required).  You absolutely must do this.
5. Build with PlatformIO

# Modifying the Hardware
Pocket FT8 Revisited was designed with KiCAD V8, and the PCBs were fabbed and assembled by PCBWay.  There are two boards, the main board and a daughter board for the filter.

# Building the Hardware
1. Attach the Teensy 4.1 MPU to the board using low profile headers.
2. Attach the Adafruit resistive 320x480 touchscreen with high profile headers.
3. Construct a suitable filter (e.g. 5-pole Chebyshev) for your chosen band of operation and solder it into the FL1 spot.  The RFFilters folder contains a KiCAD design for the filter daughter board.
4. Hand solder SMD CR2032 battery holder to reverse side of PCB and install battery (before powering-up the Teensy).
5. Hand solder THT red XMIT LED
6. Wind THT T1 (10T bifilar #26 on FT37-43 core) and install
7. Install right angle header for GPS
8. Install SMD SMA antenna socket
9. Smoke test with wall wart USB supply (not your expensive computer;)
10. Build and execute each of the BenchTests in order to check-out the circuit subsystems

# Putting Pocket FT8 Revisited On-the-Air
1. Create an SD file, config.json, to configure the Pocket FT8 for your station.  The required parameters are, callsign and frequency.  If you don't have a GPS, you'll also need location.  Install the SD card in Teensy (not the Adafruit display).
2. If you are using a GPS, connect it now.
3. Connect an antenna 
4. Connect your host computer and load your firmware

# Usage
## Configuration
During setup(), the rig reads the config.json file, if available, from the Teensy SD card.  The SD card and configuration file are required to enable the transmitter.  Important configuration parameters include:
* callsign      Station callsign.  Required to enable the transmitter.
* frequency     Operating frequency in kHz (default is 7074).
* locator       Four letter Maidenhead grid square (Required).
* enableAVC     Enable/disable SI4735 AVC (default is enabled).
* gpsTimeout    Seconds setup() will wait for the GPS to acquire a fix
* qsoTimeout    Seconds the QSO Sequencer will retransmit a msg without receiving a usable response from remote station (default is 180)
## GPS
If available, the rig acquires the current UTC time and location (Maidenhead grid square) from the attached GPS.  The first attempt occurs during setup() and does not require the PPS signal.  If the PPS signal *is* available (V2.00 hardware requires a patch wire connecting the PPS pin to Teensy digital pin 2), the firmware continues to monitor the GPS and will acquire the UTC time and location when/if they later become available.  Before a GPS fix is obtained, the firmware uses the current date/time from the battery-backed Teensy Real Time Clock (RTC) and displays that date/time in red.  After a fix is obtained, the date/time display becomes yellow.  The GPS is not required but facilitates logging and synchronization with world-wide FT8 timeslots.

## Logging
Pocket FT8 logs successful contacts to an ADIF file on the Teensy SD disk.  The date/time are recorded in UTC after the rig has successfully acquired a GPS fix.  If the rig has never had a GPS fix, the date/time come from the Teensy RTC which was likely initialized when the firmware was loaded into Teensy by your host computer.  The logging software considers a contact successful when the rig obtains the remote station's callsign and signal report.  Without a GPS, location can be obtained from the configuration file.

## Robo-Op
Pocket FT8 firmware includes a sequencer somewhat akin (or ajar...;) to those used in wsjtx and DX FT8.  The sequencer conducts a standard FT8 QSO initiated with your station's CQ, or with your reply to a station calling CQ.  The sequencer automagically coordinates its transmissions with those of the remote station to avoid "doubling" (transmitting in the same timeslot as the remote).  In most cases, the sequencer can prepare a reply during the FT8 "dwell" time (between timeslots) and transmit in the subsequent timeslot.  The sequencer is implemented as a giant state machine attempting to make the best of difficult conditions/responses to complete a QSO.  The sequencer has a configurable QSO Timeout feature to abort a run-on QSO (including CQ) arising from QRM, QRN, or a QRT/QLF remote station.  


