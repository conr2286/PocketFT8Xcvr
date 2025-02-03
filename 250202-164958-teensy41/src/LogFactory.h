#pragma once

#include <SD.h>
#include "LogFile.h"
#include "ADIFlog.h"
#include "CSVlog.h"

class LogFactory {

public:

  //Define the factory methods
  static LogFile* buildADIFlog(SDClass* sd, char* fileName);
  static LogFile* buildCSVlog(SDClass* sd, char* fileName);
};