/************************** HAL SPI Functions ***********************************

    Filename:    hal_spi.c
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
#include "hal_spi.h"

// SPI_HandleTypeDef hspi_nand;

/**
	@brief Initialize SPI bus to NAND IC.
	@note For reference only. Not to be called.
*/
//static void SPI_Init(void)
//{
//
//  /* USER CODE BEGIN SPI1_Init 0 */
//
//  /* USER CODE END SPI1_Init 0 */
//
//  /* USER CODE BEGIN SPI1_Init 1 */
//
//  /* USER CODE END SPI1_Init 1 */
//  /* SPI1 parameter configuration*/
//  hspi.Instance = SPI1;
//  hspi.Init.Mode = SPI_MODE_MASTER;
//  hspi.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi.Init.NSS = SPI_NSS_SOFT;
//  hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
//  hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi.Init.TIMode = SPI_TIMODE_DISABLE;
//  hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//  hspi.Init.CRCPolynomial = 7;
//}

/**
 	@brief Write byte to specified SPI slave address
*/
uint8_t SPI_Write_Byte(SPI_HandleTypeDef hspi, uint8_t addr, uint8_t data){
	CS_LOW();
	uint8_t txBuf[2] = {(addr | 0x80), data};
	uint8_t status = (HAL_SPI_Transmit(&hspi, txBuf, 2, TIMEOUT) == HAL_OK);
	CS_HIGH();
	return status;
}

/**
 	@brief Read byte from specified SPI slave address
*/
uint8_t SPI_Read_Byte(SPI_HandleTypeDef hspi, uint8_t addr, uint8_t *data) {
	CS_LOW();
	uint8_t dataBuffer[2] = {(addr), DUMMY_BYTE};
	uint8_t rxBuf[1];
	rxBuf[0] = DUMMY_BYTE;
	HAL_SPI_Transmit(&hspi, dataBuffer, 1, TIMEOUT);
	uint8_t status = HAL_SPI_Receive(&hspi, rxBuf, 1, TIMEOUT);
	*data = rxBuf[0];
	CS_HIGH();
	return status;
}

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

/**
 	@brief Enable SPI communication to NAND by pulling chip select pin low.
    @note Must be called prior to every SPI transmission
*/
void CS_LOW(){
	HAL_GPIO_WritePin(SPI_NSS_Port, SPI_NSS_Pin, GPIO_PIN_RESET);
}


/**
 	 @brief Close SPI communication to NAND by pulling chip select pin high.
   	 @note Must be called after every SPI transmission
*/
void CS_HIGH(){
	HAL_GPIO_WritePin(SPI_NSS_Port, SPI_NSS_Pin, GPIO_PIN_SET);
}
