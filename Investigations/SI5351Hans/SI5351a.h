#pragma once

#include <inttypes.h>

#define SI_CLK0_CONTROL	16			// Register definitions
#define SI_CLK1_CONTROL	17
#define SI_CLK2_CONTROL	18
#define SI_SYNTH_PLL_A	26
#define SI_SYNTH_PLL_B	34
#define SI_SYNTH_MS_0		42
#define SI_SYNTH_MS_1		50
#define SI_SYNTH_MS_2		58
#define SI_PLL_RESET		177

#define SI_R_DIV_1		0b00000000			// R-division ratio definitions
#define SI_R_DIV_2		0b00010000
#define SI_R_DIV_4		0b00100000
#define SI_R_DIV_8		0b00110000
#define SI_R_DIV_16		0b01000000
#define SI_R_DIV_32		0b01010000
#define SI_R_DIV_64		0b01100000
#define SI_R_DIV_128		0b01110000

#define SI_CLK_SRC_PLL_A	0b00000000
#define SI_CLK_SRC_PLL_B	0b00100000

#define XTAL_FREQ	25000000			// Crystal frequency

#define SI_I2C_ADDR	0x60				//SI5351a I2C Address
#define NUM_CLOCKS 3					//SI5351a has 3 clock outputs


void si5351OutputOff(uint8_t clk);
void si5351ClockEnable(uint8_t,bool);
void si5351SetFrequency(uint32_t frequency);
void si5351SetFrequency(uint8_t clock, uint32_t frequency, int8_t q);
int  si5351Init(const char* busName, uint8_t addr, uint32_t fXtal, uint32_t cXtal);



