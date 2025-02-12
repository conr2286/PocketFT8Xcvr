#pragma once

#include <SD.h>



// Define the system-independent file access modes
typedef enum {
    MODE_WRITE_FILE,  // Open for appending onto end of file
    MODE_READ_FILE    // Open for reading file
} LogFileModeType;


class LogFile {
   public:
    bool open(const char* fileName, LogFileModeType mode);
    int write(const char c);
    int write(const char* bfr);
    int write(const char* bfr, int count);
    void close();

   private:
    SDClass& theFileSystem = SD;
    File theFile;
};
