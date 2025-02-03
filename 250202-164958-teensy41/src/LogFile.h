
#pragma once

#include <SD.h>
#include "Contact.h"



class LogFile {

public:

  virtual int logContact(Contact* contact);

protected:
  SDClass* sd;
  char* fileName;

};