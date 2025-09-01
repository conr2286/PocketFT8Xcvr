// Enable comments in the JSON configuration file
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include "Config.h"

#include <ArduinoJson.h>
#include <SD.h>

#include "NODEBUG.h"
#include "PocketFT8Xcvr.h"
#include "UserInterface.h"

extern UserInterface ui;

/**
 * @brief Read CONFIG.JSON file into the config structure
 */
void readConfigFile() {
    JsonDocument doc;  // Key-Value pair doc
    File configFile = SD.open(CONFIG_FILENAME, FILE_READ);
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        char msg[40];
        snprintf(msg, sizeof(msg), "ERROR:  Unable to read Teensy SD file, %s\n", CONFIG_FILENAME);
        ui.applicationMsgs->setText(msg);
        delay(5000);
    }

    // Extract the configuration parameters from doc or assign their defaults to the config struct
    strlcpy(config.callsign, doc["callsign"] | DEFAULT_CALLSIGN, sizeof(config.callsign));               // Station callsign
    config.frequency = doc["frequency"] | DEFAULT_FREQUENCY;                                             // Carrier freq in kHz
    strlcpy(config.locator, doc["locator"] | "", sizeof(config.locator));                                // Four char maidenhead grid locator
    config.enableAVC = doc["enableAVC"] | DEFAULT_ENABLE_AVC;                                            // Enable automatic volume control
    config.gpsTimeout = doc["gpsTimeout"] | DEFAULT_GPS_TIMEOUT;                                         // GPS timeout
    config.qsoTimeout = doc["qsoTimeout"] | DEFAULT_QSO_TIMEOUT;                                         // QSO timeout
    config.enableDuplicates = doc["enableDuplicates"] | DEFAULT_ENABLE_DUPLICATES;                       // Respond to duplicates in log
    strlcpy(config.logFilename, doc["logFilename"] | DEFAULT_LOG_FILENAME, sizeof(config.logFilename));  // Log SD filename
    strlcpy(config.myName, doc["myName"] | DEFAULT_MY_NAME, sizeof(config.myName));                      // Operator's name
    strlcpy(config.m0, doc["M0"] | "", sizeof(config.m0));                                               // Free text msg 0
    strlcpy(config.m1, doc["M1"] | "", sizeof(config.m1));                                               // Free text msg 1
    strlcpy(config.m2, doc["M2"] | "", sizeof(config.m2));                                               // Free text msg 2
    strlcpy(config.my_sota_ref, doc["my_sota_ref"] | "", sizeof(config.my_sota_ref));                    // My station's SOTA Reference

    configFile.close();

    // Report configuration
    String configMsg = String("call=") + String(config.callsign) + String(" freq=") + String(config.frequency) + String(" kHz\n");
    if (config.enableDuplicates) configMsg += String("enableDuplicates==1 ");
    configMsg += String("log=") + String(config.logFilename) + String(" name=") + String(config.myName);
    configMsg += String(" M0='") + String(config.m0) + String("'");
    ui.applicationMsgs->setText(configMsg.c_str());

    // Let the config report linger on the display for a moment
    delay(3000);
}
