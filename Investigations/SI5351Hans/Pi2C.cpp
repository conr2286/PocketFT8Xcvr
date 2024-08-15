/**
 *
 * Pi2C --- An interface to a an I2C device on the Raspberry Pi
 *
 * @author
 * 	Jim Conrad, KQ7B		Implementation for yasdr
 *
 * 	Pi2C constructs an interface to a Raspberry Pi I2C bus (the RPi 4 supports
 * 	several busses).  You will need one Pi2C object for each bus in use, but
 * 	one object can access any device on its bus.
 *
 * 	The constructor prepares the bus for access, after which you
 * 	may use the read/write methods to access device(s) on that bus.
 *
 * @section NOTES
 *  This implementation of the Pi2C class interface was originally developed for
 *  use with Raspberry Pi (RPi) 4 hardware supervised by bullseye and tested with
 *  the SI5351a chip.
 *
 *  The RPi supports several ways to access an I2C device.  The open/read/write
 *  approach binds a file descriptor to a specific device and lacks support for
 *  the I2C "Repeated START" (e.g. START, write register, START, read byte)
 *  operation.  The smbus approach supports "Repeated START" but binds the file
 *  descriptor to a single device rather than the bus.  The ioctl I2C_RDWR
 *  approach used here binds the open file descriptor (and hence a Pi2C object)
 *  to a bus, not a specific device, and also supports the I2C "Repeated START"
 *  feature required to read a specific register within an I2C device.
 *
 *  This implementation is currently limited to 7-bit device addressing.
 *
 *  This implementation is currently limited to operating as the bus Master
 *  on the Raspberry Pi.
 *
 *  Error handling could probably use some improvement, but given the target
 *  usage in an embedded systems... what can really be done beyond rebooting
 *  and hoping things go better in the next life.
 *
 * 	@section REFERENCES
 * 	https://pimylifeup.com/raspberry-pi-i2c/
 * 	https://www.kernel.org/doc/Documentation/i2c/dev-interface
 * 	https://www.kernel.org/doc/Documentation/i2c/i2c-protocol
 *	https://learn.adafruit.com/working-with-i2c-devices/repeated-start
 *	https://android.googlesource.com/kernel/msm/+/f5335159eed416b26b7c8a5a4e8820f97dc1ad19/Documentation/i2c/dev-interface
 *	https://gist.github.com/randhawp/29728350fef0c51f17eaff8b920bf5c8
 *
 *
 * @section MIT LICENSE
 * Copyright (c) <2023> <Jim Conrad, KQ7B>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **/

//Bench test and debugging options
#define BENCHTEST                   //For bench test w/o an SI5351 on I2C
#define DEBUG                       //Enables DPRINTF output
#include "debug.h"

//Now we can deal with the usual includes
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdexcept>
#include <sys/types.h>

#ifndef BENCHTEST
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#endif

#include "Pi2C.h"

/**
 * Pi2c Constructor
 *
 * @param busName 	Linux filename of the desired I2C bus
 * @param addr		The 7-bit I2C device address
 * @throw errno
 *
 * Usage:  Pi2C("/dev/i2c-1",0x60);			//Device 0x60 on RPi I2C bus 1 (GPIO 2 and 3)
 *
 * @section NOTES
 * At this time, Pi2C joins the bus only as a controller, not as a slave.
 * At this time, the instantiated object is configured to access a single device on the
 * I2C bus.  Because I2C supports many devices on a single bus, we may someday implement
 * another constructor (sans the addr parameter) and another method for selecting an
 * I2C device address.
 *
 */
Pi2C::Pi2C(const char *busName) {

	DPRINTF("Pi2C(%s)\n",busName);
    
#ifndef BENCHTEST

	//Open the I2C bus
	fd = open(busName, O_RDWR); //Open the i2c file descriptor in read/write mode
	if (fd < 0) {
		throw errno;			//Whoopsie!
	}

	DPRINTF("Pi2C.fd=%d\n",fd);
#endif

} //I2C()

//Destructor merely closes the file descriptor resource
Pi2C::~Pi2C() {
#ifndef BENCHTEST
	close(fd); 					//Make the fd available for other activities
#endif
} //~

/**
 * Send one byte to an I2C device register over the bus accessed through the fd member
 *
 * @param dev		7-bit address of the I2C device on this bus
 * @param reg		Target register
 * @param c			Byte to be written
 * @throw			errno
 *
 * @section NOTES
  *
 */
void Pi2C::sendRegister(uint8_t dev, uint8_t reg, uint8_t c) {

	DPRINTF("Pi2C::sendRegister(0x%x,%u,%u)\n",dev,reg,c);

#ifndef BENCHTEST

	uint8_t bfr[2];					//Bytes to send to dev

	struct i2c_msg msg[1];//I2C_RDWR parameters for a write to register operation
	struct i2c_rdwr_ioctl_data req[1];	//I2C_RDWR request packet

	//Build buffer of data to transmit to the device
	bfr[0] = reg;					//Selects the destination register within the device
	bfr[1] = c;						//Byte to send to that register

	//Build ioctl's I2C_RDWR parameters to do a single write operation with 7-bit addressing
	msg[0].addr = dev;			//Specify which slave device to access
	msg[0].flags = 0;			//Write using 7-bit addressing
	msg[0].len = 2;				//Send 2 bytes (register number and data) to dev
	msg[0].buf = bfr;			//Pointer to the data buffer containing those bytes

	//Build the ioctl I2C_RDWR request to send <reg,c> to dev on I2C bus accessed thru fd
	req[0].msgs = msg;
	req[0].nmsgs = 1;
	if (ioctl(fd, I2C_RDWR, &req) < 0) {
		throw errno;
	}
#endif

} //sendRegister()





/**
 * Send count bytes to I2C device register(s) over the bus accessed through the fd member
 *
 * @param dev		7-bit address of the I2C device on this bus
 * @param reg		Target register
 * @param count		Number of bytes to send
 * @param c			Pointer to first byte in an array of bytes to send
 * @throw			errno
 *
 * Note:  The target I2C device must support burst data transfer with auto register
 * address increments in order to accept a block of data from us.
 *
 */
void Pi2C::sendRegister(uint8_t dev, uint8_t reg, uint32_t count, uint8_t* c) {
    
    DPRINTF("Pi2C::sendRegister(0x%x,%u,%u,{",dev,reg,count);
    uint8_t* pc=c;
    for(unsigned di=0;di<count;di++) DPRINTF(" %u",*pc++);
    DPRINTF(" })\n");
    
#ifndef BENCHTEST
    
    uint8_t* bfr = (uint8_t *)malloc(count+1);	//Buffer to combine reg and data
    
    struct i2c_msg msgs;			//I2C_RDWR parameters for a write to register operation
    struct i2c_rdwr_ioctl_data req;	//I2C_RDWR request packet
    
    //Build bfr containing the destination register followed by count bytes of data
    bfr[0] = reg;					//Selects the destination register within the device
    for(unsigned i=1; i<=count; i++) bfr[i] = *c++; //The actual data
    msgs.addr = dev;
    msgs.flags = 0;
    msgs.len = count+1;
    msgs.buf = bfr;
    
    //Build the ioctl I2C_RDWR request to send <reg,c> to dev on I2C bus accessed thru fd
    req.msgs = &msgs;			//ioctl parameter packet
    req.nmsgs = 1;				//Send one chunk to device
    if (ioctl(fd, I2C_RDWR, &req) < 0) {
        free(bfr);
        throw errno;
    }
    free(bfr);
#endif
    DPRINTF("return from sendRegister\n");
    
} //sendRegister()





/**
 * Read one byte from a designated register in an I2C device on this bus
 *
 * @param dev		Read from this device
 * @param reg		Target register
 * @param pBfr		Pointer to a one byte buffer to receive the data
 * @throw errno
 *
 */
void Pi2C::readRegister(uint8_t dev, uint8_t reg, uint8_t *pBfr) {
    
#ifndef BENCHTEST
	uint8_t outBfr[1];							//Buffers for selecting the register and reading result
	struct i2c_msg params[2];					//Parameters for the I2C_RDWR request
	struct i2c_rdwr_ioctl_data req[1];			//The I2C_RDWR request

	//Initialize outBfr to select the requested register in device
	outBfr[0] = reg;							//Send register number *to* device

	//Build ioctl I2C_RDWR parameters selecting a register to read from device
	params[0].addr = dev;						//Specify slave device address
	params[0].flags = 0;						//Write
	params[0].len = 1;							//Send a single byte (register number)
	params[0].buf = outBfr;						//Buffer containing the register number

	//Build ioctl I2C_RDWR parameters to read from the previously selected register in device
	params[1].addr = dev;						//Specify slave device address
	params[1].flags = I2C_M_RD | I2C_M_NOSTART;	//Then read
	params[1].len = 1;							//One byte
	params[1].buf = pBfr;						//Place the read data in user's buffer

	//The ioctl I2C_RDWR transaction
	req[0].msgs = params;						//Parameters for combined write/read operation
	req[0].nmsgs = 2;							//Two parts to this transaction
	if (ioctl(fd, I2C_RDWR, &req) < 0) {
		throw errno;
	}

#else
    *pBfr = 0x00;                   //Can't really read non-existing device
#endif
    

} //readRegister()




/**
 * Return one byte from a designated register in the I2C device accessed through the fd member
 *
 * @param dev		Read from this I2C device
 * @param reg		Target register in that device
 * @throw errno
 *
 */
uint8_t Pi2C::readRegister(uint8_t dev, uint8_t reg) {
	uint8_t c;
	readRegister(dev, reg, &c);
	DPRINTF("Pi2C::readRegister(%u,%u) returns 0x%x\n",dev, reg, c);
	return(c);
} //readRegister()





/**
 * Delay execution of the calling thread
 *
 * @param mSec		Number of milliseconds for execution to be delayed
 * @throw errno
 *
 * @section NOTES
 * This method doesn't actually access the I2C bus, but it's common for I2C devices to
 * require a delay following an issued command.  So... it's here to simplify programming I2C
 * devices and to emphasize the purpose of the delay has to do with an I2C device.
 */
void Pi2C::delay(uint32_t mSec) {

	if (usleep(mSec * 1000L) < 0) {
		throw errno;
	}

} //delay()

