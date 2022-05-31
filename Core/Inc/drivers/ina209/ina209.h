/*
 * ina231.h
 *
 *  Created on: May 10, 2022
 *      Author: liam
 */

#ifndef INC_INA209_H_
#define INC_INA209_H_
#include "main.h"
uint16_t get_configuration(uint8_t addr);
void set_configuration(uint8_t addr, uint16_t val);
uint16_t get_status_flags(uint8_t addr);
uint16_t get_smbus_alert(uint8_t addr);
uint16_t get_shunt_voltage(uint8_t addr);
uint16_t get_bus_voltage(uint8_t addr);
uint16_t get_power(uint8_t addr);
uint16_t get_current(uint8_t addr);
uint16_t get_shunt_voltage_peak_pos(uint8_t addr);
uint16_t get_shunt_voltage_peak_neg(uint8_t addr);
uint16_t get_bus_voltage_peak_max(uint8_t addr);
uint16_t get_bus_voltage_peak_min(uint8_t addr);
uint16_t get_power_peak(uint8_t addr);
uint16_t get_power_overlimit(uint8_t addr);
void set_power_overlimit(uint8_t addr, uint16_t val);
uint16_t get_bus_voltage_overlimit(uint8_t addr);
void set_bus_voltage_overlimit(uint8_t addr, uint16_t val);
uint16_t get_bus_voltage_underlimit(uint8_t addr);
void set_bus_voltage_underlimit(uint8_t addr, uint16_t val);
uint16_t get_calibration(uint8_t addr);
void set_calibration(uint8_t addr, uint16_t val);
static inline uint16_t _flip_byte_order(uint16_t input){
	// Data is transmitted MSB first, but STM is LSB.
	// This flips the byte order.
	uint16_t rtn = 0x0000;
	uint8_t lsb = input >> 8;
	uint8_t msb = input & 0x00FF;
	rtn = msb << 8 | lsb;
	return rtn;
}
#endif /* INC_INA209_H_ */
