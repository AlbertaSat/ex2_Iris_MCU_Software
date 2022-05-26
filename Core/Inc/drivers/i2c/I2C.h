/*
 * sccb.h
 *
 *  Created on: Feb 7, 2022
 *      Author: Liam
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_
#include "stm32l0xx_hal.h"
#include "arducam.h"
void wrSensorReg16_8(uint16_t regID, uint8_t regDat, uint8_t sensor);

void wrSensorRegs16_8(const struct sensor_reg reglist[], uint8_t sensor);

void rdSensorReg16_8(uint16_t regID, uint8_t *regDat, uint8_t sensor);

void i2c2_read16_8(uint8_t addr, uint16_t register_pointer, uint8_t *reg_data);
void i2c2_write16_8(uint8_t addr, uint16_t register_pointer, uint16_t register_value);

uint8_t i2c2_read8_8(uint8_t addr, uint8_t register_pointer);
void i2c2_write8_8(uint8_t addr, uint8_t register_pointer, uint8_t register_value);

void hi2c_read16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint8_t *reg_data);
void hi2c_write16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint16_t register_value);

uint8_t hi2c_read8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer);
void hi2c_write8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint8_t register_value);

uint16_t i2c2_read8_16(uint8_t addr, uint8_t register_pointer);
void i2c2_write8_16(uint8_t addr, uint8_t register_pointer, uint16_t register_value);
uint16_t hi2c_read8_16(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer);
void hi2c_write8_16(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint16_t register_value);


#endif /* INC_I2C_H_ */
