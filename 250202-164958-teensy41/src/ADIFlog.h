#pragma once
#include <SD.h>
#include "Contact.h"
#include "LogFile.h"



class ADIFlog : public LogFile {

public:
  ADIFlog(SDClass* sd, char* fileName);            //Constructor
  int logContact(Contact* contact);  //Records an entry in the log file
};