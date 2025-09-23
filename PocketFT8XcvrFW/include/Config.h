#pragma once
#include <Arduino.h>

// Configuration parameters read from SD file
#define CONFIG_FILENAME "/CONFIG.JSON"

// Configuration data struct.  TODO:  Perhaps should be a class???
typedef struct Config {
    char callsign[12];                     // 11 chars and NUL
    char locator[5];                       // 4 char maidenhead locator and NUL
    unsigned lowerFrequencyLimit;          // Band's lower limit
    unsigned operatingFrequency;           // Operating operatingFrequency in kHz
    unsigned upperFrequencyLimit;          // Band's upper limit
    long tcxoCorrection;                   // HZ correction factor used by Si5351
    unsigned long audioRecordingDuration;  // Seconds or 0 to disable audio recording
    unsigned enableAVC;                    // 0=disable, 1=enable SI47xx AVC
    unsigned gpsTimeout;                   // GPS timeout (seconds) to obtain a fix
    unsigned qsoTimeout;                   // QSO timeout (seconds) to obtain a response
    bool enableDuplicates;                 // Enable RoboOp to contact duplicates
    char logFilename[24];                  // Log filename
    char myName[16];                       // Operator's personal name, not callsign
    char m0[14];                           // Free Text Message 0 and NUL
    char m1[14];                           // Free Text Message 1 and NUL
    char m2[14];                           // Free Text Message 2 and NUL
    char my_sota_ref[12];                  // My station's SOTA Reference
} ConfigType;

// Default configuration
#define MINIMUM_FREQUENCY 14000               // Min freq supported by HW filters
#define MAXIMUM_FREQUENCY 14350               // Max freq supported by HW filters
#define DEFAULT_FREQUENCY 14074               // kHz
#define DEFAULT_TCXO_CORRECTION 0             // Hz
#define DEFAULT_CALLSIGN ""                   // There's no realistic default callsign
#define DEFAULT_AUDIO_RECORDING_DURATION 0UL  // Default of 0 seconds disables audio recording
#define DEFAULT_ENABLE_AVC 1                  // SI4735 AVC enabled by default
#define DEFAULT_GPS_TIMEOUT 60                // Number of seconds before GPS fix time-out
#define DEFAULT_QSO_TIMEOUT 180               // Number seconds Sequencer will retry transmission without a response
#define DEFAULT_ENABLE_DUPLICATES false       // RoboOp will not contact duplicates
#define DEFAULT_LOG_FILENAME "LOGFILE.ADIF"   // Default ADIF Log Filename
#define DEFAULT_MY_NAME ""                    // Operator's personal name (not callsign)

void readConfigFile(void);
unsigned getLowerBandLimit(unsigned f);  // Calculate lower band limit for operating frequency f
unsigned getUpperBandLimit(unsigned f);  // Calculate upper band limit for operating frequency f
