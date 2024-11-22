# Pocket FT8 Transceiver Revisited
An implementation of Charles Hill's Palm-Sized Pocket FT8 Transceiver updated for Teensy 4.1 with SMD components on a 4-Layer board

# Attribution and License
* Copyright (C) 2021, Charles Hill
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba: https://github.com/kgoba/ft8_lib
* Enhancements by Lars Petersen, OZ1BXM
* SN74ACT244 PA by Barb (Tnx, Barb --- I've long wondered if that would work:)
* PC Board by Jim Conrad, KQ7B (MIT License)

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Please use this software at your own risk

# Features
* FT8 Message Transmit and Receive
* Small Size, 4.0 X 2.8"
* Powered by a single 5V USB source (e.g. portable consumer power bank)
* TCXO
* Si4735 SSB Receiver & Si5351 Transmit FSK Clock
* SN74ACT244 line driver PA
* SD Card Contact Logging
* Adafruit 320 X 480 Resistive Color Touch Screen
* Configuration via SD card file, config.json

# DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.

In order to optimize both program storage and processing speed requirements so that the Teensy is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.  For Teensy 4.1, the revised sample rate required replacement of the core AudioStream.h file with the AudioStream6400.h file in the PocketFT8FW sketch folder.

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

# Motivation
In the 1960's, the ARRL Handbook published a receiver design gem known as the Miser's Dream.  I've long admired likeminded approaches achieving so much with so little.  This project revisits Charles Hill's concept for a self-contained FT8 transceiver, ideal for POTA/SOTA, and it too achieves much with little.  The entire transceiver fits in a portable enclosure, and the power demand is acceptable from a USB power block.  Rather than start with a clean slate, I chose to start with an implementation of Pocket FT8 with minimal revision, largely confined to that necessary for the Teensy 4.1 (as the 3.6 is no longer available) and implemented on a single 4-layer board, and follow that with enhancements addressing real-world on-the-air shortcomings.

Unlike the uSDX multi-mode designs, Pocket FT8 focuses entirely on FT8.  Yes, yes, I know.  That's why I have other radios.  Pocket FT8 fits in the pocket of my backpack, doesn't require an external key, microphone, headphone or computer, and offers a completely self-contained transceiver for a contest-like POTA/SOTA QSO.  It's clean.

An alternative approach, previously investigated as YASDR, might be to construct an FT8 radio hat for a Raspberry 5 supporting the comprehensive wsjtx/etc natively.  This archived project offers considerable flexibility but with higher power requirements (I'm getting too old to pack heavy batteries in the mountains;).

Charles and Barb have also progressed beyond the original Pocket FT8 with a multiband transceiver, DX FT8, on github at, https://github.com/WB2CBA/DX-FT8-FT8-MULTIBAND-TABLET-TRANSCEIVER

# Manifest
* BenchTests:  Arduino sketches for incremental tests of the hardware
* Bibliography:  "..shoulders of giants"
* Investigations:  Code and simulations exploring technologies for this project
* Legacy:  Legacy files useful for understanding the big picture of the Pocket FT8 approach
* PocketFT8XcvrFW:  Arduino 2.0 firmware sketch for the Pocket FT8 transceiver
* PocketFT8XcvrHW:  KiCad 8 files for the PCB

# Status
As of Oct 16, 2024, the V1.10 receiver is demodulating 40m USB signals and the FT8 code seems to occasionally successfully decode a message.  The transmitter produces power on the expected frequency, but no contact has yet been attempted although a station responded when I had the touchpad disabled.
As of Nov 20, 2024, the V2.00 boards have been received from PCBway and await turn-on.

# Versions
* V1.00 PCB gerbers and pos files submitted to PCBWay.
* V1.10 Transmitter works.  Receiver occasionally decodes FT8 messages.  See Issue #23 on github.
* V2.00 New PCB moves SI5351 and MCP342x to Wire1 to avoid noise (Issue #23) hampering the SI4735

# Design Notes
* The receiver is based upon the SI4735 chip offering a compact, low-power approach for most HF bands with an anticipated performance sacrifice compared to a Tayloe front-end.  So far, this sacrifice has not been a significant issue on the air in Idaho (USA).  Following up on a tip from the SI4735's application notes, the chip was found to be sensitive to noise from other I2C traffic on the same bus as the SI4735; disabling the touchscreen I2C traffic resolved the issue, greatly improving reception at the loss of the GUI.  The V2.00 boards address this issue by moving the touchscreen (MCP342X) and SI5351 traffic from Wire to the Wire1 I2C bus, isolating the SI4735 on Wire.  Likewise, the revised boards pay more attention to signal routing adjacent to noise-sensitive circuitry.  The mood at KQ7B is we haven't yet squeezed everything possible out of this chip.
* The V1.10 transmitter PA employed a MMIC a la Charles Hill's original design.  The V2.00 boards replaced the MMIC with the readily available SN74ACT244 line driver which is expected to deliver a bit more power.
* Many clock designs use a TCXO driving one of the SI5351's crystal pins and often in conflict with the SI5351 application notes recommendations.  Following up on a tip re. phase noise with that approach, this design uses a reportedly low jitter TCXO driving the SI5351's CLKIN pin.  Following up on yet another tip regarding the SI5351's phase noise when driving heavy loads, the V2.00 boards buffer the XCLK signal from the SI5351.  All this may be overkill.
* The V2.00 boards and the original Pocket FT8 project use an external MCP342x ADC to poll the touchscreen.  With a non-blocking ADC library, the Teensy 4.1 could likely use the second internal ADC but this idea has not yet been tested (it can be achieved using a patch to the V2.00 boards).
* The V1.10 boards employed a 5-pole Chebyshev LP filter providing a measured 40dB of 2nd harmonic rejection and excellent 3rd harmonic rejection of a steady state (Tune) carrier.  However, FT8 transmit/receive switching produced a brief but stronger 2nd harmonic burst.  The root cause of this burst has not yet been investigated, but the V2.00 boards are designed to accept filter plugins allowing experimentation with alternative filters (e.g. a harmonic trap) if the problem persists.
* The V1.10 boards did not offer rejection of received signals below the operating frequency.  Investigations into the SI4735 performance found strong signals in the 540..7000 kHz range with the potential to limit the receiver's performance.  The V2.00 board filter plugins offer a chance to explore a bandpass design to protect the receiver's front end.
* The long-term goal is to use a GPS to identify the operator's grid square and sync the RTC to UTC.  The V2.00 boards support a GPS connection but no firmware has been developed as yet.

# Boards and Parts Availability
Pocket FT8 Revisited is not a kit.  If you seek a Heathkit-like experience, check-out the QRP Labs products as Pocket FT8 construction demands some engineering skills.  That said, I sometimes have a *very* limited quantity of excess 4-layer boards with most SMD components in-place if you'd like to experiment.
