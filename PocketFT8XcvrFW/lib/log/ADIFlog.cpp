/**
** ADIF --- ADIF logging to a File
**
** @author Jim Conrad (KQ7B)
**
** Design Notes:
**  There are a number of different classes in play here.  The idea is to
**  separate the responsibilities for assembling details (e.g. callsigns,
**  signal reports, etc) about a contact from the log file's encoding
**  (e.g. ADIF, CSV, TXT, XLSX, whatever).  Furthermore, we may someday
**  support many encodings, so we've constructed a factory that
**  can build different types of logs.  The various classes are:
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

#include "ADIFlog.h"

#include <string.h>

#include "Contact.h"
#include "ContactLogFile.h"
#include "DEBUG.h"
#include "FileSystemAdapter.h"

// Define the size of the longest log entry (including the NUL)
#define LOG_ENTRY_SIZE 256
#define FIELD_SIZE 32

/**
 * ADIFlog constructor
 *
 * @param sd Pointer to the SD drive object where the log will be written
 * @param fileName Name of the log file on the above drive
 *
 * Usage:  Caller is reponsible for initializing the specified SD drive
 * with something like, SD.begin()
 *
 * Note:  If ADIF required a header (a la CSV), then here is where we'd write
 * the header to the log file.
 *
 **/
ADIFlog::ADIFlog(const char* fileName) {
    DTRACE();

    // Initialize our member variables
    this->fileName = fileName;
}

/**
** Implements logContact to record a valid Contact entry to the SD disk file
**
** @return 0==success, -1==error
**
**/
int ADIFlog::logContact(Contact* contact) {
    char entry[LOG_ENTRY_SIZE];
    char field[FIELD_SIZE];

    DFPRINTF("workedCall='%s', myCall='%s', band='%s', mode='%s', qsoDate='%s', qsoTime='%s', workedRSL='%s', myRSL='%s')\n", contact->getWorkedCall(), contact->getMyCall(), contact->getBand(), contact->getMode(), contact->getQSOdate(), contact->getQSOtime(), contact->getWorkedRSL(), contact->getMyRSL());

    // Only log valid contacts
    if (!contact->isValid()) {
        DTRACE();
        return -1;  // Error:  Invalid contact
    }

    // Open the log file for appending
    bool err = logFileAdapter.open(this->fileName, MODE_WRITE_FILE);
    if (err) {
        DTRACE();
        return -1;  // Error:  Unable to open fileName on sd drive
    }

    // Initialize the log entry
    entry[0] = 0;  // NUL terminator

    DTRACE();

    // Assemble the required fields into the log entry string
    snprintf(field, sizeof(field), "<qso_date:%u>%s", strlen(contact->getQSOdate()), contact->getQSOdate());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "<time_on:%u>%s", strlen(contact->getQSOtime()), contact->getQSOtime());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "<call:%u>%s", strlen(contact->getWorkedCall()), contact->getWorkedCall());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "<station_call:%u>%s", strlen(contact->getMyCall()), contact->getMyCall());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "<band:%u>%s", strlen(contact->getBand()), contact->getBand());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "<mode:%u>%s", strlen(contact->getMode()), contact->getMode());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "<rst_sent:%u>%s", strlen(contact->getWorkedRSL()), contact->getWorkedRSL());
    strlcat(entry, field, sizeof(entry));
    snprintf(field, sizeof(field), "<rst_rcvd:%u>%s", strlen(contact->getMyRSL()), contact->getMyRSL());
    strlcat(entry, field, sizeof(entry));

    // Assemble the optional fields into the log entry string
    if (strlen(contact->getMyLocator())) {
        snprintf(field, sizeof(field), "<my_gridsquare:%u>%s", strlen(contact->getMyLocator()), contact->getMyLocator());
        strlcat(entry, field, sizeof(entry));
    }
    if (strlen(contact->getWorkedLocator())) {
        snprintf(field, sizeof(field), "<gridsquare:%u>%s", strlen(contact->getWorkedLocator()), contact->getWorkedLocator());
        strlcat(entry, field, sizeof(entry));
    }
    if (strlen(contact->getMySOTAref())) {
        snprintf(field, sizeof(field), "<my_sota_ref:%u>%s", strlen(contact->getMySOTAref()), contact->getMySOTAref());
        strlcat(entry, field, sizeof(entry));
    }
    if (strlen(contact->getWorkedSOTAref())) {
        snprintf(field, sizeof(field), "<sota_ref:%u>%s", strlen(contact->getWorkedSOTAref()), contact->getWorkedSOTAref());
        strlcat(entry, field, sizeof(entry));
    }

    // And a couple more optional fields just for fun
    if (strlen(contact->getRig())) {
        snprintf(field, sizeof(field), "<my_rig:%u>%s", strlen(contact->getRig()), contact->getRig());
        strlcat(entry, field, sizeof(entry));
    }
    if (strlen(contact->getPwr())) {
        snprintf(field, sizeof(field), "<tx_pwr:%u>%s", strlen(contact->getPwr()), contact->getPwr());
        strlcat(entry, field, sizeof(entry));
    }

    // Append End-of-Record
    strlcat(entry, "<eor>\n", sizeof(entry));
    DPRINTF("entry='%s'\n", entry);

    // Record the assembled entry in the log file
    logFileAdapter.write(entry);
    logFileAdapter.close();
    return 0;
}
