/************************** Flash Memory Driver ***********************************

    Filename:    I2C.c
    Description: I2C Drivers for Iris Electronics

    Version:     0.1
    Author:      Liam Droog

********************************************************************************

    Version History.

    Ver.        Date            Comments

    0.1        Jan 2022         In Development

********************************************************************************

    The following functions are available in this library:


********************************************************************************/

#include "I2C.h"


uint8_t hi2c_read_register(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t return_value = 0;

    status = HAL_I2C_Mem_Read(&hi2c, addr, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, &return_value, 1, 100);

    /* Check the communication status */
//      if(status != HAL_OK)
//      {
//          // Error handling, for example re-initialization of the I2C peripheral
//      	printf("I2C read from 0x%x failed...\r\n", register_pointer );
//      }
//      else{
//      	printf("I2C read from 0x%x was successful!\r\n", register_pointer );
//      }
    return return_value;
}

void hi2c_write_register(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint16_t register_value)
{
    HAL_StatusTypeDef status;
    uint8_t dataBuffer[1];
    dataBuffer[0] = register_value;
    status = HAL_I2C_Mem_Write(&hi2c, addr, (uint16_t)register_pointer, I2C_MEMADD_SIZE_16BIT, dataBuffer, 1, 100);

//      /* Check the communication status */
//      if(status != HAL_OK)
//      {
//          // Error handling, for example re-initialization of the I2C peripheral
//      	printf("I2C write to 0x%x failed...\r\n", register_pointer );
//      }
//      else{
//      	printf("I2C write was successful\r\n");
//      }
}
