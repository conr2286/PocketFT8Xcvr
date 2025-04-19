

#include "display.h"

#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>

#include "GPShelper.h"
#include "HX8357_t3n.h"
#include "UserInterface.h"

char log_filename[] = "FT8_Traffic.txt";
File Log_File;

extern time_t getTeensy3Time();
extern char Station_Call[];  // six character call sign + /0
extern char Locator[];       // four character locator  + /0

extern int log_flag, logging_on;

extern GPShelper gpsHelper;  // Public data acquired by GPShelper

extern HX8357_t3n tft;
extern UserInterface ui;

/**
 * Helper function to pad a char[] to a specified length
 *
 * @param str  Pointer to the char[] string to pad
 * @param size sizeof(str) including the NUL terminator
 * @param c    The pad character (e.g. ' ')
 *
 * strlpad() pads the specified array containing a NUL terminated char string
 * with the specified char, c, ensuring that str remains NUL terminated.  The
 * resulting string, including the NUL terminator, will occupy no more than
 * size chars in str[].
 *
 **/
char* strlpad(char* str, unsigned size, char c) {
    const char NUL = 0;
    bool paddingUnderway = false;
    int i;

    for (i = 0; i < size - 1; i++) {
        if (str[i] == NUL) paddingUnderway = true;
        if (paddingUnderway) {
            str[i] = c;
        }
    }

    str[size - 1] = NUL;

    return str;

}  // strlpad()

/**
 * Local helper function to pad a char[] to a specified length
 *
 * @param dst  Pointer to the destination char[] string
 * @param src  Pointer to the source char[] string
 * @param c    The pad character (e.g. ' ')
 * @param size sizeof(dst) including the NUL terminator
 *
 * strlpad() copies chars from src[] to dst[], padding dst[] with c to ensure
 * it's filled (including the NUL terminator).  The resulting dst string,
 * including the NUL terminator, will occupy no more than size chars in dst[].
 *
 **/
char* strlpad(char* dst, char* src, char c, unsigned size) {
    const char NUL = 0;
    bool paddingUnderway = false;
    int i;

    for (i = 0; i < size - 1; i++) {
        if (src[i] == NUL) paddingUnderway = true;
        if (paddingUnderway) {
            dst[i] = c;
        } else {
            dst[i] = src[i];
        }
    }

    dst[size - 1] = NUL;

    return dst;

}  // strlpad()

// /**
//  * @brief Set the GUI's Transmit/Receive/Pending icon color
//  * @param indicator Specifies what we're doing
//  */
// void setXmitRecvIndicator(IndicatorIconType indicator) {
//     unsigned short color;  // Indicator icon color
//     char* string;
//     char paddedString[9];

//     switch (indicator) {
//         // We are receiving
//         case INDICATOR_ICON_RECEIVE:
//             color = HX8357_GREEN;
//             string = "RECEIVE";
//             break;
//         // Transmission pending for next appropriate timeslot
//         case INDICATOR_ICON_PENDING:
//             color = HX8357_YELLOW;
//             string = "PENDING";
//             break;
//         // Transmission in progress
//         case INDICATOR_ICON_TRANSMIT:
//             color = HX8357_RED;
//             string = "TRANSMIT";
//             break;
//         // Tuning in progress
//         case INDICATOR_ICON_TUNING:
//             color = HX8357_ORANGE;
//             string = "TUNING";
//             break;
//         // Lost in the ozone again
//         default:
//             color = HX8357_BLACK;
//             string = " ";
//             break;
//     }

//     // tft.setTextColor(color, HX8357_BLACK);
//     // // tft.setTextSize(2);
//     // tft.setCursor(DISPLAY_XMIT_RECV_INDICATOR_X, DISPLAY_XMIT_RECV_INDICATOR_Y);
//     strlpad(paddedString, string, ' ', sizeof(paddedString));
//     //tft.print(paddedString);
//     ui.displayMode(String(string), color);
// }  // setIndicatorIcon()

// void display_frequency(int x, int y, int value) {
//     char string[9];  // print format stuff
//     sprintf(string, "F=%i", value);
//     tft.setTextColor(HX8357_WHITE, HX8357_BLACK);
//     //tft.setTextSize(2);
//     tft.setCursor(x, y);
//     tft.print(string);
// }

void display_time(int x, int y) {
    getTeensy3Time();
    char string[13];  // print format stuff
    sprintf(string, "%02i:%02i:%02i", hour(), minute(), second());
    if (gpsHelper.validGPSdata) {
        tft.setTextColor(HX8357_GREEN, HX8357_BLACK);  // GPS-acquired UTC time
    } else {
        tft.setTextColor(HX8357_RED, HX8357_BLACK);  // Unknown zone and accuracy
    }
    // tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.print(string);
}

void display_date(int x, int y) {
    getTeensy3Time();
    char string[13];  // print format stuff
#if DISPLAY_DATE == MMDDYY
    sprintf(string, "%02i/%02i/%02i", month(), day(), year() % 1000);
#else
    sprintf(string, "%02i/%02i/%02i", year() % 1000, month(), day());
#endif
    if (gpsHelper.validGPSdata) {
        tft.setTextColor(HX8357_GREEN, HX8357_BLACK);  // GPS-acquired UTC date
        // strlcat(string, " UTC", sizeof(string));
    } else {
        tft.setTextColor(HX8357_RED, HX8357_BLACK);  // Unknown zone and accuracy
    }
    // tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.print(string);
}

/**
 *
 * Display informational messages in the OUTBOUND text region
 *
 * @param msg Message to be displayed
 *
 * Note:  Display must be initialized
 *
 **/
void displayInfoMsg(const char* msg) {
    displayInfoMsg(msg, HX8357_YELLOW);
}  // displayMsg()

/**
 *
 * Display informational messages in the OUTBOUND text region
 *
 * @param msg Message to be displayed
 * @param color Color of displayed message
 *
 * Note:  Display must be initialized
 *
 **/
void displayInfoMsg(const char* msg, uint16_t color) {
    char bfr[24];
    strlpad(bfr, msg, ' ', sizeof(bfr));
    tft.setTextColor(color, HX8357_BLACK);
    // tft.setTextSize(2);
    tft.setCursor(DISPLAY_OUTBOUND_X, DISPLAY_OUTBOUND_Y);
    tft.print(bfr);
}  // displayMsg()
