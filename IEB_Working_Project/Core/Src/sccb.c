#include <stdio.h>
#include "stm32l0xx_hal.h"
#include "arducam.h"
#include "debug.h"
#include "sccb.h"
#include "main.h"
extern I2C_HandleTypeDef hi2c2;

#define SCCB_READ 1

static uint16_t VIS_ADDRESS = 0x78;
static uint16_t NIR_ADDRESS = 0x7C;

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
