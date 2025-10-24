/*
 * decode_ft8.h
 *
 *  Created on: Nov 2, 2019
 *      Author: user
 */
#pragma once

#include <Arduino.h>

#include "msgTypes.h"

int ft8_decode(void);

static const String sp = String(" ");
// typedef struct
class Decode {
   public:
    Decode() : freq_hz(0), sync_score(0), snr(0), distance(0), msgType(MSG_UNKNOWN), sequenceNumber(0) {
        field1[0] = field2[0] = field3[0] = locator[0] = decode_time[0] = 0;
    }  // Decode()

    // String toString(void) { return String(field1) + sp + String(field2) + sp + String(field3); }
    String toString(void) {
        String result = field1;
        result += sp;
        result += field2;
        result += sp;
        result += field3;
        return result;
    }

    char field1[14];               // Their station's call
    char field2[14];               // Our station's call
    char field3[7];                // Extra info
    char locator[7];               // Their locator if we have it
    int freq_hz;                   //
    char decode_time[10];          // Timestamp when message successfully decoded
    int sync_score;                //
    int snr;                       // Their received signal level
    int distance;                  // KM between their and our station
    MsgType msgType;               // Type of received message (e.g. CQ, LOC, RSL...)
    unsigned long sequenceNumber;  // Sequencer's timeslot sequenceNumber when msg was received
} /*Decode*/;

typedef struct
{
    char decode_time[10];
    char call[7];

} Calling_Station;

typedef struct
{
    char decode_time[10];
    char call[7];
    int distance;
    int snr;
    int freq_hz;
} CQ_Station;

Decode* getNewDecoded(void);

void save_Answer_CQ_List(void);

void display_Answer_CQ_Items(void);

int Check_Calling_Stations(int num_decoded);
void clear_CQ_List_box(void);
// void display_details(int decoded_messages);
void display_messages(int decoded_messages);
void clear_display_details(void);
int Check_CQ_Calling_Stations(int num_decoded, int reply_state);
int Check_QSO_Calling_Stations(int num_decoded, int reply_state);
void Check_CQ_Stations(int num_decoded);

void display_selected_call(int index);
void process_selected_CQ(void);

char rsl2s(int rsl);
