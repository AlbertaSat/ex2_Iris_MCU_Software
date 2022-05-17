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
uint16_t get_shunt_voltage(uint8_t addr);
uint16_t get_bus_voltage(uint8_t addr);
uint16_t get_power(uint8_t addr);
uint16_t get_current(uint8_t addr);
uint16_t get_calibration(uint8_t addr);
void set_calibration(uint8_t addr, uint16_t val);
uint16_t get_enable(uint8_t addr);
void set_enable(uint8_t addr, uint16_t val);
uint16_t get_alert_limit(uint8_t addr);
void set_alert_limit(uint8_t addr, uint16_t val);
static uint16_t _flip_byte_order(uint16_t input);
#endif /* INC_INA209_H_ */
