#include <iris_system.h>
#include <I2C.h>
#include <stdio.h>
#include "stm32l0xx_hal.h"
#include "arducam.h"
#include "debug.h"
extern I2C_HandleTypeDef hi2c2;

#define SCCB_READ 1
// arducam functions
/**
 * @brief Writes Arducam Sensor i2c register
 *
 * @param regID target register ID; 16 bit
 * @param regDat register data to write; 8 bit
 * @param sensor target sensor
 */
void wrSensorReg16_8(uint16_t regID, uint8_t regDat, uint8_t sensor) {
    i2c2_write16_8(sensor, regID, regDat);
    HAL_Delay(1);
}

/**
 * @brief Writes a struct of (register, value) to a target sensor
 *
 * @param reglist sensor_reg struct containing registers and values to write to sensor
 * @param sensor  target sensor
 */
void wrSensorRegs16_8(struct sensor_reg reglist[], uint8_t sensor) {
    struct sensor_reg *curr = reglist;
    for (curr = reglist; curr->reg != 0xffff; curr++) {
        wrSensorReg16_8(curr->reg, curr->val, sensor);
    }
    return;
}

/**
 * @brief reads sensor reg
 *
 * @param regID target register ID; 16 bit
 * @param regDat register data; 8 bit
 * @param sensor target sensor
 */
void rdSensorReg16_8(uint16_t regID, uint8_t *regDat, uint8_t sensor) {
    i2c2_read16_8(sensor, regID, regDat);
    return;
}

// I2C2 Functions
/**
 * @brief reads 16 bit register with 8 bit value on i2c bus 2 (internal)
 *
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 16 bit register value
 * @param reg_data          pointer to where to write the register data
 */
void i2c2_read16_8(uint8_t addr, uint16_t register_pointer, uint8_t *reg_data) {
    hi2c_read16_8(hi2c2, addr, register_pointer, reg_data);
    return;
}

/**
 * @brief writes 16 bit register with 8 bit value on i2c bus 2 (internal)
 *
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 16 bit register value
 * @param register_value    8 bit value to write to register
 */
void i2c2_write16_8(uint8_t addr, uint16_t register_pointer, uint8_t register_value) {
    hi2c_write16_8(hi2c2, addr, register_pointer, register_value);
    return;
}

/**
 * @brief reads 8 bit register with 8 bit value on i2c bus 2 (internal)
 *
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 8 bit register value
 * @param reg_data          pointer to where to write the register data
 */
void i2c2_read8_8(uint8_t addr, uint8_t register_pointer, uint8_t *reg_data) {
    hi2c_read8_8(hi2c2, addr, register_pointer, reg_data);
    return;
}

/**
 * @brief writes 8 bit register with 8 bit value on i2c bus 2 (internal)
 *
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 8 bit register value
 * @param register_value    8 bit value to write to register
 */
void i2c2_write8_8(uint8_t addr, uint8_t register_pointer, uint8_t register_value) {
    hi2c_write8_8(hi2c2, addr, register_pointer, register_value);
    return;
}

/**
 * @brief reads 8 bit register with 16 bit value on i2c bus 2 (internal)
 *
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 16 bit register value
 * @param register_value    pointer to 16 bit value
 */
void i2c2_read8_16(uint8_t addr, uint8_t register_pointer, uint16_t *reg_data) {
    hi2c_read8_16(hi2c2, addr, register_pointer, reg_data);
    return;
}

/**
 * @brief writes 8 bit register with 16 bit value on i2c bus 2 (internal)
 *
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 8 bit register value
 * @param register_value    16 bit value to write to register
 */
void i2c2_write8_16(uint8_t addr, uint8_t register_pointer, uint16_t register_value) {
    hi2c_write8_16(hi2c2, addr, register_pointer, register_value);
    return;
}

/**
 * @brief General function to read 8 bit value from 16 bit register via I2C
 *
 * @param hi2c              I2C_HandleTypeDef structure
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 16 bit register address
 * @param reg_data          pointer to 8 bit register value
 */
void hi2c_read16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint8_t *reg_data) {
    HAL_StatusTypeDef rc;
    rc = HAL_I2C_Mem_Read(&hi2c, addr << 1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, reg_data, 1, 100);
    if (rc != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C read xmit to 0x%x failed: 0x%x\r\n", register_pointer, rc);
    }
    return;
}

/**
 * @brief General function to write 8 bit value to 16 bit register via I2C
 *
 * @param hi2c              I2C_HandleTypeDef structure
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 16 bit register address
 * @param register_value    8 bit register value
 */
void hi2c_write16_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint16_t register_value) {
    uint8_t dataBuffer[1];
    HAL_StatusTypeDef status;
    dataBuffer[0] = register_value;
    status =
        HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, dataBuffer, 1, 100);
    if (status != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C16_8 write to 0x%x register 0x%x failed\r\n", addr, register_pointer);
        iris_log(buf);
    }
}

/**
 * @brief General function to read 8 bit value from 8 bit register via I2C
 *
 * @param hi2c              I2C_HandleTypeDef structure
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 8 bit register address
 * @param reg_data          pointer to 8 bit register value
 */
void hi2c_read8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint8_t *reg_data) {
    HAL_StatusTypeDef status = HAL_OK;
    status = HAL_I2C_Mem_Read(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, reg_data, 1, 100);
    if (status != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C8_8 read from 0x%x register 0x%x failed\r\n", addr, register_pointer);
        iris_log(buf);
    }
}

/**
 * @brief General function to write 8 bit value to 8 bit register via I2C
 *
 * @param hi2c              I2C_HandleTypeDef structure
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 8 bit register address
 * @param register_value    pointer to 8 bit register value
 */
void hi2c_write8_8(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint8_t register_value) {
    uint8_t dataBuffer[1];
    HAL_StatusTypeDef status;
    dataBuffer[0] = register_value;
    status =
        HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, dataBuffer, 1, 100);
    if (status != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C8_8 write to 0x%x failed: 0x%x\r\n", addr, register_pointer);
        iris_log(buf);
    }
}

/**
 * @brief General function to read 16 bit value from 8 bit register via I2C
 *
 * @param hi2c              I2C_HandleTypeDef structure
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 8 bit register address
 * @param reg_data          pointer to 16 bit register value
 */
void hi2c_read8_16(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint16_t *reg_data) {
    HAL_StatusTypeDef status = HAL_OK;
    status = HAL_I2C_Mem_Read(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, reg_data, 2, 100);
    if (status != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C8_16 read from 0x%x register 0x%x failed\r\n", addr, register_pointer);
        iris_log(buf);
    }
    return;
}

/**
 * @brief General function to write 16 bit value to 8 bit register via I2C
 *
 * @param hi2c              I2C_HandleTypeDef structure
 * @param addr              7 bit device address
 * @param register_pointer  pointer to 8 bit register address
 * @param register_value    pointer to 16 bit register value
 */
void hi2c_write8_16(I2C_HandleTypeDef hi2c, uint8_t addr, uint8_t register_pointer, uint16_t register_value) {
    uint16_t dataBuffer[1];
    HAL_StatusTypeDef status;
    dataBuffer[0] = register_value;
    status =
        HAL_I2C_Mem_Write(&hi2c, addr << 1, (uint8_t)register_pointer, I2C_MEMADD_SIZE_8BIT, dataBuffer, 2, 100);
    if (status != HAL_OK) {
        char buf[64];
        sprintf(buf, "I2C8_16 write to 0x%x failed: 0x%x\r\n", addr, register_pointer);
        iris_log(buf);
    }
}
