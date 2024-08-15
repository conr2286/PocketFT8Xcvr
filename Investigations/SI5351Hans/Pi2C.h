////////////////////////////////////////////////////////////////////////////////
// License:  This  program  is  free software; you can redistribute it and/or //
// modify  it  under the terms of the GNU General Public License as published //
// by  the  Free Software Foundation; either version 3 of the License, or (at //
// your  option)  any  later version. This program is distributed in the hope //
// that it will be useful, but WITHOUT ANY WARRANTY; without even the implied //
// warranty  of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the //
// GNU General Public License for more details.                               //
////////////////////////////////////////////////////////////////////////////////
// pi2c.h: This is the header file for the Pi2c class which allows for easy   //
// communication to an Arduino from the Raspberry Pi over the I2C bus.        //
// The default usage is for a Raspberry Pi Rev 1 - using the I2C bus          //
// "/dev/i2c-1".  Rev0 and the "/dev/i2c-0" bus can be specfied though if     //
// needed.                                                                    //
////////////////////////////////////////////////////////////////////////////////
// Example Usage:                                                             //
// Please see https://bitbucket.org/JohnnySheppard/pi2c for example usage.    //
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define BENCHTEST

#include <stdint.h>
#include <stddef.h>
#ifndef BENCHTEST
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#endif
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), */
#include <sys/ioctl.h>

class Pi2C {

private:
		int fd;							//Linux file descriptor accessing the I2C bus

public:
		Pi2C(const char *);					//Constructor
		~Pi2C();						//Destruction


		void sendRegister(uint8_t, uint8_t, uint8_t);
		void sendRegister(uint8_t dev, uint8_t reg, uint32_t count, uint8_t* c);

		uint8_t readRegister(uint8_t, uint8_t);
		void readRegister(uint8_t, uint8_t, uint8_t*);
		
		void delay(uint32_t);

};
