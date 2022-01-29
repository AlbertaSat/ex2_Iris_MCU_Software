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
 * @file spi_bitbang.h
 * @author Liam Droog
 * @date 2021-12-29
 *
 * @brief Bit-Bang SPI Driver
 *
 * Contains functions for reading and writing via a GPIO bit-bang interface to an SPI slave
 *
 */
#ifndef INC_SPI_BITBANG_H_
#define INC_SPI_BITBANG_H_
#include "stm32l0xx_hal.h"

#define NSS_Pin GPIO_PIN_6
#define NSS_Port GPIOB

#define CLK_Pin GPIO_PIN_7
#define CLK_Port GPIOB

#define MOSI_Pin GPIO_PIN_14
#define MOSI_Port GPIOB

#define MISO_Pin GPIO_PIN_4
#define MISO_Port GPIOA


void _CS_LOW();
void _CS_HIGH();
void _CLK_LOW();
void _CLK_HIGH();

uint8_t read_byte(uint8_t byte);
void write_byte(uint8_t addr, uint8_t byte);

GPIO_PinState bit_read(uint8_t byte, int j);
//void delay_us (uint32_t nus);
#endif /* INC_SPI_BITBANG_H_ */
