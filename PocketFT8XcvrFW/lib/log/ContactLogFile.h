
#pragma once

#include "FileSystemAdapter.h"
#include "Contact.h"

class ContactLogFile {
   public:
    virtual int logContact(Contact* contact);

   protected:
    LogFile logFileAdapter;
    const char* fileName;
};