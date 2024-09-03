# Pocket FT8 Class E PA Prototype

This simulation explores a Class-E PA for the Pocket FT8 transceiver

## Features
* Operation from a single +5V USB power source
* ~1 Watt output into 50 Ohm resistive load
* Single CLK input
* Tank circuit tuned for Class E voltage/current phasing

## Motivation
Seek an approach to increase the Pocket FT8's 50..100mW output to the vicinity of 1 Watt powered by a single +5V USB portable power brick for SOTA/POTA operaton.

## Design Rational
* FT8 operation is currently found within a narrow frequency range on any single band. 
* Multiband operation is not critical to SOTA/POTA operation
* However, power efficiency and heat dissipation are important, especially for SOTA.
* Anticipated operation from a single 5V power source relaxes the Class E tendancy to exceed practical Vds(max)
* While BS170s have struggled in some applications, especially at higher voltages, higher power, and higher frequencies, they are not expected to be overly problematic on 40 and 20M with a 5V power source.
* Consequently, the Class E approach appears attractive

## Questions
* Is the BS170 Rds at Vcc=5V satisfactory with two paralleled devices in Class E?  Or will a 1W Class E PA require three devices?
* Is the output network overly sensitive to frequency and load variations, or can it maintain satisfactory Class E operation over anticipated frequency shifts and loads?

## Attributions
* VK1SV Class E calculatior, https://people.physics.anu.edu.au/~dxt103/calculators/class-e.php

## Results
* Simulation was performed for (3) paralleled BS170 FETs driven by an approximation of (3) paralleled 74ACT00 gates with a 12.5 Ohm (50/4) load.  The tank circuit was simulated with real-world rather than ideal capacitor values.
* Drain voltage and current are roughly out-of-phase per Class E expectation.
* Limiting factors in the design may include the gate drive, Ciss, Rds and the internal charging of the FET architecture.
* Po is lower than design point (1 watt).
