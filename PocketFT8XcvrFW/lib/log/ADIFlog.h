#pragma once

#include "Contact.h"
#include "ContactLogFile.h"

class ADIFlog : public ContactLogFile {
   public:
    ADIFlog(const char* fileName);     // Constructor
    int logContact(Contact* contact);  // Records an entry in the log file

   private:
};
