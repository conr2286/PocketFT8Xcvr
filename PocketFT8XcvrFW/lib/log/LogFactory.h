#pragma once

#include "ADIFlog.h"
#include "CSVlog.h"
#include "ContactLogFile.h"

class LogFactory {
   public:
    // Define the factory methods
    static ContactLogFile* buildADIFlog(const char* fileName);
    static ContactLogFile* buildCSVlog(const char* fileName);
};