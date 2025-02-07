
// Define the system-independent file access modes
typedef enum {
    MODE_WRITE,  // Open for appending onto end of file
    MODE_READ    // Open for reading file
} LogFileModeType;

// Define the Arduino implementation for the SD file system
#ifdef ARDUINO

#include <SD.h>

class LogFile {
   public:
    LogFile();
    bool open(const char* fileName, LogFileModeType mode);
    int write(const char c);
    int write(const char* bfr);
    int write(const char* bfr, int count);
    void close();

   private:
    SDClass& theFileSystem = SD;
    File theFile;
};

#else
#include <stdio.h>

class LogFile {
   public:
    LogFile();
    bool open(const char* fileName, LogFileModeType mode);
    int write(const char c);
    int write(const char* bfr);
    int write(const char* bfr, int count);
    void close();

   private:
    FILE theFile;
};

#endif