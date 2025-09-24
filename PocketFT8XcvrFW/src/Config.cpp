// Enable comments in the JSON configuration file
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include "Config.h"

#include <ArduinoJson.h>
#include <SD.h>

#include "DEBUG.h"
#include "PocketFT8Xcvr.h"
#include "UserInterface.h"

/**
 * @brief Define the struct to record the low/high frequency in each band
 */
typedef struct BandLimit {
    unsigned low;   // Lowest frequency in this band
    unsigned high;  // Highest frequency in this band
} BandLimitType;

/**
 * @brief Define the low/high frequency limits of each supported amateur band
 *
 * @note The Si4735 can't support the higher HF bands.  Furthermore, the hardware
 * low-pass filter must be appropriate for the frequency configured in CONFIG.JSON
 *
 * The getLowerBandLimit() and getUpperBandLimit() functions search this table to
 * determine which band, if any, a specified frequency lies.
 */
static BandLimitType amateurBandLimits[] = {
    {1800, 2000},  // 160 meters
    {3500, 4000},  // 80 meters
    // Can the Si4735 support 60 meter channels???
    {7000, 7300},    // 40 meters
    {10130, 10150},  // 30 meters
    {14000, 14350},  // 20 meters
    {18068, 18168},  // 17 meters
    {21000, 21450}   // 15 meters
};

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
    config.operatingFrequency = doc["frequency"] | DEFAULT_FREQUENCY;                                    // Carrier freq in kHz
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
    config.tcxoCorrection = doc["tcxoCorrection"] | DEFAULT_TCXO_CORRECTION;

    configFile.close();

    // Report configuration
    // String configMsg = String("call=") + String(config.callsign) + String(" freq=") + String(config.frequency) + String(" kHz\n");
    // if (config.enableDuplicates) configMsg += String("enableDuplicates==1 ");
    // configMsg += String("log=") + String(config.logFilename) + String(" name=") + String(config.myName);
    // configMsg += String(" M0='") + String(config.m0) + String("'");
    // ui.applicationMsgs->setText(configMsg.c_str());

    // Determine the amateur band's min/max frequencies for the configured operating frequency (needed by Si4735)
    config.lowerFrequencyLimit = getLowerBandLimit(config.operatingFrequency);  // kHz
    config.upperFrequencyLimit = getUpperBandLimit(config.operatingFrequency);  // kHz

    // Check for invalid operating frequency
    DPRINTF("lowerFrequencyLimit=%u, operatingFrequency=%u, upperFrequencyLimit=%u\n", config.lowerFrequencyLimit, config.operatingFrequency, config.upperFrequencyLimit);
    if (config.lowerFrequencyLimit == 0 || config.upperFrequencyLimit == 0) config.operatingFrequency = 0;  // Short between the headsets :(

    // Display the configuration
    AListBox* popup = new AListBox(10, 10, 480 - 20, 320 - 20, A_RED);
    popup->addItem(popup, String("call=") + String(config.callsign) + String(" freq=") + String(config.operatingFrequency) + String(" kHz\n"));
    popup->addItem(popup, String("myName='") + String(config.myName) + String("'"));
    popup->addItem(popup, String("enableDuplicates=") + String(config.enableDuplicates));
    popup->addItem(popup, String("M0='") + String(config.m0) + String("'"));
    popup->addItem(popup, String("M1='") + String(config.m1) + String("'"));
    popup->addItem(popup, String("M2='") + String(config.m2) + String("'"));
    popup->addItem(popup, String("my_sota_ref='") + String(config.my_sota_ref) + String("'"));
    popup->addItem(popup, String(" "));

    // Let the config report linger on the display before removing it
    delay(5000);
    delete popup;
}  // readConfigFile()

/**
 * @brief Find the amateur band's lower frequency limit for operating frequency f
 * @param f Desired operating frequency in kHz
 * @return Lower limit in kHz or 0 if f is an invalid frequency
 */
unsigned getLowerBandLimit(unsigned f) {
    for (unsigned i = 0; i < sizeof(amateurBandLimits); i++) {
        if ((f >= amateurBandLimits[i].low) && (f <= amateurBandLimits[i].high)) return amateurBandLimits[i].low;
    }
    return 0;  // Oopsie... Short between the headsets
}

/**
 * @brief Find the amateur band's upper frequency limit for operating frequency f
 * @param f Desired operating frequency in kHz
 * @return Upper limit in kHz or 0 if f is an invalid frequency
 */
unsigned getUpperBandLimit(unsigned f) {
    for (unsigned i = 0; i < sizeof(amateurBandLimits); i++) {
        if ((f >= amateurBandLimits[i].low) && (f <= amateurBandLimits[i].high)) return amateurBandLimits[i].high;
    }
    return 0;  // Oopsie... Short between the headsets
}
