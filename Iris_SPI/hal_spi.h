/************************** HAL SPI Functions ***********************************

    Filename:    hal_spi.h
    Description: Implements SPI wrapper functions for use by low-level drivers.
                 Uses HAL library for STM32L0 series.

    Version:     0.1
    Author:      Liam Droog

********************************************************************************

    Version History.

    Ver.    Date            Comments

    0.1     Jan 2022        In Development

********************************************************************************

    The following functions are available in this library:


********************************************************************************/

#ifndef INC_HAL_SPI_H_
#define INC_HAL_SPI_H_

#include "stm32l0xx_hal.h"

// Chip select pin

#define SPI_NSS_Pin 	GPIO_PIN_15
#define SPI_NSS_Port 	GPIOA
#define DUMMY_BYTE 		0x00
#define TIMEOUT 		1000

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

    void CS_LOW(void);
    void CS_HIGH(void);

/******************************************************************************
 *                                  List of APIs
 *****************************************************************************/
    uint8_t SPI_Write_Byte(SPI_HandleTypeDef hspi, uint8_t addr, uint8_t data);
    uint8_t SPI_Read_Byte(SPI_HandleTypeDef hspi, uint8_t addr, uint8_t *data);


#endif /* INC_HAL_SPI_H_ */
