#pragma once


  //Define the Sequencer State Machine's states.  The sequencer machine steps our station
  //through idling, initiating a contact with CQ, calling a known station, QSO-ing with
  //another station, or tuning.  This may be a bit different than some related designs so
  //pay attention.  ;)
  typedef enum {
    IDLE,  //IDLEing:  Not CQing, CALLing, QSOing nor TUNEing (just monitoring the traffic)

    //These are the "beacon mode" states arising when a QSO begins with our CQ
    CQ_PENDING,   //CQing:  Awaiting timeslot after CQ button click
    XMIT_CQ,      //CQing:  Transmitting our CQ
    LISTEN_LOC,   //CQing:  Listening for any response with locator
    XMIT_RSL,     //QSOing: Transmitting their RSL
    LISTEN_RRSL,  //QSOing: Listening for Roger and our RSL
    XMIT_RR73,    //QSOing: Transmitting RR73
    LISTEN_73,    //QSOing: Listening for their 73

    //These are the "QSO mode" states arising when we begin a QSO by calling a known (heard) station
    LOC_PENDING,  //CALLing: Awaiting timeslot after known station's message click
    XMIT_LOC,     //CALLing: Transmitting our locator in response to their CQ
    LISTEN_RSL,   //QSOing:  Listening for our RSL
    XMIT_RRSL,    //QSOing:  Transmitting Roger and their RSL
    LISTEN_RRR,   //QSOing:  Listen for their RRR/RR73/73
    XMIT_73,      //QSOing:  Transmitting 73

    //Tuning mode
    TUNING  //TUNEing:  Transmitting unmodulated carrier

  } SequencerStateType;
