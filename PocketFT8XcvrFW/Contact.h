/**
 * @brief Models a single contact between our station and a worked (remote, target) station
 *
**/
#pragma once

class Contact {

public:

  //Define the size of the char arrays recording the various fields
  static const int sizeCallsign = 12;  //11 chars and a NUL
  static const int sizeDate = 9;       //8 chars and a NUL
  static const int sizeTime = 7;       //6 chars and a NUL
  static const int sizeBand = 6;       //5 chars and a NUL
  static const int sizeMode = 4;       //3 chars and a NUL
  static const int sizeRSL = 4;        //3 chars and a NUL
  static const int sizeSOTAref = 14;   //14 chars and a NUL
  static const int sizeLocator = 7;    //6 chars and a NUL

  //Define the methods recording info about a contact
  Contact();                                                                    //Constructor
  void begin(char* workedCall, char* myCall, unsigned long freq, char* mode);    //Begin a new QSO
  bool isValid(void);                                                       //Determine if the current QSO, if any, is a valid (completed) contact
  void setWorkedRSL(char* rsl);
  void setMyRSL(char* rsl);
  void setMySOTAref(char* sotaRef);
  void setWorkedSOTAref(char* sotaRef);
  void setMyLocator(char* locator);
  void setWorkedLocator(char* locator);
  bool end();                                                               //End a QSO

  //Define the getters used to extract info about the contact
  char* getWorkedCall();
  char* getMyCall();
  char* getQSOdate();
  char* getQSOtime();
  char* getBand();
  char* getMode();
  char* getWorkedRSL();
  char* getMyRSL();
  char* getMySOTAref();
  char* getWorkedSOTAref();
  char* getMyLocator();
  char* getWorkedLocator();

private:

  //These are the required fields for a valid (completed) contact
  char workedCall[sizeCallsign];  //Remote station's callsign
  char myCall[sizeCallsign];      //Our station operator's callsign
  char qsoDate[sizeDate];         //UTC date (YYYYMMDD) when QSO began
  char qsoTime[sizeTime];         //UTC time (HHMMSS) when QSO began
  char band[sizeBand];            //ITU amateur band (meters)
  char mode[sizeMode];            //Modulation method (e.g. FT8)
  char workedRSL[sizeRSL];        //Remote station's Received Signal Level we reported to them
  char myRSL[sizeRSL];            //Our station's Received Signal Level reported to us

  //These are the optional fields (initialized as empty strings by default) that can be supplied if known
  char mySOTAref[sizeSOTAref];      //Our station's International SOTA Reference
  char workedSOTAref[sizeSOTAref];  //Worked statoin's International SOTA Reference
  char myLocator[sizeLocator];      //Our station's maidenhead locator (gridsquare)
  char workedLocator[sizeLocator];  //Worked station's maidenhead locator (gridsquare)
};