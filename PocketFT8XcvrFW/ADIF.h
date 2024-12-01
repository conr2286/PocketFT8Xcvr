





class ADIF {

  private:
  char* myCall;
  char* theirCall;
  char* myGridsquare;
  char* theirGridsquare;
  char* qsoMode;
  char* qsoFreq;
  char* mySOTARef;          //My SOTA Reference
  char* mySIGinfo;          //My (POTA) Special Interest Group info
  char* logFile;            //Filename on SD disk

  public:
  ADIF(char* logFile, char* myCall, char* myGridsquare, char* qsoMode, char* mySOTARef, char* mySIGinfo);
  int logQSO(char* theirCall, char* theirGridsquare, char* date, char* time, char* myRSL, char* theirRSL);

};