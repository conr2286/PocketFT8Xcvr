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
** @param myGridsquare My station's gridsquare (optional)
** @param logMode How to log the QSO's mode (e.g. DATA or FT8)
** @param mySOTARef My SOTA Reference or NULL (optional)
** @param mySIGinfo My (POTA) Special Interest Group info or NULL (optional)
**
** Optional parameters may be NULL.
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

  //Initialize the SD library if card is available
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Error:  ADIF unable to access the SD card");  //Someday move this message to display
  }

}  //ADIF()



/**
** Logs a QSO entry to the SD disk file
**
** @param workedCall Worked station's callsign (required)
** @param qsoDate UTC start date for QSO (required)
** @param qsoTime UTC start time for QSO (required)
** @param workedGridsquare Worked station's gridsquare or NULL (optional)
** @param myRSL My signal report or NULL(optional)
** @param workedRSL Worked station's signal report or NULL (optional)
**
** @return 0==success, -1==error
**
** My station's call and the QSO mode (e.g. DATA or FT8) are obtained from ADIF's member
** variables and recorded in the log file.
**
** The logfile is opened and closed with each call to logQSO() ensuring the data gets
** flushed to the SD media (so that it may be removed).
**
** Entries for optional parameters passed to logQSO() as NULL pointers do not appear in the
** log file; empty (i.e. zero-length) parameters appear as white space in the log.
**
** Signal reports are logged as dB (e.g. "R-13") per FT8 conventions.
**/
int ADIF::logQSO(char* workedCall, char* qsoDate, char* qsoTime, char* workedGridsquare, char* myRSL, char* workedRSL) {

  //Open the log file
  File logFile = SD.open(this->logFile, FILE_WRITE);
  if (!logFile) {
    printf("Error:  Unable to open log file '%s' for writing\n", this->logFile);
    return -1;
  }

  //Assemble the required log items
  char entry[256];
  snprintf(entry, sizeof(entry),
           "<mode:4>%4s<qso_date:8>%8s<time_on:6>%6s<station_callsign:7>%7s<call:7>%7s", this->qsoMode, qsoDate, qsoTime, this->myCall, workedCall);

  //Append optional worked grid square to the log entry string
  if (workedGridsquare != NULL) {
    strlcat(entry, "<gridsquare:4>", sizeof(entry));
    strlcat(entry, workedGridsquare, sizeof(entry));
  }

  //Append optional my signal level to the log entry string
  if (myRSL != NULL) {
    strlcat(entry, "<myRSL:5>", sizeof(entry));
    strlcat(entry, myRSL, sizeof(entry));
  }

  //Append optional worked signal level to the log entry string
  if (workedRSL != NULL) {
    strlcat(entry, "<workedRSL:4>", sizeof(entry));
    strlcat(entry, workedRSL, sizeof(entry));
  }

  //Append a NL terminator
  strlcat(entry, "\n", sizeof(entry));

  //Write the completed entry to the log file
  logFile.write(entry, strlen(entry));

  //Flush log entry to the SD disk
  logFile.close();  //Flush
  return 0;         //Return success
}
