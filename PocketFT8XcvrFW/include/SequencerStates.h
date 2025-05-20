#pragma once

// Define the Sequencer State Machine's states.  The sequencer machine steps our station
// through idling, initiating a contact with CQ, calling a known station, QSO-ing with
// another station, or tuning.  This may be a bit different than some related designs so
// pay attention.  ;)
typedef enum {
    IDLE = 0,  // IDLEing:  Not CQing, CALLing, QSOing nor TUNEing (just monitoring the traffic)

    // These are the "beacon mode" states arising when a QSO begins with our CQ
    CQ_PENDING = 1,   // CQing:  Awaiting timeslot after CQ button click
    XMIT_CQ = 2,      // CQing:  Transmitting our CQ
    LISTEN_LOC = 3,   // CQing:  Listening for any response with locator
    RSL_PENDING = 4,  // QSOing: We are awaiting an appropriate timeslot to transmit an RSL
    XMIT_RSL = 5,     // QSOing: Transmitting their RSL
    LISTEN_RRSL = 6,  // QSOing: Listening for Roger and our RSL
    RRR_PENDING = 7,  // QSOing: We are awaiting an appropriate timeslot to transmit our RR73
    XMIT_RRR = 8,     // QSOing: Transmitting RR73
    LISTEN_73 = 9,    // QSOing: Listening for their 73

    // These are the "QSO mode" states arising when we begin a QSO by calling a known (heard) station
    LOC_PENDING = 10,   // CALLing: Awaiting timeslot after known station's message click
    XMIT_LOC = 11,      // CALLing: Transmitting our locator in response to their CQ
    LISTEN_RSL = 12,    // QSOing:  Listening for our RSL
    RRSL_PENDING = 13,  // QSOing:  Awaiting timeslot to transmit RRSL (e.g. R-12)
    XMIT_RRSL = 14,     // QSOing:  Transmitting Roger and their RSL
    LISTEN_RRR = 15,    // QSOing:  Listen for their RRR/RR73/73
    M73_PENDING = 16,   // QSOing:  Awaiting timeslot to transmit 73
    XMIT_73 = 17,       // QSOing:  Transmitting 73

    // Tuning mode
    TUNING = 18  // TUNEing:  Transmitting unmodulated carrier

} SequencerStateType;
