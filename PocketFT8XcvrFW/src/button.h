/**
 * Defines button handles and functions
 */

#pragma once
#include <Arduino.h>

//-----Define button handles
#define BUTTON_CQ 0  // CQ
#define BUTTON_AB 1  // ABORT
#define BUTTON_TU 2  // TUNE
#define BUTTON_TX 3  // Reserved
#define BUTTON_M1 4  // Custom
#define BUTTON_M2 5  // Custom
#define BUTTON_M3 6  // Custom
#define BUTTON_M4 7  // Custom
#define BUTTON_SY 8  // Reserved

//-----Define button functions
int testButton(uint8_t index);
void drawButton(uint16_t i);
void checkButton(void);
void executeButton(uint16_t index);
void display_all_buttons(void);
void resetButton(uint16_t index);

//-----Define helper functions
void check_FT8_Touch(void);
void pollTouchscreen(void);
void check_WF_Touch(void);
// void set_startup_freq(void);
void terminate_transmit_armed(void);
void process_serial(void);
void EEPROMWriteInt(int address, int value);
int EEPROMReadInt(int address);

void store_encoders(void);

int EEPROMReadInt(int address);

void LPF_SendRegister(uint8_t reg, uint8_t val);
void LPF_init();
void LPF_write(uint16_t data);
void LPF_set_latch(uint8_t io, bool latch);
void LPF_set_lpf(uint8_t f);
