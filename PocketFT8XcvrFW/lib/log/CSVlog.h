#pragma once

#include "Contact.h"
#include "ContactLogFile.h"

class CSVlog : public ContactLogFile {
   public:
    CSVlog(const char* fileName);  // Constructor
    int logContact(Contact* contact);     // Records an entry in the log file
};