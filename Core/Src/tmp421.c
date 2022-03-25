/*
 * tmp421.c
 *
 *  Created on: Mar. 24, 2022
 *      Author: Liam Droog
 */
#include "tmp421.h"

uint8_t tempSensorList[num_temp_sensors] = {
		0x4C
};
// change bit 2 of config 1 to set to extended binary and make life easier. Need to initalize them prior tho
// add initialization function for all sensors
// add reset function to the stuff n things
uint16_t get_temp(uint8_t sensor_addr){
	// returns a 16 bit unsigned integer with bits [15:8] as the 'high' byte,
	// and the bits [7:4] bits as the 'low' byte. High byte is the integer value with a -64 celsius offset
	// Low byte is [7:4] with 0.0625 celsius per count. Temp is the sum of the high and low byte.
	uint8_t highbyte = i2c2_read8_8(VIS_TEMP_SENSOR, 0x00);
	uint8_t lowbyte = i2c2_read8_8(VIS_TEMP_SENSOR, 0x10);
	return ((uint16_t)highbyte << 8) | lowbyte;
}

void init_temp_sensors(void){
	for (uint8_t i; i<num_temp_sensors; i++){
		uint8_t tmp = i2c2_read8_8(tempSensorList[i], 0x09);
		uint8_t flip = tmp | 0b100; // flip bit 2 to activate extended temp range
		i2c2_write8_8(tempSensorList[i], 0x09, flip);
	}
	return;
}


