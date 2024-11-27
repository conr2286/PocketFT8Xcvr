/**
** ADIF --- ADIF logging to SD File
**
** @author Jim Conrad (KQ7B)
**
**/

#include <SD.h>
#include "ADIF.h"


/**
** ADIF Constructor
**
** @param logFile Name of the logfile on the SD disk
** @param myCall My station's call sign
** @param myGridsquare My station's gridsquare
** @param logMode How to log FT8 QSO mode (e.g. DATA or FT8)
** @param mySOTARef My SOTA Reference
** @param mySIGinfo My (POTA) Special Interest Group info
**
** The required parameters are logFile and myCall.  Unused parameters may be NULL.
**
**/
ADIF::ADIF(char* logFile, char* myCall, char* myGridsquare, char* logMode, char* mySOTARef, char* mySIGinfo) {

  //Initialize our member variables
  this->logFile = logFile;
  this->myCall = myCall;
  this->myGridsquare = myGridsquare;
  this->qsoMode = logMode;
  this->mySOTARef = mySOTARef;
  this->mySIGinfo = mySIGinfo;

  //Initialize the SD library if available
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Error:  ADIF unable to access the SD card");  //Someday move this message to display
  }

}  //ADIF()



/**
** Logs a QSO to the SD disk file
**
** @param theirCall Worked station's callsign (required)
** @param theirGridsquare Worked station's gridsquare (optional, may be NULL)
**
** @return 0==success, -1==error
**
** 
**
** The logfile is opened and closed with each call to logQSO to "ensure" the data is actually flushed
** to the SD media.
**/
int ADIF::logQSO(char* theirCall, char* theirGridsquare, char* qsoDate, char* qsoTime) {

  //Open the log file
  File logFile = SD.open(this->logFile, FILE_WRITE);
  if (!logFile) {
    printf("Error:  Unable to open log file '%s' for writing\n", this->logFile);
    return -1;
  }

  //Assemble the required log entries
  char entry[256];
  snprintf(entry,sizeof(entry), 
  "<mode:4>%4s<qso_date:8>%8s<time_on:6>%6s<station_callsign:7>%7s<call:7>%7s", this->qsoMode, qsoDate, qsoTime, this->myCall, theirCall);

  //Append the optional log entries one-by-one
}
