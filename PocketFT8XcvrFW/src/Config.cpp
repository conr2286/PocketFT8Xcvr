// Enable comments in the JSON configuration file
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include "Config.h"

#include <ArduinoJson.h>
#include <SD.h>

#include "DEBUG.h"
#include "UserInterface.h"
#include "PocketFT8Xcvr.h"

extern UserInterface ui;


//Implementation of the ConfigType object
//ConfigType config;

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
    strlcpy(config.callsign, doc["callsign"] | DEFAULT_CALLSIGN, sizeof(config.callsign));  // Station callsign
    config.frequency = doc["frequency"] | DEFAULT_FREQUENCY;
    strlcpy(config.locator, doc["locator"] | "", sizeof(config.locator));
    config.enableAVC = doc["enableAVC"] | DEFAULT_ENABLE_AVC;
    config.gpsTimeout = doc["gpsTimeout"] | DEFAULT_GPS_TIMEOUT;
    config.qsoTimeout = doc["qsoTimeout"] | DEFAULT_QSO_TIMEOUT;
    config.enableDuplicates = doc["enableDuplicates"] | DEFAULT_ENABLE_DUPLICATES;
    strlcpy(config.logFilename, doc["logFilename"] | DEFAULT_LOG_FILENAME, sizeof(config.logFilename));
    configFile.close();

    // Report configuration
    String configMsg = String("Config callsign=") + String(config.callsign) + String(" frequency=") + String(config.frequency) + String(" kHz\n");
    if (config.enableDuplicates) configMsg += String("enableDuplicates==1 ");
    configMsg += String("logFilename=") + String(config.logFilename);
    ui.applicationMsgs->setText(configMsg.c_str());

    // DPRINTF("enableAVC=%d\n", config.enableAVC);
    // DPRINTF("enableDuplicates=%d", config.enableDuplicates);

    // Let the config report linger on the display for a moment
    delay(2000);
}
