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
#include <stdbool.h>

// Ram-face Arducam NSS (VIS)
#define NSS1_Pin GPIO_PIN_0
#define NSS1_Port GPIOA

// Anti-Ram-face Arducam NSS (NIR)
#define NSS2_Pin GPIO_PIN_1
#define NSS2_Port GPIOA

#define CLK_Pin GPIO_PIN_4
#define CLK_Port GPIOA

#define NSS1_Pin GPIO_PIN_0
#define NSS1_Port GPIOA

#define MOSI_Pin GPIO_PIN_2
#define MOSI_Port GPIOA

#define MISO_Pin GPIO_PIN_3
#define MISO_Port GPIOA


void _CS1_LOW();
void _CS1_HIGH();
void _CS2_LOW();
void _CS2_HIGH();
void _CLK_LOW();
void _CLK_HIGH();

uint8_t read_spi_reg(uint8_t addr, uint8_t sensor);
bool write_spi_reg(uint8_t addr, uint8_t packet, uint8_t sensor);
void spi_read_multiple_bytes(uint8_t addr, uint32_t length, uint8_t sensor);

GPIO_PinState bit_read(uint8_t byte, int j);
//void delay_us (uint32_t nus);
#endif /* INC_SPI_BITBANG_H_ */
