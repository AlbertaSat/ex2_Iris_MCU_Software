/*
 * Copyright (C) 2015  University of Alberta
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/**
 * @file nand_spi.c
 * @author Tharun Suresh
 * @date 2021-12-29
 * 
 * @brief SPI Wrapper Functions 
 * 
 * Implements SPI wrapper functions for use by low-level drivers.
 * Uses HAL library for STM32L0 series.
*/

#include "nand_spi.h"

/******************************************************************************
 *                              Initialization
 *****************************************************************************/

static SPI_HandleTypeDef *hspi_nand;

// /**
//  * @brief Initialize SPI bus to NAND IC.
//  * @note For reference only. Not to be called.
//  */
// void NAND_SPI_Init(void) {

//     /* NAND SPI parameter configuration*/
//     hspi_nand.Instance = NAND_SPI;
//     hspi_nand.Init.Mode = SPI_MODE_MASTER;
//     hspi_nand.Init.Direction = SPI_DIRECTION_2LINES;
//     hspi_nand.Init.DataSize = SPI_DATASIZE_8BIT;
//     hspi_nand.Init.CLKPolarity = SPI_POLARITY_LOW;
//     hspi_nand.Init.CLKPhase = SPI_PHASE_1EDGE;
//     hspi_nand.Init.NSS = SPI_NSS_SOFT;
//     hspi_nand.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
//     hspi_nand.Init.FirstBit = SPI_FIRSTBIT_MSB;
//     hspi_nand.Init.TIMode = SPI_TIMODE_DISABLE;
//     hspi_nand.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//     hspi_nand.Init.CRCPolynomial = 7;

//     HAL_SPI_Init(hspi_nand);
// };

// /**
//  * @brief Initialize GPIO pins connected to NAND IC.
//  * @note For reference only. Not to be called.
//  */
// void NAND_GPIO_Init(void){

//     GPIO_InitTypeDef NAND_GPIO_InitStruct = {0};

//     HAL_GPIO_WritePin(NAND_NWP_PORT, NAND_NWP_PIN, GPIO_PIN_SET); // active low
//     HAL_GPIO_WritePin(NAND_NHOLD_PORT, NAND_NHOLD_PIN, GPIO_PIN_SET); // active low
//     HAL_GPIO_WritePin(NAND_NCS_PORT, NAND_NCS_PIN, GPIO_PIN_SET); // active low

//     NAND_GPIO_InitStruct.Pin = NAND_NWP_PIN;
//     NAND_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//     NAND_GPIO_InitStruct.Pull = GPIO_NOPULL;
//     NAND_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     HAL_GPIO_Init(NAND_NWP_PORT, &NAND_GPIO_InitStruct);

//     NAND_GPIO_InitStruct.Pin = NAND_NHOLD_PIN;
//     NAND_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//     NAND_GPIO_InitStruct.Pull = GPIO_NOPULL;
//     NAND_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     HAL_GPIO_Init(NAND_NHOLD_PORT, &NAND_GPIO_InitStruct);

//     NAND_GPIO_InitStruct.Pin = NAND_NCS_PIN;
//     NAND_GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//     NAND_GPIO_InitStruct.Pull = GPIO_NOPULL;
//     NAND_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     HAL_GPIO_Init(NAND_NCS_PORT, &NAND_GPIO_InitStruct);

// };

/**
 * @brief Stores the HAL SPI Handle for use in all subsequent transactions
 * 
 * @param[in] hspi 
 */
void NAND_SPI_Init(SPI_HandleTypeDef *hspi) {
    hspi_nand = hspi;
};

/**
 * @brief Calls HAL_Delay() for stated number of milliseconds
 * 
 * @param[in] milliseconds Number of milliseconds to delay
 */
void NAND_Wait(uint8_t milliseconds){
    HAL_Delay(milliseconds);
};


/******************************************************************************
 *                     Send & Receive Complete Transactions 
 *****************************************************************************/

/**
 * @brief Write data to NAND.
 * 
 * @param data_send[in]    Pointer to struct with data and length
 * @return NAND_SPI_ReturnType 
 */
NAND_SPI_ReturnType NAND_SPI_Send(SPI_Params *data_send) {
    HAL_StatusTypeDef send_status;

    __nand_spi_cs_low();
    send_status = HAL_SPI_Transmit(hspi_nand, data_send->buffer, data_send->length, NAND_SPI_TIMEOUT);
    __nand_spi_cs_high();

    if (send_status != HAL_OK) {
        return SPI_Fail; 
    } else {
        return SPI_OK;
    }

};

/**
 * @brief Send and receive data from NAND in one transaction.
 * 
 * @param data_send[in]    Pointer to struct with sending data buffer and length of buffer
 * @param data_recv[out]    Pointer to struct with receive data buffer and length of buffer
 * @return NAND_SPI_ReturnType 
 */
NAND_SPI_ReturnType NAND_SPI_SendReceive(SPI_Params *data_send, SPI_Params *data_recv) {
    HAL_StatusTypeDef recv_status;

    __nand_spi_cs_low();
    HAL_SPI_Transmit(hspi_nand, data_send->buffer, data_send->length, NAND_SPI_TIMEOUT);
    recv_status = HAL_SPI_Receive(hspi_nand, data_recv->buffer, data_recv->length, NAND_SPI_TIMEOUT);
    __nand_spi_cs_high();

    if (recv_status != HAL_OK) {
        return SPI_Fail; 
    } else {
        return SPI_OK;
    }
};

/**
 * @brief Read data from NAND.
 * 
 * @param data_recv[in]    Pointer to struct with read data buffer and length of receive data
 * @return NAND_SPI_ReturnType 
 */
NAND_SPI_ReturnType NAND_SPI_Receive(SPI_Params *data_recv) {
    HAL_StatusTypeDef receive_status;

    __nand_spi_cs_low();
    receive_status = HAL_SPI_Receive(hspi_nand, data_recv->buffer, data_recv->length, NAND_SPI_TIMEOUT);
    __nand_spi_cs_high();

    if (receive_status != HAL_OK) {
        return SPI_Fail;
    } else {
        return SPI_OK;
    }
};

/******************************************************************************
 *                  Send command and data in one transaction  
 *****************************************************************************/

/**
 * @brief Send a command and associated data to NAND in one transaction
 * 
 * @param cmd_send[in]     Pointer to struct with command and length of command
 * @param data_send[in]    Pointer to struct with data and length of data
 * @return NAND_SPI_ReturnType 
 */
NAND_SPI_ReturnType NAND_SPI_Send_Command_Data(SPI_Params *cmd_send, SPI_Params *data_send) {
    HAL_StatusTypeDef send_status;

    __nand_spi_cs_low();
    HAL_SPI_Transmit(hspi_nand, cmd_send->buffer, cmd_send->length, NAND_SPI_TIMEOUT);
    send_status = HAL_SPI_Transmit(hspi_nand, data_send->buffer, data_send->length, NAND_SPI_TIMEOUT);
    __nand_spi_cs_high();

    if (send_status != HAL_OK) {
        return SPI_Fail; 
    } else {
        return SPI_OK;
    }
};

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

/**
 * @brief Enable SPI communication to NAND by pulling chip select pin low.
 * @note Must be called prior to every SPI transmission
 */
void __nand_spi_cs_low(void) {
    HAL_GPIO_WritePin(NAND_NCS_PORT, NAND_NCS_PIN, GPIO_PIN_RESET);
};

/**
 * @brief Close SPI communication to NAND by pulling chip select pin high.
 * @note Must be called after every SPI transmission
 */
void __nand_spi_cs_high(void) {
    HAL_GPIO_WritePin(NAND_NCS_PORT, NAND_NCS_PIN, GPIO_PIN_SET);
};