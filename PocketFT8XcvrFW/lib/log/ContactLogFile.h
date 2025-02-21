
#pragma once

#include "FileSystemAdapter.h"
#include "Contact.h"

class ContactLogFile {
   public:
    virtual int logContact(Contact* contact) = 0;
    virtual ~ContactLogFile() {};

   protected:
    LogFile logFileAdapter;
    const char* fileName;
    unsigned nLogEntries;         //Number of new log entries recorded in this session
};