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

#include <SD.h>

#include "DEBUG.h"

/**
 * @brief Opens the specified file for reading or writing
 * @param fileName Specified file's name
 * @param mode MODE_READ or MODE_WRITE for reading or writing to end of a file
 * @return true==error, false==success
 *
 * If mode==MODE_WRITE the file will be created if it doesn't exist.  Writing
 * will begin at the end of an existing file.
 *
 * Warning:  When executing on an Arduino, the setup() function needs to invoke
 * SD.begin() to initialize the SD filesystem before LogFile can be opened.
 */
bool LogFile::open(const char* fileName, LogFileModeType mode) {
    // DPRINTF("open('%s',%u)\n", fileName, mode);
    bool err = true;
    switch (mode) {
        case MODE_READ_FILE:
            theFile = SD.open(fileName, FILE_READ);
            err = false;
            break;
        case MODE_WRITE_FILE:
            theFile = SD.open(fileName, FILE_WRITE);
            err = false;
            break;
        default:
            theFile = NULL;
            break;
    }
    return err;
}  // open()

/**
 * @brief Read a NL-terminated line of text from an open log file
 * @param bfr Pointer to a char[] buffer to receive the text
 * @param size Sizeof(bfr)
 * @return Number of bytes read or -1 if EOF
 *
 * Notes:  The NL character is read but not returned in bfr.  An empty
 * line (terminated by NL) will return 0 bytes.  End-of-File will
 * return -1. The bfr will be NUL terminated if size>0.
 *
 */
int LogFile::readLine(char* bfr, int size) {
    int count = 0;

    // Deal with weird corner cases
    if (size == 0) return -1;  // Error, no usable buffer
    bfr[0] = 0;               // Initialize bfr with a NUL terminator
    if (size == 1) return -1;  // Error, no usable buffer

    //DTRACE();
    // Loop reading chars into bfr until NL, full, EOF or error
    while (count < size) {
        int c = theFile.read();  // Read one char from log file
        if (c == -1) {            // Error or EOF?
            if (count > 0) {      // Did loop actually read anything?
                return count;     // Yes, return count of chars read
            } else {              // No, nothing read.  Probably EOF.
                return -1;        // Return error indication
            }
        }
        if (c == '\n') return count;  // Finish on NL
        bfr[count++] = c;             // Store ordinary char in caller's bfr
        bfr[count] = 0;               // Always maintain a NUL terminator at end of bfr
    }
    return count;  // We filled bfr[] without finding NL
}  // readLine()

/**
 * @brief Write a NUL-terminated char* string to the log file
 * @param bfr NUL-terminated char* string
 * @return Number of bytes written
 */
int LogFile::write(const char* bfr) {
    // DPRINTF("write('%s')\n", bfr);
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
