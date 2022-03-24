/*
 * tmp421.c
 *
 *  Created on: Mar. 24, 2022
 *      Author: Liam Droog
 */
#include "tmp421.h"

// change bit 2 of config 1 to set to extended binary and make life easier. Need to initalize them prior tho
// add initialization function for all sensors
// add reset function to the stuff n things
uint16_t get_temp(uint8_t sensor_addr){
	// returns a 16 bit unsigned integer with bits [15:8] as the 'high' byte,
	// and the bits [7:4] bits as the 'low' byte. High byte is
	uint8_t highbyte = i2c2_read8_8(VIS_TEMP_SENSOR, 0x00);
	uint8_t lowbyte = i2c2_read8_8(VIS_TEMP_SENSOR, 0x10);
	return ((uint16_t)highbyte << 8) | lowbyte;
}
