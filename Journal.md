# Project Journal

# Outstanding Questions
* Why did Hans power the HX8357D from the Teensy's 3.3V regulator rather then the
5V supply where more current is available?
* What is the purpose of Charles' complex RC network between Si4735
  and Teensy?  The Skyworks application notes suggest a simple approach.
* Is a notch needed in the output filter to reject the second harmonic?
* Why did Charles include T1 following the MMIC if the MMIC's output impedance is 50 ohms?

# Log
1. HX8537D molex connector wired to connect IM2 to +3OUT, avoiding the
need to solder the on-board jumper.
2. Paranoia:  A goal of the bypass caps near the active devices/modules is to localize the anticipated ground currents rather than disperse them throughout the ground plane where they might become troublesome.

