# Pocket FT8 PA Prototype

This investigation pursues a Class-D push-pull PA for the Pocket FT8 transceiver

## Features
* Operation from a single +5V USB power source
* ~1 Watt output into 50 Ohm resistive load
* Single CLK input
* Chebyshev or trapped Butterworth low-pass filter choices

## Motivation
Seek an approach to increase the Pocket FT8's 50..100mW output to the vicinity of 1 Watt while maintaining operation from a +5V USB portable power source.

## Design Rational
* The push-pull design was chosen to reduce 2nd harmonic output.  Although a perfect square wave clock would not have 2nd harmonic energy, the amplified real-world clock presented to the low-pass filter is not a true square wave.
* Simulations found both a 5th order Chebyshev and Butterworth (augmented with a harmonic trap) provide effective filtering of the 2nd and 3rd harmonics.  The PCB artwork supports investigation of both approaches.
* While BS170s have struggled in some applications, especially at higher voltages, higher power, and higher frequencies, they are not expected to be overly problematic on 40 and 20M with a 5V power source.
* The BS170 low input capacitance should be within the drive capability of the SN74LVC2G86 XOR gates.
* The differential drive for the push-pull finals can be achieved using either two CLK outputs from an SI5351 or from two XOR gates.  Since the SI5351's limited drive capacity already necessitates the use of some form of driver, this design chose the XOR route and the use of the SN74LVC2G86 as the driver.

## Smoke Warning
Without a CLK input signal, the XOR gates will drive one or the other BS170 into saturation, shorting the DC 5V supply to GND through T1 and L3.  ALWAYS SUPPLY THE CLK SIGNAL BEFORE APPLYING +5V POWER TO THE PA PROTOTYPE!!!

