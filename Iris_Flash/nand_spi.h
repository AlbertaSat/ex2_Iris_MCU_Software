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
 * @file nand_spi.h
 * @author Tharun Suresh
 * @date 2021-12-29
 * 
 * @brief SPI Wrapper Functions 
 * 
 * Implements SPI wrapper functions for use by low-level drivers.
 * Uses HAL library for STM32L0 series.
*/

#include "stm32l0xx_hal.h"

/******************************************************************************
 *          For reference only: NAND SPI HARDWARE SETTINGS
 *****************************************************************************/

#define NAND_SPI        SPI2

#define NAND_NWP_PIN    GPIO_PIN_6
#define NAND_NHOLD_PIN  GPIO_PIN_7
#define NAND_NCS_PIN    GPIO_PIN_12

#define NAND_NWP_PORT   GPIOB
#define NAND_NHOLD_PORT GPIOB
#define NAND_NCS_PORT   GPIOB

/******************************************************************************
 *                           Local Definitions
 *****************************************************************************/

#define DUMMY_BYTE         0x00
#define NAND_SPI_TIMEOUT   1000  /* max time for a transaction. nCS pulls high after */

/* using custom return type to keep higher layers as platform-agnostic as possible */
typedef enum {
    SPI_OK,
    SPI_Fail
} NAND_SPI_ReturnType;

/* SPI Transaction Parameters */
typedef struct {
    uint8_t *buffer;
    uint16_t length;
} SPI_Params;

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

    void __nand_spi_cs_low(void); 
    void __nand_spi_cs_high(void); 

/******************************************************************************
 *                                  List of APIs
 *****************************************************************************/
   
    /* General functions */
    void NAND_SPI_Init(SPI_HandleTypeDef *hspi);
    void NAND_Wait(uint8_t milliseconds);

    /* Wrapper functions for sending and receiving data */
    NAND_SPI_ReturnType NAND_SPI_Send(SPI_Params *data_send);
    NAND_SPI_ReturnType NAND_SPI_SendReceive(SPI_Params *data_send, SPI_Params *data_recv);
    NAND_SPI_ReturnType NAND_SPI_Receive(SPI_Params *data_recv);

    NAND_SPI_ReturnType NAND_SPI_Send_Command_Data(SPI_Params *cmd_send, SPI_Params *data_send);

/******************************************************************************/
