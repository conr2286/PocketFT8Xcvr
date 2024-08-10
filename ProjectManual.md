# Project Journal

# Outstanding Questions
Q: Why did Hans power the HX8357D from the Teensy's 3.3V regulator rather then the
5V supply where more current is available?

Q: What is the purpose of Charles' complex RC network between Si4735
  and Teensy?  The Skyworks application notes suggest a simple approach.
  
Q: Is a notch needed in the output filter to reject the second harmonic?
A: Simulation of the filter using a swept sine wave source found weak rejection of the second harmonic at 36 mHz.  Adherance to US FCC 97.307d seemingly depends upon the square wave XCLK composition of primarily odd-numbered harmonics.  Simulation with a trap constructed with a capacitor paralleled with L1 provides strong attenuation of the 36 mHz harmonic with minimal degradation of the 3rd at 54 mHz.  Conclusion:  add C26 in parallel with L1 to schematic but don't load the component on the board unless spectral testing requires it.

Q: Why did Charles include T1 following the MMIC if the MMIC's output impedance is 50 ohms?

# Log
1. HX8537D molex connector wired to connect IM2 to +3OUT, avoiding the
need to solder the on-board jumper.
2. Paranoia:  A goal of the bypass caps near the active devices/modules is to localize the anticipated ground currents rather than disperse them throughout the ground plane where they might become troublesome.

