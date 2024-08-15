#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "SI5351a.h"

int main(void)
{
	//Initialize SI5351a on Raspberry Pi I2C bus 1
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("Starting...\n");
	int err = si5351Init("/dev/i2c-1",0x60,25000000,10);
	if (err<0) {
		printf("si5351Init failed\n");
		exit(-1);
	}

	//Start the Clk0 and Clk1 oscillators
	si5351SetFrequency(0, 10000000, 0);
	//si5351setFrequency(1, 1000000, 90);
	printf("Enabling clock(s)\n");

	si5351ClockEnable(0,true);	//Enable Clk0
	//si5351ClockEnable(1,true);



	return 0;
}

