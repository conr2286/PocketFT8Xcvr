##DESCRIPTION
    This folder contains PlatformIO Unity tests for turning-on a new Pocket FT8 board.

##VERSION
    As of March 23, 2026, the tests target Version 4 of the hardware

##NOTES
    * A new board should be fully assembled with both SMT and THT parts installed prior to attempting any of these tests
    * A new board, passing a "smoke" pre-test, is ready to attempt test_01
    * The tests are named (e.g. test_01, test_02, etc) in numerical order, the order in which they should be attempted on a new board
    * These tests are not all pure unit tests as some interact with the operator using the hardware (a pure unit test wouldn't interact with a human;).
