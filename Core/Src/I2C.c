#include <I2C.h>
#include <stdio.h>
#include "stm32l0xx_hal.h"
#include "arducam.h"
#include "debug.h"
#include "main.h"
extern I2C_HandleTypeDef hi2c2;

#define SCCB_READ 1

static uint16_t VIS_ADDRESS = 0x3C;
static uint16_t NIR_ADDRESS = 0x3E;

// arducam functions
void wrSensorReg16_8(uint16_t regID, uint8_t regDat, uint8_t sensor) {
    if (sensor == VIS_SENSOR){
    	i2c2_write16_8(VIS_ADDRESS, regID, regDat);
    }
    else{
    	i2c2_write16_8(NIR_ADDRESS, regID, regDat);
    }
    HAL_Delay(1);
}

void wrSensorRegs16_8(const struct sensor_reg reglist[], uint8_t sensor) {
    const struct sensor_reg *curr = reglist;
    for (curr=reglist; curr->reg != 0xffff; curr++) {
        wrSensorReg16_8(curr->reg, curr->val, sensor);
    }
    return;
}

void rdSensorReg16_8(uint16_t regID, uint8_t *regDat, uint8_t sensor) {
    if (sensor == VIS_SENSOR){
//        rc = HAL_I2C_Mem_Read(&hi2c2, VIS_ADDRESS, regID, I2C_MEMADD_SIZE_16BIT, regDat, 1, 100);
    	i2c2_read16_8(VIS_ADDRESS, regID, regDat);
    }
    else{
//        rc = HAL_I2C_Mem_Read(&hi2c2, NIR_ADDRESS, regID, I2C_MEMADD_SIZE_16BIT, regDat, 1, 100);
    	i2c2_read16_8(NIR_ADDRESS, regID, regDat);

    }
    return;
}

// I2C2 Functions
void i2c2_read16_8(uint8_t addr, uint16_t register_pointer, uint8_t *reg_data){
	// todo generalize
	hi2c_read16_8(hi2c2, addr, register_pointer, reg_data);
	return;
}
void i2c2_write16_8(uint8_t addr, uint16_t register_pointer, uint16_t register_value){
	hi2c_write16_8(hi2c2, addr, register_pointer, register_value);
	return;
}

uint8_t i2c2_read8_8(uint8_t addr, uint8_t register_pointer){
	uint8_t val = hi2c_read8_8(hi2c2, addr, register_pointer);
	return val;
}
void i2c2_write8_8(uint8_t addr, uint8_t register_pointer, uint8_t register_value){
	hi2c_write8_8(hi2c2, addr, register_pointer, register_value);
	return;
}

//untested
uint16_t i2c2_read8_16(uint8_t addr, uint8_t register_pointer){
	uint16_t val = hi2c_read8_16(hi2c2, addr, register_pointer);
	return val;
}
//untested
void i2c2_write8_16(uint8_t addr, uint8_t register_pointer, uint16_t register_value){
	hi2c_write8_16(hi2c2, addr, register_pointer, register_value);
	return;
}

// General functions
void hi2c_read16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint8_t *reg_data)
{
    	HAL_StatusTypeDef rc;
    	rc = HAL_I2C_Mem_Read(&hi2c, addr<<1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, reg_data, 1, 100);
        if (rc != HAL_OK) {
            char buf[64];
            sprintf(buf, "I2C read xmit to 0x%x failed: 0x%x\r\n", register_pointer, rc);
        }
    return;
}

void hi2c_write16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint16_t register_value)
{
    uint8_t dataBuffer[1];
    HAL_StatusTypeDef status = HAL_OK;
    dataBuffer[0] = register_value;
    status = HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, dataBuffer, 1, 100);
    if (status != HAL_OK) {
            char buf[64];
            sprintf(buf, "I2C16_8 write to 0x%x register 0x%x failed\r\n", addr, register_pointer);
            DBG_PUT(buf);
        }
}

// UNTESTED BELOW
uint8_t hi2c_read8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer)
{
    uint16_t return_value = 0;
	HAL_StatusTypeDef status = HAL_OK;
    status = HAL_I2C_Mem_Read(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, &return_value, 1, 100);
    if (status != HAL_OK) {
            char buf[64];
            sprintf(buf, "I2C8_8 read from 0x%x register 0x%x failed\r\n", addr, register_pointer);
            DBG_PUT(buf);
        }
    return return_value;
}

void hi2c_write8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint8_t register_value)
{
    uint8_t dataBuffer[1];
	HAL_StatusTypeDef status = HAL_OK;
    dataBuffer[0] = register_value;
    status = HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, dataBuffer, 1, 100);
    if (status != HAL_OK) {
            char buf[64];
            sprintf(buf, "I2C8_8 write to 0x%x failed: 0x%x\r\n", addr, register_pointer);
            DBG_PUT(buf);
        }
}

uint16_t hi2c_read8_16(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer)
{
    uint16_t return_value = 0;
	HAL_StatusTypeDef status = HAL_OK;
    status = HAL_I2C_Mem_Read(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, &return_value, 2, 100);
    if (status != HAL_OK) {
            char buf[64];
            sprintf(buf, "I2C8_16 read from 0x%x register 0x%x failed\r\n", addr, register_pointer);
            DBG_PUT(buf);
        }
    return return_value;
}

void hi2c_write8_16(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint16_t register_value)
{
    uint16_t dataBuffer[1];
	HAL_StatusTypeDef status = HAL_OK;
    dataBuffer[0] = register_value;
    status = HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, dataBuffer, 2, 100);
    if (status != HAL_OK) {
            char buf[64];
            sprintf(buf, "I2C8_16 write to 0x%x failed: 0x%x\r\n", addr, register_pointer);
            DBG_PUT(buf);
        }
}
