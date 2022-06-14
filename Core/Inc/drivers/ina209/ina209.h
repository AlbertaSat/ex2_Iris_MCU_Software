/*
 * ina231.h
 *
 *  Created on: May 10, 2022
 *      Author: liam
 */

#ifndef INC_INA209_H_
#define INC_INA209_H_
#include "main.h"
void get_configuration(uint8_t addr, uint16_t *retval);
void set_configuration(uint8_t addr, uint16_t *val);
void get_status_flags(uint8_t addr, uint16_t *retval);
void get_smbus_alert(uint8_t addr, uint16_t *retval);
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


#endif /* INC_INA209_H_ */
