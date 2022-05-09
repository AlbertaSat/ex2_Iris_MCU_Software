/*
 * command_handler.c
 *
 *  Created on: May 9, 2022
 *      Author: liam
 */
#include "command_handler.h"
/*
 * todo:
 * 		- Find a way to send sensors into idle mode without erasing regs
 * 		  otherwise save sensor regs somewhere in a struct.
 *		- Write functions to interface with sensor currently, but need to adapt to
 *		  work with Ron's NAND Flash stuff.
 *
*/
void take_image(){
	return;
}

void transfer_image(){
	/*
	 * Todo: this will be a bruh
	 * 			- Buffer images into 512 byte chunks
	 * 			- Transmission inludes 1 start byte + 512 image bytes + 1 end byte
	 * 				- total 514 bytes
	 * 			Flow:
	 * 				- Buffer 512 bytes of image in to memory
	 * 				- send header byte (not yet confirmed)
	 * 				- clock out 512 byte chunk
	 * 				- clock FF until next 512 byte chunk is ready
	 * 				- send footer byte (not yet confirmed)
	 * 				rinse and repeat.
	 *
	 */
	return;
}
void get_image_length(){
	// todo: translate 3 byte to 1 32 bit int and return that bish
	return;
}

void count_images(){
	//todo: Implement image counter
	return;
}

void sensor_idle(){
	// todo: send sensor idle command
	return;
}

void sensor_active(){
	// todo: send sensor active command
	return;
}

void get_housekeeping(){
	return;
}


void update_sensor_I2C_regs(){
	return;
}

void update_current_limits(){
	return;
}


