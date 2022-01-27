/************************** Flash Memory Driver ***********************************

    Filename:    I2C.h
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


#ifndef INC_I2C_H_
#define INC_I2C_H_
// Includes
#include "stm32l0xx_hal.h"

// TODO: I2C Register Parameters
//typedef struct  {
//	uint16_t reg;
//	uint16_t val;
//}sensor_reg;

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/


/******************************************************************************
 *                                  List of APIs
 *****************************************************************************/

    /* Wrapper functions for sending and receiving data */
	uint8_t hi2c_read_register(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer);
	void hi2c_write_register(I2C_HandleTypeDef hi2c, uint8_t addr, uint16_t register_pointer, uint16_t register_value);


/******************************************************************************/
#endif /* INC_I2C_H_ */
