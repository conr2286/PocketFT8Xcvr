
#pragma once

#include <Arduino.h>

#include "Contact.h"
#include "FileSystemAdapter.h"
#include "lexical.h"

class ContactLogFile {
   public:
    virtual int logContact(Contact* contact) = 0;
    virtual ~ContactLogFile() {};
    static bool isKnownCallsign(char*);
    static void addKnownCallsign(char*);

   protected:
    const unsigned LOG_ENTRY_SIZE = 300;            // Max #chars in our ADIF contact entry
    const unsigned FIELD_SIZE = 64;                 // Max #chars in any of our ADIF fields
    static const unsigned callsignTableSize = 509;  // Prime size of callsign hash table
    LogFile logFileAdapter;                         // Filesystem adapter
    const char* fileName;                           // The log's filename on SD
    unsigned nLogEntries;                           // Number of new log entries recorded in this session

    void buildListOfKnownCallsigns(void);
    int parseADIF(char* value, char* contact, const char* key, unsigned size);
    static unsigned hashString(char*);
};