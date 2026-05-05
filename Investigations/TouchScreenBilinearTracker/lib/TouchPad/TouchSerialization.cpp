#include <Arduino.h>
#include <FS.h>

#include "TouchScreen.h"

/**
 * @brief Save ("serialize") the TouchScreen calibration state to a File
 * @param theFile Writeable file
 * @return true if successful, false otherwise
 *
 * DISCUSSION:
 * The touchCalibrationTable contains the calibration data which must be serialized to
 * save/restore the calibration state (and avoid recalibration).  The serialized data
 * is validated with a CRC16 "checksum".
 *
 * TouchScreen's serialization methods are implemented in separate cpp files
 * allowing the linker to select the media (e.g. SD, EEPROM, etc) required by the
 * application.  This implementation is for a File (e.g. SD) media.
 *
 * ATTRIBUTION:
 *  Original implementation by Jim Conrad with CRC guidance from CoPilot.
 *
 * LICENSE:
 *  Copyright (C) 2026 Jim Conrad
 *  MIT https://opensource.org/license/mit
 */
bool TouchScreen::serialize(File theFile) {
    theCalibrationTable.checksum = crc16((uint8_t*)&theCalibrationTable.nodes, sizeof(TouchCalibrationNode) * N_TARGETS);  // Checksum of nodes[]
    int n = theFile.write(&theCalibrationTable, sizeof(TouchCalibrationTable));                                            // Write nodes and checksum to theFile
    return n == sizeof(TouchCalibrationTable);                                                                             // Hopefully wrote the entire array
}

/**
 * @brief Restore ("deserialize") the TouchScreen state from a File
 * @param theFile Readable file
 * @return true if successful, false otherwise
 */
bool TouchScreen::deserialize(File theFile) {
    int n = theFile.read(&theCalibrationTable, sizeof(TouchCalibrationTable));                                          // Read the calibration data and stored checksum
    if (n != sizeof(TouchCalibrationTable)) return false;                                                               // Check for FileIO problems
    uint16_t computedChecksum = crc16((uint8_t*)&theCalibrationTable.nodes, sizeof(TouchCalibrationNode) * N_TARGETS);  // Computed checksum
    return computedChecksum == theCalibrationTable.checksum;                                                            // Confirm computed and read checksums match
}

/**
 * @brief Helper method to calculate a simple 16-bit CRC
 * @param data Data buffer
 * @param length sizeof buffer
 * @return crc16 of buffer
 *
 * @note We are using the CRC-16-CCITT polynomial (0x1021)
 */
uint16_t TouchScreen::crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;  // Standard initial state

    for (size_t i = 0; i < length; i++) {
        // Bring the next byte into the top 8 bits of the CRC register
        crc ^= (uint16_t)data[i] << 8;

        for (int j = 0; j < 8; j++) {
            // Check if the leftmost bit is 1
            if (crc & 0x8000) {
                // Shift and XOR with the polynomial
                crc = (crc << 1) ^ 0x1021;
            } else {
                // Just shift
                crc <<= 1;
            }
        }
    }
    return crc;
}
