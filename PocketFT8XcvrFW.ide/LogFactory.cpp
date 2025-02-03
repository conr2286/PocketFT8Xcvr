/**
 * Builds concrete instances of a LogFile
 *
 * Design Notes
 *  LogFactory is designed to (well, someday...) enable support for different types of
 *  log files (e.g. ADIF, CSV, TXT, XLSX, whatever), each derived from LogFile.
 *
 * Limitations
 *  At this time, the factory can only create an instance of an ADIFlog.
 *
 *
**/
#include <SD.h>
#include "DEBUG.h"
#include "LogFile.h"
#include "LogFactory.h"
#include "ADIFlog.h"



/**
 * ADIFlog factory method
 *
 * @param sd  Pointer to the SD drive to contain the log file
 * @param fileName Pointer to the SD file's name
 *
 **/
static LogFile* LogFactory::buildADIFlog(SDClass* sd, char* fileName) {
  DTRACE();
  return new ADIFlog(sd, fileName);
}



/**
 * CSVlog factory method
 *
 * @param sd  Pointer to the SD drive to contain the log file
 * @param fileName Pointer to the SD file's name
 *
 **/
static LogFile* LogFactory::buildCSVlog(SDClass* sd, char* fileName) {
  DTRACE();
  return new CSVlog(sd, fileName);
}