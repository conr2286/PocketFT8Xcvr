
#pragma once

#include <Arduino.h>
#include <string>
#include <map>

#include "FileSystemAdapter.h"
#include "Contact.h"
#include "lexical.h"

class ContactLogFile {
   public:
    virtual int logContact(Contact* contact) = 0;
    virtual ~ContactLogFile() {};


   protected:
    const unsigned LOG_ENTRY_SIZE = 256;
    const unsigned FIELD_SIZE = 32;
    LogFile logFileAdapter;
    const char* fileName;
    unsigned nLogEntries;         //Number of new log entries recorded in this session
    void buildListOfKnownCallsigns(void);
    std::map<std ::string, int> knownCallsigns;
    int parseADIF(char* value, char* contact, const char* key, unsigned size);


};