/**
** CSVlog --- CSV logging to a File
**
** @author Jim Conrad (KQ7B)
**
** Design Notes:
**  There are a number of different classes in play here.  The idea is to
**  separate the responsibilities for assembling details (e.g. callsigns,
**  signal reports, etc) about a contact from the log file's encoding
**  (e.g. ADIF, CSV, TXT, XLSX, whatever).  Furthermore, we may someday
**  support multiple encodings, so we've constructed a factory that
**  someday could build different types of logs.  The various classes are:
**
**    LogFile:    Defines the interface for logging a contact in any log file.
**    LogFactory: Builds instances of various types (e.g. ADIF, CSV...) of logs.
**    ADIFlog:    Logs a contact with ADIF encoding
**    CSVlog:     Logs a contact with CSV encoding
**    Contact:    Assembles fields about a QSO that will be logged
**
**  If you want to log in a different encoding, e.g. TXT, then you would
**  create a new TXTlog class and modify the LogFactory to build a TXTlog
**  instance.
**
**/
#include "CSVlog.h"

#include <Arduino.h>
#include <SD.h>
#include <string.h>

#include "Contact.h"
#include "ContactLogFile.h"
#include "DEBUG.h"

// Define the size of the longest log entry (including the NUL)
#define LOG_ENTRY_SIZE 256
#define FIELD_SIZE 16

/**
 * Constructor
 *
 * @param fileName Name of the log file on the above drive
 *
 * Usage:  Caller is reponsible for initializing the specified SD drive
 * with something like, SD.begin()
 *
 **/
CSVlog::CSVlog(const char* fileName) {
    char entry[LOG_ENTRY_SIZE];

    // Initialize our member variables
    this->fileName = fileName;

    // //If the file does not already exist, then we need to write the CSV header
    // if (!sd->exists(fileName)) {

    //   //Open the new log file
    //   File logFile = this->sd->open(this->fileName, FILE_WRITE);
    //   if (logFile == NULL) {
    //     return;  //Error:  Unable to open fileName on sd drive
    //   }

    //   //Assemble the CSV header
    //   entry[0] = 0;  //Empty string
    //   strlcat(entry, "qso_date, ", sizeof(entry));
    //   strlcat(entry, "time_on, ", sizeof(entry));
    //   strlcat(entry, "call, ", sizeof(entry));
    //   strlcat(entry, "station_call, ", sizeof(entry));
    //   strlcat(entry, "band, ", sizeof(entry));
    //   strlcat(entry, "mode, ", sizeof(entry));
    //   strlcat(entry, "rst_sent, ", sizeof(entry));
    //   strlcat(entry, "rst_rcvd, ", sizeof(entry));
    //   strlcat(entry, "my_gridsquare, ", sizeof(entry));
    //   strlcat(entry, "gridsquare, ", sizeof(entry));
    //   strlcat(entry, "my_sota_ref, ", sizeof(entry));
    //   strlcat(entry, "sota_ref", sizeof(entry));

    //   //Append the end-of-record indicator (newline for a CSV file)
    //   strlcat(entry, "\n", sizeof(entry));
    //   DPRINTF("header='%s'\n",entry);

    //   //Write the CSV header entry to the log file
    //   logFile.write(entry, strlen(entry));
    //   logFile.close();
    // }  // !exists

}  // constructor

/**
** Implements logContact to record a valid Contact entry to the SD disk file
**
** @return 0==success, -1==error
**
**/
int CSVlog::logContact(Contact* contact) {
    char entry[LOG_ENTRY_SIZE];
    char field[FIELD_SIZE];

    // Only log valid contacts
    if (!contact->isValid()) {
        return -1;  // Error:  Invalid contact
    }

    // Open the log file for appending
    bool err = logFileAdapter.open(this->fileName, MODE_WRITE);
    if (err) {
        return -1;  // Error:  Unable to open fileName on sd drive
    }

    // Initialize the log entry
    entry[0] = 0;  // NUL terminator

    // Assemble the required fields into the log entry string
    snprintf(field, sizeof(field), "%s, ", contact->getQSOdate());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "%s, ", contact->getQSOtime());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "%s, ", contact->getWorkedCall());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "%s, ", contact->getMyCall());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "%s, ", contact->getBand());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "%s, ", contact->getMode());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "%s, ", contact->getWorkedRSL());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "%s, ", contact->getMyRSL());
    strlcat(entry, field, sizeof(entry));

    // We always record the optional fields in a CSV log because the fields are positional
    snprintf(field, sizeof(field), "%s, ", contact->getMyLocator());
    strlcat(entry, field, sizeof(entry));

    snprintf(field, sizeof(field), "%s, ", contact->getWorkedLocator());
    strlcat(entry, field, sizeof(entry));

    snprintf(field, sizeof(field), "%s, ", contact->getMySOTAref());
    strlcat(entry, field, sizeof(entry));

    snprintf(field, sizeof(field), "%s", contact->getWorkedSOTAref());
    strlcat(entry, field, sizeof(entry));

    // Append End-of-Record
    strlcat(entry, "\n", sizeof(entry));
    DPRINTF("entry='%s'", entry);

    // Record the assembled entry in the log file
    logFileAdapter.write(entry, strlen(entry));
    logFileAdapter.close();

}  // logContact()
