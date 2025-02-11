/**
 * Builds concrete instances of a ContactLogFile
 *
 * Design Notes
 *  LogFactory is designed to (well, someday...) enable support for different types of
 *  log files (e.g. ADIF, CSV, TXT, XLSX, whatever), each derived from ContactLogFile.
 *
 * Limitations
 *  At this time, the factory only supports an instance of an ADIFlog.
 *
 *
 **/
#include "LogFactory.h"

#include "ADIFlog.h"
#include "ContactLogFile.h"
#include "DEBUG.h"

/**
 * ADIFlog factory method
 *
 * @param fileName Pointer to the SD file's name
 *
 **/
static ContactLogFile* LogFactory::buildADIFlog(const char* fileName) {
    DTRACE();
    return new ADIFlog(fileName);
}

/**
 * CSVlog factory method
 *
 * @param fileName Pointer to the SD file's name
 *
 **/
static ContactLogFile* LogFactory::buildCSVlog(const char* fileName) {
    DTRACE();
    return new CSVlog(fileName);
}