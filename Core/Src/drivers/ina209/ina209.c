/*
 *  INA209.c
 *	Drivers for INA209 Current Sense
 *	See Datasheet for possible register values
 *	https://www.ti.com/lit/ds/symlink/ina209.pdf
 *
 *	INA209 registers are Most Significant Byte first - LSB PEEPS BEWARE
 *	(MSB peeps just toss the _flip_byte_order() function)
 *  Created on: May 10, 2022
 *  Author: Liam Droog
 */

#include <ina209.h>
#include <I2C.h>


void _flip_byte_order();

void get_configuration(uint8_t addr, uint16_t *retval) {
    // POR is x399F
	i2c2_read8_16(addr, 0x00, retval);
    _flip_byte_order(retval);
    return;
}

void set_configuration(uint8_t addr, uint16_t *val) {
	_flip_byte_order(val);
    i2c2_write8_16(addr, 0x00, *val);
    return;
}

void get_status_flags(uint8_t addr, uint16_t *retval) {
    //	POR is x0000
	i2c2_read8_16(addr, 0x01, retval);
	_flip_byte_order(retval);
    return ;
}

// probably not needed; designs have alert pin grounded
void get_smbus_alert(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x02, retval);
	_flip_byte_order(retval);
	return;
}

void get_shunt_voltage(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x03, retval);
	_flip_byte_order(retval);
	return;
}

void get_bus_voltage(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x04, retval);
	_flip_byte_order(retval);
	return;
}

void get_power(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x05, retval);
	_flip_byte_order(retval);
	return;
}

// Current defaults to 0 on POR before calibration register (x16) is set
void get_current(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x06, retval);
	_flip_byte_order(retval);
	return;
}

void get_shunt_voltage_peak_pos(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x07, retval);
	_flip_byte_order(retval);
	return;
}

void get_shunt_voltage_peak_neg(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x08, retval);
	_flip_byte_order(retval);
    return;
}

void get_bus_voltage_peak_max(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x09, retval);
	_flip_byte_order(retval);
    return;
}

void get_bus_voltage_peak_min(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x0A, retval);
	_flip_byte_order(retval);
    return;
}

void get_power_peak(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x0B, retval);
	_flip_byte_order(retval);
    return;
}

void get_power_overlimit(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x11, retval);
	_flip_byte_order(retval);
    return;
}

void set_power_overlimit(uint8_t addr, uint16_t *val) {
	_flip_byte_order(val);
	i2c2_write8_16(addr, 0x11, *val);
    return;
}

void get_bus_voltage_overlimit(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x12, retval);
	_flip_byte_order(retval);
    return;
}

void set_bus_voltage_overlimit(uint8_t addr, uint16_t  *val) {
	_flip_byte_order(val);
	i2c2_write8_16(addr, 0x12, *val);
    return;
}

void get_bus_voltage_underlimit(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x13, retval);
	_flip_byte_order(retval);
    return;
}

void set_bus_voltage_underlimit(uint8_t addr, uint16_t  *val) {
	_flip_byte_order(val);
    i2c2_write8_16(addr, 0x13, *val);
    return;
}

void get_calibration(uint8_t addr, uint16_t *retval) {
	i2c2_read8_16(addr, 0x16, retval);
	_flip_byte_order(retval);
    return;
}

void set_calibration(uint8_t addr, uint16_t *val) {
	_flip_byte_order(val);
	i2c2_write8_16(addr, 0x16, *val);
    return;
}

void _flip_byte_order(uint16_t *input) {
    // Data is transmitted MSB first, but STM is LSB.
    // This flips the byte order.
    uint8_t rtn = 0x0000;
    uint8_t lsb = *input >> 8;
    uint8_t msb = *input & 0x00FF;
    rtn = msb << 8 | lsb;
    *input = rtn;
    return;
}
