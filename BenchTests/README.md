# README.MD

The BenchTests exist to check-out a newly constructed board.  They are embedded tests, executing in  Teensy.  Most are easily built using the Arduino IDE 2.00 as they are fairly simple in structure.

## Usage
The tests are numbered, Test01-*, Test02-*, etc.  The numerical order of the tests offers a recommended ordering of which tests to execute first, second and so on.

## Issues
Considerable duplication of supporting code between PocketFT8FW and the BenchTests exist because the Arduino IDE does not readily handle code sharing between tests and the production firmware.  The longterm solution to this problem will likely have the tests join the production firmware in PlatformIO which offers an improved way to share code between libraries and tests.

## Documentation
At this time, many tests include unfortunately sparce comments in the code describing what they exercise and what results can be expected.