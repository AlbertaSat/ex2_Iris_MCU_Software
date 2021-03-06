/*
 * ina231.h
 *
 *  Created on: May 10, 2022
 *      Author: liam
 */

#ifndef INC_INA209_H_
#define INC_INA209_H_
#include <iris_system.h>

void get_configuration(uint8_t addr, uint16_t *retval);
void set_configuration(uint8_t addr, uint16_t *val);
void get_status_flags(uint8_t addr, uint16_t *retval);
void get_control_register(uint8_t addr, uint16_t *retval);
void set_control_register(uint8_t addr, uint16_t *val);
void get_shunt_voltage(uint8_t addr, uint16_t *retval);
void get_bus_voltage(uint8_t addr, uint16_t *retval);
void get_power(uint8_t addr, uint16_t *retval);
void get_current(uint8_t addr, uint16_t *retval);
void get_shunt_voltage_peak_pos(uint8_t addr, uint16_t *retval);
void get_shunt_voltage_peak_neg(uint8_t addr, uint16_t *retval);
void get_bus_voltage_peak_max(uint8_t addr, uint16_t *retval);
void get_bus_voltage_peak_min(uint8_t addr, uint16_t *retval);
void get_power_peak(uint8_t addr, uint16_t *retval);
void get_power_overlimit(uint8_t addr, uint16_t *retval);
void set_power_overlimit(uint8_t addr, uint16_t *val);
void get_bus_voltage_overlimit(uint8_t addr, uint16_t *retval);
void set_bus_voltage_overlimit(uint8_t addr, uint16_t *val);
void get_bus_voltage_underlimit(uint8_t addr, uint16_t *retval);
void set_bus_voltage_underlimit(uint8_t addr, uint16_t *val);
void get_calibration(uint8_t addr, uint16_t *retval);
void set_calibration(uint8_t addr, uint16_t *val);
void init_ina209(uint8_t addr);
void reset_ina209(uint8_t addr);

#endif /* INC_INA209_H_ */
