/*
 * INA231.c
 *
 *  Created on: May 10, 2022
 *      Author: liam
 */

#include "ina231.h"

uint16_t get_configuration(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x00));
	return rtn;
}

void set_configuration(uint8_t addr, uint16_t val){
	i2c2_write8_16(addr, 0x00, _flip_byte_order(val));
	return;
}

uint16_t get_shunt_voltage(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x01));
	return rtn;
}

uint16_t get_bus_voltage(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x02));
	return rtn;
}

uint16_t get_power(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x03));
	return rtn;
}

uint16_t get_current(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x04));
	return rtn;
}

uint16_t get_calibration(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x05));
	return rtn;
}
void set_calibration(uint8_t addr, uint16_t val){
	i2c2_write8_16(addr, 0x05, _flip_byte_order(val));
	return;
}

uint16_t get_enable(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x06));
	return rtn;
}

void set_enable(uint8_t addr, uint16_t val){
	i2c2_write8_16(addr, 0x06, _flip_byte_order(val));
	return;
}

uint16_t get_alert_limit(uint8_t addr){
	uint16_t rtn = _flip_byte_order(i2c2_read8_16(addr, 0x07));
	return rtn;
}

void set_alert_limit(uint8_t addr, uint16_t val){
	i2c2_write8_16(addr, 0x07, _flip_byte_order(val));
	return;
}

static uint16_t _flip_byte_order(uint16_t input){
	// Data is transmitted MSB first, but STM is LSB. This flips the byte order.
	uint16_t rtn = 0x0000;
	uint8_t lsb = input >> 8;
	uint8_t msb = input & 0x00FF;
	rtn = msb << 8 | lsb;
	return rtn;

}


