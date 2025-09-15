# Pocket FT8 Revisited
A derivative of Charles Hill's Palm-Sized Pocket FT8 Transceiver

# Attribution
* Copyright (C) 2025, James R Conrad (KQ7B), conr2286 *at* gmail *dot* com
* Copyright (C) 2021, Charles Hill (W5BAA), https://github.com/Rotron/Pocket-FT8 
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba (YL3JG): https://github.com/kgoba/ft8_lib
* SN74ACT244 PA by Barb (WB2CBA), https://github.com/WB2CBA/DX-FT8-FT8-MULTIBAND-TABLET-TRANSCEIVER 
* Adafruit, PJRC and many Arduino libraries copyrighted by their respective authors

# Permissive MIT License (applies to most project files)
* Copyright (c) 2018 Kārlis Goba
* Copyright (C) 2019 Ricardo Lima Caratti
* Copyright (C) 2021, Charles Hill (W5BAA)
* Copyright (C) 2024, 2025 Jim Conrad (KQ7B)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# CopyLeft GPL License (applies to the SI5351 library)
* Copyright (C) 2015-2019 Jason Milldrum and Dana H. Myers


# Features
* Single band, FT8 Tranceiver
* Designed for 160-15 meters, tested on 40m
* Small Size, 4.0 X 2.8", 4-layer board
* Powered by a single 5V USB source (e.g. portable USB power block) delivering ~375 mA
* Si4735 SSB Receiver
* TCXO stabilized Si5351 Clock
* SN74ACT244 PA delivering ~250 mW on 40M (Tnx, Barb;)
* ADIF Contact Logging to SD card
* Adafruit 320 X 480 Resistive Color Touch Screen
* Optional Adafruit Ultimate GPS-derived location and UTC date/time
* RoboOp sequences FT8 QSOs somewhat akin to wsjtx

# DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.  To optimize program storage and processing speed so the Teensy is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.  For Teensy 4.1, the revised sample rate requires replacement of the core AudioStream.h file with the AudioStream6400.h file in the PocketFT8FW sketch folder.

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

# Motivation and Related Projects
In the mid-60s, the ARRL Handbook published a receiver design known as the Junior Miser's Dream offering so much with so little.  Today, Charley Hill's concept for a self-contained FT8 transceiver, ideal for portable operation, again achieves so much with so little.  The entire transceiver fits in a small enclosure, powered by a USB power block.  Pocket FT8 Revisited is a derivative of Charley's original concept.

Unlike the uSDX and related multi-mode designs, Pocket FT8 focuses entirely on FT8.

Charley and Barb have progressed with another Pocket FT8 derivative, DX FT8, at https://github.com/WB2CBA/DX-FT8-FT8-MULTIBAND-TABLET-TRANSCEIVER, most notably replacing the SI4735 with a Tayloe receiver.  Pocket FT8 Revisited may someday
    take that route but, for the moment, the SI4735 (whose spec'd AM sensitivy
    is absymal) seems able to QSO with any station receiving its 375 mW transmission.

# Manifest
* BenchTests:  Arduino sketches for incremental tests of the hardware
* Bibliography:  "..shoulders of giants"
* Doc:  Minimalist user manual
* Enclosure:  STLs for 3D printing a plastic case for an older version (needs update)
* Examples:  Sample JSON configuration and ADIF log files
* Extras:  Modified Teensy AudioStream6400.h header (you *need* this)
* Investigations:  Code and simulations exploring technologies for this project
* Legacy:  Some old files I'm still hoarding 
* Mfg:  BOM files for assembly.  V2.00 has been built; the current BOM awaits tariff issues.
* PocketFT8XcvrFW:  The transceiver firmware sources
* PocketFT8XcvrHW:  KiCad files for the PCB
* RFFilters:  Investigations into various filter designs
* Mfg:  Files associated with PCB manufacturing
* Extras:  Contains Teensy AudioStream.h modified for 6400 samples/second.  You *need* this.
* Schematics:  PDFs of hardware schematics.  V2.00 matches V2.00 BOM.
* SpectralPurity:  Results of investigations into transmitter output using V2.00 hardware.

# Versions
* V1.00 PCB gerbers and pos files submitted to PCBWay.
* V1.10 Transmitter works.  Receiver occasionally decodes FT8 messages.  See Issue #23 on github.
* V2.00 Revised PCB moves the SI5351 and MCP342x to Wire1 to avoid I2C noise (Issue #23) hampering the SI4735
* V2.10 Implements GPS, ADIF logging and the RoboOp QSO sequencer.  Uses V2.00 hardware with patch wires for GPS connector.  The GPS is optional but *strongly* recommended.


# Status
* August, 2025:  Patched (for GPS PPS) V2.00 hardware tested with most current firmware on 40M.  The schematic and PCB are at V3.00 awaiting a satisfactory tariff prior to fabrication (I'm currently quoted $400+ to build just two boards).  The V3.00 hardware replaces the patch wires supporting the GPS PPS signal on V2.00 boards, and adds an expansion connector to implement future functionality.

# Implementationi Notes
* The receiver is based upon the SI4735 chip offering a compact, low-power approach for most HF bands  Following up on a tip from the SI4735's application notes, the chip was found to be extremely sensitive to noise from other I2C traffic on the same bus as the SI4735.  The V2.00 boards address this issue by moving the touchscreen (MCP342X) and SI5351 traffic from the Wire to the Wire1 I2C bus, isolating the SI4735 on Wire.  Likewise, the V2.00 boards pay more attention to signal routing adjacent to noise-sensitive circuitry.
* Many clock designs use a TCXO driving one of the SI5351's XTAL pins and often in conflict with the SI5351 application notes recommendations.  Many designers have commented (not always of one voice;) on this approach.  Following up on a now-misplaced tip re. phase noise, this design employs a low jitter TCXO driving the SI5351's CLKIN pin.  Following up on yet another misplaced tip regarding the SI5351's phase noise when driving heavy loads, the V2.00 boards buffer the XCLK signal from the SI5351.  All this may be overkill but it was easy to implement.
* The V2.00 boards and the original Pocket FT8 project use an external MCP342x ADC to poll the touchscreen.  With a non-blocking ADC library, the Teensy 4.1 could likely use the second internal ADC but this idea has not yet been tested (it can be achieved using a patch to the V2.00 boards).  Alternative approaches include using a touchscreen controller.  These are considered for a future V4.00.
* The V1.10 LP filter lacked rejection of received signals below the operating frequency.  Spectral investigations into the found surprisingly strong signals in the 500..7000 kHz range having the potential to limit the receiver's performance.  The V2.00 board filter plugins offer a chance to explore a bandpass design to protect the receiver's front end.  Suitable bandpass filters have been modeled but not yet been tested.
* The V2.00 boards support the Adafruit Ultimate GPS Breakout board.  The firmware supports using the PPS digital interrupt signal to determine when the GPS acquires a fix; however, the V2.00 PCB requires a patch wire to connect the PPS pin to Teensy Digital Pin 2, and the GPS connector has the PPS pin in an awkward location for a ribbon cable.  This will hopefull be fixed in the V3.00 hardware when tariffs permit.  The patch is easy to solder and I strongly recommend it.

# Boards and Parts Availability
Pocket FT8 Revisited is not a kit.  If you seek a kit-like experience, check-out QRP Labs, the uSDX follow-ons, or the DX FT8 kit because Pocket FT8 construction requires some engineering skills and tools including KiCAD, the Arduino IDE, and Visual Studio Code with the PlatformIO extensions.  This project is more about homebrewing than operating.  That said, sometimes I have spare boards for one version or another.

# Building the Firmware
1. Install Arduino 2.X IDE (the BenchTests still require the Arduino IDE, sorry)
2. Using Arduino 2.X, install support for the Teensy 4.1 board
3. Install Visual Studio Code and PlatformIO
4. Using PlatformIO, install support for the Teensy 4.1 platform
5. Using PlatformIO, install support for the required libraries (documented in the PocketFT8XcvrFW folder)
6. Copy Extras/AudioStream6400.h into
 .../Library/Arduino15/packages/teensy/hardware/avr/1.59.0/cores/teens4 asArduinoStream.h --- This file redefines AUDIO_SAMPLE_RATE_EXACT enabling Teensy 4.1 to operate the audio stream at 6400 samples/second (required).  You absolutely *must* do this; PocketFT8XcvrFW.cpp will not compile if you forget.
7. Modify the definitions of MINIMUM_FREQUENCY and MAXIMUM_FREQUENCY in PocketFT8XcvrFW.cpp for your choice of amateur band.
7. Build the firmware with PlatformIO.  The src files build without warnings; library files... not so much.

# Hardware Notes
Pocket FT8 Revisited was designed with KiCAD, and the V2.00 PCBs were fabbed and the SMD assembled by PCBWay.  There are two boards, the main board (PocketFT8XcvrHW) and a daughter board for the filter (RFFilters).  I used the 5Pole40MLPFilter in the SpectralPurity test.

# Building the Hardware
1. Attach the Teensy 4.1 MPU to the board using *low profile* headers.
2. Attach the Adafruit resistive 320x480 touchscreen with high profile headers (so the display clears the MPU)
3. Construct a suitable filter (e.g. 5-pole Chebyshev) for your chosen band of operation and solder it into the FL1 spot.  The RFFilters folder contains a KiCAD design for the filter daughter board.
4. Hand solder SMD CR2032 battery holder to reverse side of PCB and install battery (before powering-up the Teensy).  T
5. Hand solder THT red XMIT LED
6. Wind THT T1 (10T bifilar #26 on FT37-43 core) and install
7. Install right angle header for GPS.  Hand solder a PPS patch wire from the GPS connector to the MCU as described above.
8. Hand solder the SMD SMA antenna socket to the board
9. Smoke test with wall wart USB supply (not your expensive computer;)
10. Build and execute each of the BenchTests (with Arduino IDE 2.00) in numerical order to individually check-out the circuit subsystems.

# Putting Pocket FT8 Revisited On-the-Air
1. Create an SD file, config.json, to configure the Pocket FT8 for your station (see the sample file in Examples).  The required parameters are, callsign and frequency.  If you don't have a GPS, you'll also need location.  Install the SD card in Teensy (*not* in the Adafruit display).
2. If you are using a GPS, connect it now.
3. Connect an antenna 
4. Connect your host computer and load your firmware

# Usage
## CONFIG.JSON
During setup(), the rig reads the config.json file, if available, from the Teensy SD card.  The SD card and configuration file are required to enable the transmitter.  See the example config file in the Examples folder.  The important configuration parameters include:
* callsign      Station callsign.  *Required* to enable the transmitter.
* frequency     Operating frequency in kHz (default is 7074).
* locator       Four letter Maidenhead grid square (Required if no GPS).
* enableAVC     Enable/disable SI4735 AVC (default is enabled).
* gpsTimeout    Seconds setup() will wait for the GPS to acquire a fix
* qsoTimeout    Seconds the QSO Sequencer will retransmit a msg without receiving a usable response from remote station (default is 180)

## GPS
If available, the rig will use the current UTC time and location (Maidenhead grid square) from an attached GPS.  The V2.00 hardware requires a patch wire to connect the GPS PPS connector pin to Teensy digital pin 2.  The firmware monitors PPS interrupts and begins using the UTC time and location only when/if the GPS has acquired a satellite fix.  Without a GPS fix, the firmware uses the date/time from the battery-backed Teensy Real Time Clock (RTC) and displays that date/time in red.  After a fix is obtained, the date/time display appears in green.  The GPS is not required but *greatly* facilitates logging and accurate synchronization with FT8 timeslots.

## Logging
Pocket FT8 logs successful contacts to an ADIF file on the Teensy SD disk.  The date/time are recorded in UTC after the rig acquires a GPS fix.  If the rig has never had a GPS fix, the date/time come from the RTC initialized when the firmware was last loaded into Teensy by your host computer --- that may work for FT8 but the UTC timezone (for log) is unknown.  You really do want the GPS.  The logging software considers a contact successful when the rig obtains the remote station's callsign and signal report (yes, that's a little more severe than required by ARRL's LoTW).  Without a GPS, the grid locator can be obtained from the configuration file.

## Robo-Op
Pocket FT8 firmware includes a sequencer somewhat akin (or ajar...;) to that used in wsjtx and DX FT8.  The sequencer conducts a standard FT8 QSO initiated with your station's CQ, or with your reply to a station calling CQ.  The sequencer automagically coordinates its transmissions with those of the remote station to avoid "doubling" (transmitting in the same timeslot as the remote transmission).  In most cases, the sequencer prepares a reply during the FT8 "dwell" time (between timeslots) and can transmit in the very next timeslot.  If not... well... it will wait for an appriopriate timeslot.
The sequencer is implemented as a giant state machine that attempts to make the best of difficult conditions/responses to complete a troubled QSO.  The sequencer has a configurable QSO Timeout feature to abort a run-on QSO (including CQ) arising from QRM, QRN, or a QRT/QLF remote station.  As with most large state machines... the code is difficult to follow, but is mitigated with an abundance of comments that attempt to explain what's happening in the various event handlers and their resulting actions.


