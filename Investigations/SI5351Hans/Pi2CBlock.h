/*
 * SI5351RegBlock.h
 *
 *  Created on: Apr 10, 2023
 *      Author: jconrad
 */

#include <stdint.h>
#include "Pi2C.h"

#pragma once

struct Pi2CRegData {
	Pi2CRegData*	next;				//Link to next element of list
	uint8_t			reg;				//Target register
	uint8_t			data;				//Byte to send to that register
};

class Pi2CBlock {
	uint8_t i2cDevice;					//7-bit I2C device address
	Pi2CRegData* first;					//Ordered list of <reg,data> tuples
	unsigned	count;					//Number of elements in list of <reg,data> tuples
	Pi2C* 		i2c;					//The I2C bus

public:
	Pi2CBlock(Pi2C*, uint8_t);
	virtual ~Pi2CBlock();
	void sendRegister(uint8_t,uint8_t);
	void close();

};


