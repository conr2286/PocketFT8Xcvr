/*
 * SI5351RegBlock.cpp
 *
 *  Created on: Apr 10, 2023
 *      Author: jconrad
 */
#include <cstdlib>
#include <memory>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "Pi2CBlock.h"

/**
 * Construct an optimized register block for I2C device at addr
 *
 * @param i2C		Pi2C bus
 * @param addr		7-bit I2C address of device
 *
 * This class constructs an object working with Pi2C to optimally send a
 * sequence of registers to the I2C device at addr through its sendRegister()
 * method.  All reguests to sendRegister() on an block object must address the
 * same I2C device.  However, the sendRegister()s may target registers within
 * that device in any order.  The close() method must be called after all
 * requests are complete.  Close() will flush the registers' data to the
 * device in sorted order, using burst mode when the requests target registers
 * in sequential order, else their data will be flushed through individual
 * transactions.
 *
 */
Pi2CBlock::Pi2CBlock(Pi2C* i2C, uint8_t addr) {
	i2cDevice = addr;					//Remember the target device address
	count = 0;							//Reset count of <register,data> tuples
	first = NULL;						//Ordered list of <register,data> tuples
	i2c = i2C;
}




/**
 *  Buffer <reg,data> to later send to i2cDevice
 *
 *  @param reg	I2C device register
 *  @param data	Value to write to that register
 *
 *  All requests to sendRegister() address the same device in this object.
 *  Each request consists of a <reg,data> tuple and may appear in any order (they
 *  need not be sequential registers).  Even gaps in the register sequence are
 *  permitted.  The data is actually sent to the device after close().
 *
 *  Note:  all invocations of sendRegister() by this object address the same
 *  I2C device configured in the constructor.
 *
 *  Caution:  Register data is written to the I2C device in sequential (by reg
 *  number) order, not the invocation order of sendRegister().  While this can
 *  improve performance over the I2C bus (through use of burst mode writes),
 *  some devices may require their register values to be assigned in invocation
 *  order, not in sequential register order.
 */
void Pi2CBlock::sendRegister(uint8_t reg, uint8_t data) {

	printf("Pi2CBlock::sendRegister(%u,%u)\n",reg,data);

	//Build new <reg,data> tuple
	Pi2CRegData *newTuple = new (Pi2CRegData);
	newTuple->next = NULL;
	newTuple->reg = reg;		//close() will write this register...
	newTuple->data = data;		//with this data value.

	//Insert at head of list?
	if (count == 0) {
		first = newTuple;
		count = 1;
		printf("Inserted %u at head of list\n",reg);
		return;
	}

	//We don't actually send the register to the device until the reg block is closed.
	//All we do now is insert it into the ordered list of <reg,data> tuples.
	Pi2CRegData *curTuple = first;
	while (curTuple->next != NULL) {

		//Insert into midst of the list?
		if (newTuple->reg < curTuple->next->reg)
			break;

		//Not here, check the next
		curTuple = curTuple->next;

	}

	//Exited loop with curTuple referencing the tuple to proceed newTuple.
	if (curTuple->next==NULL) printf("Inserted %u at end of existing list\n",reg);
	else printf("Inserted %u into midst of existing list\n",reg);
	newTuple->next = curTuple->next;
	curTuple->next = newTuple;
	count++;

} //sendRegister()




/**
 *  Close a block of <reg,data> tuples
 *
 *  The <reg,data> tuples appear in an ordered (by reg number) list.  Note
 *  that the list may have gaps.  The idea is to use burst mode to write
 *  blocks of sequential (no gap) data to the I2C device.  If all the
 *  tuples are sequential, then a single burst write will service the
 *  entire list.  If there are gaps, then multiple writes will be required.
 *
 */
void Pi2CBlock::close() {

	printf("Pi2CBlock::close()\n");

	//Perhaps there isn't anything to send
	if (count==0) return;

	//Construct a buffer large enough to hold all of the data from the tuple list
	uint8_t* bfr = (uint8_t *)malloc(count);		//It's an array of bytes

	//Outer loop walks the list, assembling one block of sequential data into the buffer
	Pi2CRegData* curTuple = first;		//Begin examining list at first tuple
	printf("Closing reg block list\n");
	while(curTuple!=NULL) {

		//Start filling the buffer
		printf("Start filling bfr[] at reg %u\n",curTuple->reg);
		uint8_t firstReg = curTuple->reg;		//Register number of first tuple in bfr
		uint8_t nextReg  = firstReg+1;			//Number of next expected register following curTuple
		unsigned n = 0;							//Buffer is currently empty
		do {
			printf("Filling bfr[%u] for reg %u with %u\n",n,curTuple->reg,curTuple->data);
			bfr[n++] = curTuple->data;			//Buffer a byte of data from this tuple
			curTuple = curTuple->next;			//Now consider the following tuple if any
			if (curTuple == NULL) break;		//End of list?
		} while(curTuple->reg==nextReg++);		//No, does this following tuple belong in bufr?

		//Loop exited when bfr has n bytes of sequential data starting at firstReg
		i2c->sendRegister(i2cDevice,firstReg,n,bfr);	//Burst write bfr to firstReg in device addr

	} //Outer loop

	//Free the buffer after processing the entire tuple list
	free(bfr);
	printf("Closed bfr---------------------------------------\n");

} //close()






Pi2CBlock::~Pi2CBlock() {

	//Free the list of tuples
	Pi2CRegData *p = first;
	while (p!=NULL) {
		Pi2CRegData* next = p->next;
		free(p);
		p = next;
	}

}

