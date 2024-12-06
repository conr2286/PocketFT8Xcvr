# Pocket FT8 Revisited
An SMD implementation of Charles Hill's Palm-Sized Pocket FT8 Transceiver updated for Teensy 4.1 on a 4-Layer board

# Attribution
* Copyright (C) 2021, Charles Hill (W5BAA)
* Si4735 Library developed by Ricardo Caritti: https://github.com/pu2clr/SI4735
* FT8 Decoding Library by Karlis Goba (YL3JG): https://github.com/kgoba/ft8_lib
* SN74ACT244 PA by Barb (WB2CBA)
* PC Board and enhancements by Jim Conrad (KQ7B)
* And the Adafruit/PJRC libraries

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
* Small Size, 4.0 X 2.8"
* Powered by a single 5V USB source (e.g. portable USB power block) delivering ~375 mA
* TCXO
* Si4735 SSB Receiver & Si5351 Transmit FSK Clock
* SN74ACT244 line driver PA
* SD Card Contact Logging
* Adafruit 320 X 480 Resistive Color Touch Screen
* Station configuration via SD card file, config.json

# DSP Audio Architecture
Decoding FT8 requires significant data storage and processing speed.  In order to optimize both program storage and processing speed requirements so that the Teensy is not over taxed, the Teensy Audio Library has been modified to allow Analog to Digital conversion to be run at the rate of 6400 samples per second. This allows audio data processing to be done at 3200 Hz. The 3200 Hz audio processing with a 2048 FTT to process the received audio for FT8 decoding yields a bin spacing of 3.125 Hz.  For Teensy 4.1, the revised sample rate required replacement of the core AudioStream.h file with the AudioStream6400.h file in the PocketFT8FW sketch folder.

The algorithms developed by Karlis Goba use the 3.125 Hz spaced FFT bins to be screened in both frequency and time so that errors in symbol frequency and time reception  can be overcome to provide really great FT8 decoding. The end spacing of the FT8 algorithms is 6.25 Hz.

# Motivation and Related Projects
About 1966, the ARRL Handbook published a receiver design known as the Junior Miser's Dream that achieved so much with so little.  Likewise, Charley Hill's design for a self-contained FT8 transceiver, ideal for POTA/SOTA, again achieves so much with so little.  The entire transceiver fits in a portable enclosure, and the power demand is acceptable for a USB power block.  Pocket FT8 Revisited V1.10 is a mostly faithful re-implementation of Charley's concept with revisions largely limited to those necessary for the Teensy 4.1.  From there, the project has and continues to implement enhancements to get just a little more from so little.

Unlike the uSDX multi-mode designs, Pocket FT8 focuses entirely on FT8.  Yes, yes, I know.  That's why I have other radios.  Pocket FT8 fits in the pocket of my backpack, doesn't require an external key, microphone, headphone, cell phone, or computer, and offers a completely self-contained transceiver for a contest-like POTA/SOTA "QSO."  It's clean.

An alternative approach, previously investigated as YASDR, is to construct an FT8 radio hat for a Raspberry 5 supporting the comprehensive wsjtx/etc natively.  This archived project offers considerable flexibility but with higher power requirements (I'm getting too old to pack heavy batteries in the mountains;).  This approach is worth revisiting in the future if we can get the power demand down.

Charley and Barb have also progressed beyond the original Pocket FT8 with a multiband FT8 transceiver, DX FT8, at https://github.com/WB2CBA/DX-FT8-FT8-MULTIBAND-TABLET-TRANSCEIVER, replacing the SI4735 with a Tayloe receiver.

# Manifest
* BenchTests:  Arduino sketches for incremental tests of the hardware
* Bibliography:  "..shoulders of giants"
* Investigations:  Code and simulations exploring technologies for this project
* PocketFT8XcvrFW:  Arduino sketch for the Pocket FT8 Revisited transceiver
* PocketFT8XcvrHW:  KiCad 8 files for the PCB

# Versions
* V1.00 PCB gerbers and pos files submitted to PCBWay.
* V1.10 Transmitter works.  Receiver occasionally decodes FT8 messages.  See Issue #23 on github.
* V2.00 A new PCB moves the SI5351 and MCP342x to Wire1 to avoid I2C noise (Issue #23) hampering the SI4735

# Design Notes
* Versions of Pocket FT8 Revisited are tested on 40M and 20M at KQ7B.
* The receiver is based upon the SI4735 chip offering a compact, low-power approach for most HF bands  Following up on a tip from the SI4735's application notes, the chip was found to be sensitive to noise from other I2C traffic on the same bus as the SI4735.  Disabling the touchscreen I2C in firmware resolved the issue, greatly improving reception at the loss of the GUI.  The V2.00 boards address this issue by moving the touchscreen (MCP342X) and SI5351 traffic from the Wire to the Wire1 I2C bus, isolating the SI4735 on Wire.  Likewise, the revised boards pay more attention to signal routing adjacent to noise-sensitive circuitry.
* CW and SSB modes appear infeasible.  The SI4735's PLL limits the receiver's tuning to 1 kHz steps which is incompatible with amateur CW and SSB operation.  If you need these modes in a kit, check-out the QRP Labs transceivers; Hans does an awesome job.
* The V1.10 transmitter PA employed a MMIC a la Charles Hill's original design.  The V2.00 boards replaced the MMIC with the readily available SN74ACT244 line driver which is expected to deliver a bit more power (an idea borrowed from the DX FT8).
* Many clock designs use a TCXO driving one of the SI5351's crystal pins and often in conflict with the SI5351 application notes recommendations.  Following up on a tip re. phase noise with that approach, this design employs a low jitter TCXO driving the SI5351's CLKIN pin.  Following up on yet another tip regarding the SI5351's phase noise when driving heavy loads, the V2.00 boards buffer the XCLK signal from the SI5351.  All this may be overkill but it was easy to implement.
* The V2.00 boards and the original Pocket FT8 project use an external MCP342x ADC to poll the touchscreen.  With a non-blocking ADC library, the Teensy 4.1 could likely use the second internal ADC but this idea has not yet been tested (it can be achieved using a patch to the V2.00 boards).  Alternative approaches include using a touchscreen controller.
* The V1.10 boards employed a 5-pole Chebyshev LP filter providing a measured 40dB of 2nd harmonic rejection and excellent 3rd harmonic rejection of a steady state (Tune) carrier.  However, FT8 transmit/receive switching produced a very brief but annoying 2nd harmonic burst.  The root cause has not yet been investigated, but the V2.00 boards are designed to accept filter plugins allowing experimentation with alternative filters (e.g. a harmonic trap) if the problem persists.
* The V1.10 LP filter lacked rejection of received signals below the operating frequency.  Investigations into the SI4735 performance found surprisingly strong signals in the 540..7000 kHz range having the potential to limit the receiver's performance.  The V2.00 board filter plugins offer a chance to explore a bandpass design to protect the receiver's front end.
* The long-term goal is to use a GPS to identify the operator's grid square and sync the clock to UTC.  The V2.00 boards support a GPS connection but no firmware has been completed as yet.
* The touchscreen circuitry is proving troublesome.  While it walmost always works, its occasional erratic performance is annoying.  One of the project's long-term goals is to better understand and resolve the issue with hardware, firmware or both.

# Boards and Parts Availability
Pocket FT8 Revisited is not a kit.  If you seek a kit-like experience, check-out QRP Labs, the uSDX follow-ons, or the DX FT8 kit as Pocket FT8 construction demands some engineering skills.  That said, I sometimes have a *very* limited quantity of excess 4-layer boards with most SMD components in-place if you'd like to experiment.  Alternatively, the KiCad files are available as a starting point for your innovations.



