#include <I2C.h>
#include <stdio.h>
#include "stm32l0xx_hal.h"
#include "arducam.h"
#include "debug.h"
#include "main.h"
extern I2C_HandleTypeDef hi2c2;

#define SCCB_READ 1

static uint16_t VIS_ADDRESS = 0x78;
static uint16_t NIR_ADDRESS = 0x7C;
// arducam functions
int wrSensorReg16_8(uint16_t regID, uint8_t regDat, uint8_t sensor) {
    uint8_t data = regDat;
    HAL_StatusTypeDef rc;
    if (sensor == VIS_SENSOR){
        rc = HAL_I2C_Mem_Write(&hi2c2, VIS_ADDRESS, regID, I2C_MEMADD_SIZE_16BIT, &data, 1, 100);
    }
    else{
        rc = HAL_I2C_Mem_Write(&hi2c2, NIR_ADDRESS, regID, I2C_MEMADD_SIZE_16BIT, &data, 1, 100);

    }
    if (rc != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C write to 0x%x failed: 0x%x\r\n", regID, rc);
    }
    else
        HAL_Delay(1);
    return (int) rc;
}

int wrSensorRegs16_8(const struct sensor_reg reglist[], uint8_t sensor) {
    const struct sensor_reg *curr = reglist;
    for (curr=reglist; curr->reg != 0xffff; curr++) {
        wrSensorReg16_8(curr->reg, curr->val, sensor);
    }
    return 0;
}

int rdSensorReg16_8(uint16_t regID, uint8_t *regDat, uint8_t sensor) {
    HAL_StatusTypeDef rc;
    if (sensor == VIS_SENSOR){
        rc = HAL_I2C_Mem_Read(&hi2c2, VIS_ADDRESS, regID, I2C_MEMADD_SIZE_16BIT, regDat, 1, 100);

    }
    else{
        rc = HAL_I2C_Mem_Read(&hi2c2, NIR_ADDRESS, regID, I2C_MEMADD_SIZE_16BIT, regDat, 1, 100);

    }
    if (rc != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C read xmit to 0x%x failed: 0x%x\r\n", regID, rc);
    }
    return (int) rc;
}

// I2C2 Functions
uint8_t i2c2_read16_8(uint8_t addr, uint16_t register_pointer){
//	uint8_t val = hi2c_read16_8(hi2c2, addr, register_pointer);
//	return val;
	HAL_StatusTypeDef status = HAL_OK;
	    uint16_t return_value = 0;
	    status = HAL_I2C_Mem_Read(&hi2c2, addr, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, &return_value, 1, 100);
	    return return_value;
}
void i2c2_write16_8(uint8_t addr, uint16_t register_pointer, uint16_t register_value){
	hi2c_write16_8(hi2c2, addr << 1, register_pointer, register_value);
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
// General functions
uint8_t hi2c_read16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer)
{
    uint8_t return_value = 0;
    HAL_I2C_Mem_Read(&hi2c, addr << 1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, &return_value, 1, 100);
    return return_value;
}

void hi2c_write16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint16_t register_value)
{
    uint8_t dataBuffer[1];
    dataBuffer[0] = register_value;
    HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, dataBuffer, 1, 100);
}

// UNTESTED BELOW
uint8_t hi2c_read8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer)
{
    uint16_t return_value = 0;

    HAL_I2C_Mem_Read(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, &return_value, 1, 100);
    return return_value;
}

void hi2c_write8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint8_t register_value)
{
    uint8_t dataBuffer[1];
    dataBuffer[0] = register_value;
    HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, dataBuffer, 1, 100);
}
