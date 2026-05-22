#include <Arduino.h>
#include <FS.h>
#include <DEBUG.h>

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
 * The caller is responsible for creating and closing the serialized data file.
 *
 * The calibration data is protected from inadvertent damage by a CRC.
 *
 * ATTRIBUTION:
 *  Original implementation by Jim Conrad with crucial CRC guidance from CoPilot.
 *
 * LICENSE:
 *  Copyright (C) 2026 Jim Conrad
 *  MIT https://opensource.org/license/mit
 */
bool TouchScreen::serialize(File theFile) {
    // Sanity checks
    if (!initialized || !calibrated) {
        DTRACE();
        return false;
    }

    // Save calibration data and its checksum
    calibrationTable.checksum = crc16((uint8_t*)&calibrationTable.nodes, sizeof(TouchCalibrationNode) * N_TARGETS);  // Checksum of nodes[]
    int n = theFile.write(&calibrationTable, sizeof(TouchCalibrationTable));                                         // Write nodes and checksum to theFile
    return n == sizeof(TouchCalibrationTable);                                                                       // Hopefully wrote the entire array
}  // serialize()

/**
 * @brief Restore ("deserialize") the TouchScreen state from a File
 * @param theFile Readable file
 * @return true if successful, false otherwise
 */
bool TouchScreen::deserialize(File theFile) {
    // Sanity checks
    if (!initialized) {
        DTRACE();
        return false;
    }

    // Restore calibration data from file
    int n = theFile.read(&calibrationTable, sizeof(TouchCalibrationTable));  // Read the calibration data and stored checksum
    if (n != sizeof(TouchCalibrationTable)) return false;                    // Check for FileIO problems

    // Verify checksum
    uint16_t computedChecksum = crc16((uint8_t*)&calibrationTable.nodes, sizeof(TouchCalibrationNode) * N_TARGETS);  // Computed checksum
    calibrated = (computedChecksum == calibrationTable.checksum);

    return calibrated;  // Confirm computed and read checksums match
}  // deserialize()

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
}  // crc16()
