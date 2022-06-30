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
 * @file spi_bitbang.c
 * @author Liam Droog
 * @date 2021-12-29
 *
 * @brief Bit-Bang SPI Driver
 *
 * Contains functions for reading and writing via a GPIO bit-bang interface to an SPI slave
 *
 */

#include <stdbool.h>
#include "spi_bitbang.h"

// Pin config
//  /*Configure GPIO pins : SPI_B_MOSI_Pin SPI_B_CLK_Pin */
//  GPIO_InitStruct.Pin = SPI_B_MOSI_Pin|SPI_B_CLK_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /*Configure GPIO pins : SPI_B_MISO_Pin */
//  GPIO_InitStruct.Pin = SPI_B_MISO_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//  /*Configure GPIO pin : SPI2_CS_Pin */
//  GPIO_InitStruct.Pin = SPI2_CS_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_PULLUP;
//  GPIO_InitStrurct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(SPI2_CS_GPIO_Port, &GPIO_InitStruct);
//
//  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIOA, SPI2_CS_Pin, GPIO_PIN_RESET);
//
//  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIOB, SPI_B_MOSI_Pin|SPI_B_NSS_Pin|SPI_B_CLK_Pin, GPIO_PIN_RESET);

// todo burst mode!

/**
 * @brief reads spi register from target sensor
 *
 * @param addr      8 bit register address
 * @param sensor    target sensor
 * @return uint8_t
 * todo: consider switching to pointers (will require a decent amount of refactor)
 */
uint8_t read_spi_reg(uint8_t addr, uint8_t sensor) {
    uint8_t rec;
    // CS Low
    if (sensor == 0) {
        _CS1_LOW(); // VIS sensor is CS1
    } else {
        _CS2_LOW(); // NIR sensor is CS2
    }
    // Send Phase
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read(addr, i));
        _CLK_HIGH();
        HAL_GPIO_ReadPin(MISO_Port, MISO_Pin);
        _CLK_LOW();
    }

    // Recieve phase
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read(0x00, i));
        _CLK_HIGH();
        if (HAL_GPIO_ReadPin(MISO_Port, MISO_Pin) == GPIO_PIN_SET) {
            rec = rec << 1 | 0b1;
        } else {
            rec = rec << 1 | 0b0;
        }
        _CLK_LOW();
    }

    if (sensor == 0) {
        _CS1_HIGH();
    } else {
        _CS2_HIGH();
    }
    HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, GPIO_PIN_RESET);
    return rec;
}

/**
 * @brief writes spi register on target sensor
 *
 * @param addr      target spi register
 * @param packet    value to write to register
 * @param sensor    target sensor
 * @return true
 * @return false
 * todo: switch to pointers; stop using bools
 */
bool write_spi_reg(uint8_t addr, uint8_t packet, uint8_t sensor) {
    // CS Low
    if (sensor == 0) {
        _CS1_LOW(); // VIS sensor is CS1
    } else {
        _CS2_LOW(); // NIR sensor is CS2
    }
    // Send Phase
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read((addr | 0x80), i));
        _CLK_HIGH();
        HAL_GPIO_ReadPin(MISO_Port, MISO_Pin);
        _CLK_LOW();
    }

    // Write phase
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read(packet, i));
        _CLK_HIGH();
        HAL_GPIO_ReadPin(MISO_Port, MISO_Pin);
        _CLK_LOW();
    }

    if (sensor == 0) {
        _CS1_HIGH();
    } else {
        _CS2_HIGH();
    }
    HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, GPIO_PIN_RESET);

    return true;
}

/**
 * @brief Reads multiple bytes from target spi register
 *
 * @param addr      target register address
 * @param length    number of bytes to read
 * @param sensor    target sensor
 */
uint8_t spi_read_burst(uint8_t sensor) {
    uint8_t rec;
	// Receive phase
	HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, GPIO_PIN_RESET);
	for (int i = 0; i < 8; i++) {
		_CLK_HIGH();
		if (HAL_GPIO_ReadPin(MISO_Port, MISO_Pin) == GPIO_PIN_SET) {
			rec = rec << 1 | 0b1;
		} else {
			rec = rec << 1 | 0b0;
		}
		_CLK_LOW();
	}
	return rec;
}

void spi_init_burst(uint8_t sensor){
	if (sensor == 0) {
		_CS1_LOW(); // VIS sensor is CS1
	} else {
		_CS2_LOW(); // NIR sensor is CS2
	}
	uint8_t addr = 0x3C;
	// Send Phase
	    for (int i = 0; i < 8; i++) {
	        HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read(addr, i));
	        _CLK_HIGH();
	        HAL_GPIO_ReadPin(MISO_Port, MISO_Pin);
	        _CLK_LOW();
	    }
	return;
}

void spi_deinit_burst(uint8_t sensor){
    if (sensor == 0) {
        _CS1_HIGH();
    } else {
        _CS2_HIGH();
    }
	return;
}
/**
 * @brief Gets state of j'th bit in a byte
 *
 * @param byte  byte to read
 * @param j     position of bit
 * @return      GPIO_PinState
 */
GPIO_PinState bit_read(uint8_t byte, int j) {
    byte = byte << j;
    if (byte & 0x80) {
        return GPIO_PIN_SET;
    }
    return GPIO_PIN_RESET;
}

void _CS1_LOW() { HAL_GPIO_WritePin(NSS1_Port, NSS1_Pin, GPIO_PIN_RESET); }
void _CS1_HIGH() { HAL_GPIO_WritePin(NSS1_Port, NSS1_Pin, GPIO_PIN_SET); }

void _CS2_LOW() { HAL_GPIO_WritePin(NSS2_Port, NSS2_Pin, GPIO_PIN_RESET); }
void _CS2_HIGH() { HAL_GPIO_WritePin(NSS2_Port, NSS2_Pin, GPIO_PIN_SET); }

void _CLK_LOW() { HAL_GPIO_WritePin(CLK_Port, CLK_Pin, GPIO_PIN_RESET); }
void _CLK_HIGH() { HAL_GPIO_WritePin(CLK_Port, CLK_Pin, GPIO_PIN_SET); }
