/*
 * tmp421.c
 *
 *  Created on: Mar. 24, 2022
 *      Author: Liam Droog
 */
#include "tmp421.h"
#include "debug.h"

// add reset function to the stuff n things
uint16_t get_temp(uint8_t sensor_addr){
	// returns a 16 bit unsigned integer with bits [15:8] as the 'high' byte,
	// and the bits [7:4] bits as the 'low' byte. High byte is the integer value with a -64 celsius offset
	// Low byte is [7:4] with 0.0625 celsius per count. Temp is the sum of the high and low byte.
	uint8_t highbyte = i2c2_read8_8(sensor_addr, 0x00);
	uint8_t lowbyte = i2c2_read8_8(sensor_addr, 0x10);
	return ((uint16_t)highbyte << 8) | lowbyte;
}

void init_temp_sensors(void){
	// change bit 2 of config 1 to set to extended binary
	// and make life easier.
	i2c2_write8_8(VIS_TEMP_SENSOR, 0x09, 0x04);
	i2c2_write8_8(NIR_TEMP_SENSOR, 0x09, 0x04);
	i2c2_write8_8(TEMP3, 0x09, 0x04);
	i2c2_write8_8(TEMP4, 0x09, 0x04);
}


