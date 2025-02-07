/**
 *  FileSystemAdapter provides an interface to the host operating system's filesystem
 *
 *  The FileSystemAdapter enables the log classes to be somewhat independent of a
 *  host operating system's file ystem by consolidating a filesystem's dependencies
 *  into this one class.
 *
 *  The adapter can conceptually be implemented using conditionally compiled code for
 *  multiple filesystems of interest, or you might provide separate source
 *  files for the host systems and build only the one of interest.
 *
 *  DESIGN NOTES:
 *      + The adapter is not a singleton because, at least in principle, a host
 *      operating might support multiple file systems, each with a different
 *      interface.
 *      + The adapter must implement these two methods:  begin(), open()
 *      + The open() method provides access to a
 *
 *  LIMITATIONS
 *      + In practice, some filesystem dependencies (e.g. file name length, character
 *      set, punctuation, etc) are exposed.
 */

#include "FileSystemAdapter.h"

//---------------------------------------------------------------------------------------------------------------------
//  Arduino SD Filesystem Implementation
//---------------------------------------------------------------------------------------------------------------------
#ifdef ARDUINO

#include <SD.h>

/**
 * @brief Opens the specified file for reading or writing
 * @param fileName Specified file's name
 * @param mode MODE_READ or MODE_WRITE for reading or writing to end of a file
 * @return true==success, false==failure
 *
 * If mode==MODE_WRITE the file will be created if it doesn't exist.  Writing
 * will begin at the end of an existing file.
 *
 * Warning:  When executing on an Arduino, the setup() function needs to invoke
 * SD.begin() to initialize the SD filesystem before LogFile can be opened.
 */
bool LogFile::open(const char* fileName, LogFileModeType mode) {
    bool result = false;
    switch (mode) {
        case MODE_READ:
            theFile = SD.open(fileName, FILE_READ);
            result = true;
            break;
        case MODE_WRITE:
            theFile = SD.open(fileName, FILE_WRITE);
            result = true;
            break;
        default:
            theFile = NULL;
            break;
    }
    return result;
}  // open()

/**
 * @brief Write a NUL-terminated char* string to the log file
 * @param bfr NUL-terminated char* string
 * @return Number of bytes written
 */
int LogFile::write(const char* bfr) {
    return theFile.write(bfr);
}  // write()

/**
 * @brief Write the specified number of bytes to the log file
 * @param bfr Data to be written
 * @param count Number of bytes to write
 * @return Number of bytes actually written
 */
int LogFile::write(const char* bfr, int count) {
    return theFile.write(bfr, count);
}  // write()

/**
 * @brief Write the specified byte to the log file
 * @param c The byte to be written
 * @return Number of bytes actually written
 */
int LogFile::write(char c) {
    return theFile.write(c);
}

void LogFile::close(void) {
    return theFile.close();
}  // close()

#else
//---------------------------------------------------------------------------------------------------------------------
//  Native UNIX-like Filesystem Implementation
//---------------------------------------------------------------------------------------------------------------------
#include <stdio.h>

/**
 * @brief Opens the specified file for reading or writing
 * @param fileName Specified file's name
 * @param mode MODE_READ or MODE_WRITE for reading or writing to end of a file
 * @return true==success, false==failure
 *
 * If mode==MODE_WRITE the file will be created if it doesn't exist.  Writing
 * will begin at the end of an existing file.
 *
 */
bool LogFile::open(const char* fileName, LogFileModeType mode) {
    bool result = false;
    switch (mode) {
        case MODE_READ:
            theFile = fopen(fileName, "r");
            result = true;
            break;
        case MODE_WRITE:
            theFile = fopen(fileName, "a");
            result = true;
            break;
        default:
            theFile = NULL;
            break;
    }
    return result;
}  // open()

/**
 * @brief Write a NUL-terminated char* string to the log file
 * @param bfr NUL-terminated char* string
 * @return Number of bytes written
 */
int LogFile::write(const char* bfr) {
    return fputs(bfr, theFile);
}  // write()

/**
 * @brief Write the specified number of bytes to the log file
 * @param bfr Data to be written
 * @param count Number of bytes to write
 * @return Number of bytes actually written
 */
int LogFile::write(const char* bfr, int count) {
    return fwrite(bfr, count, 1, theFile);
}  // write()





void LogFile::close(void) {
    return theFile.close();
}  // close()

#endif
