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

/*
 * Note on application of SPI burst readout from arducam sensor
 *
 * 		General flow consists of:
 *
 * 		spi_init_burst(sensor);
 * 		for (j=0; j<num_of_chunks; j++){
 *  		for (i=0; i<chunk_length; i++) {
 *      		imagebyte = spi_read_burst(sensor);
 *      	}
 *  	}
 * 		spi_deinit_burst(sensor);
 *
 * 		Important to make sure that deinit_burst gets called otherwise the CS will stay low.
 * 		Also don't toggle CS while doing a burst read - I think it will muck it up and you'll have to re-init
 * 		burst mode. ie: leave CS low until the entire image is read off.
 *
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
extern TIM_HandleTypeDef htim2;

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


/**
 * @brief reads spi register from target sensor
 *
 * @param addr      8 bit register address
 * @param sensor    target sensor
 * @return uint8_t
 * todo: consider switching to pointers (will require a decent amount of refactor)
 */
uint8_t read_spi_reg(uint8_t addr, uint8_t sensor) {
	uint8_t rec[8];
    // preallocate for teh z o o m
	uint8_t address[8];
	set_bits(address, addr);

    // CS Low
    if (sensor == 0x3C) {
        _CS1_LOW(); // VIS sensor is CS1
    } else {
        _CS2_LOW(); // NIR sensor is CS2
    }
    // Send Phase
	for (int i = 0; i < 8; i++) {
		if (address[i] == 1){
			MOSI_Port->BSRR = MOSI_Pin;
		}else{
			MOSI_Port->BRR = MOSI_Pin;
		}
		delay_us(SPI_DELAY);
		CLK_Port->BSRR = CLK_Pin;  // high
		if ((MISO_Port->IDR & MISO_Pin) != (uint32_t)GPIO_PIN_RESET)
		        {}
		delay_us(SPI_DELAY);
		CLK_Port->BRR = CLK_Pin;  // low
	}

    // Receive phase
    for (int i = 0; i < 8; i++) {
    	// write low regardless for dummy byte, this logic keeps the duty cycle for the clock
    	// at around 50% without having to mess with the delay_us();
    	if (address[i] == 1){
			MOSI_Port->BRR = MOSI_Pin;
		}else{
			MOSI_Port->BRR = MOSI_Pin;
		}
    	delay_us(SPI_DELAY);
        CLK_Port->BSRR = CLK_Pin;  // high
        if ((MISO_Port->IDR & MISO_Pin) != (uint32_t)GPIO_PIN_RESET)
        {
          rec[i] = 1;
        }else
        {
        	rec[i] = 0;
        }
        delay_us(SPI_DELAY);
        CLK_Port->BRR = CLK_Pin;  // low
    }

    if (sensor == 0x3C) {
        _CS1_HIGH();
    } else {
        _CS2_HIGH();
    }
	MOSI_Port->BRR = MOSI_Pin; // ensure mosi stays low

    return parse_bits(rec);
}

/**
 * @brief writes spi register on target sensor
 *
 * @param addr      target spi register
 * @param packet    value to write to register
 * @param sensor    target sensor
 * @return true
 * @return false
 */
bool write_spi_reg(uint8_t addr, uint8_t packet, uint8_t sensor) {
	// preallocate for teh z o o m
	uint8_t address[8];
	uint8_t to_send[8];
	set_bits(address, (addr|0x80));
	set_bits(to_send, packet);

    // CS Low
    if (sensor == 0x3C) {
        _CS1_LOW(); // VIS sensor is CS1
    } else {
        _CS2_LOW(); // NIR sensor is CS2
    }
    // Send Phase
    for (int i = 0; i < 8; i++) {
        if (address[i] == 1){
        	MOSI_Port->BSRR = MOSI_Pin;
        }else{
        	MOSI_Port->BRR = MOSI_Pin;
        }
        delay_us(SPI_DELAY);
        CLK_Port->BSRR = CLK_Pin;  // high
        // keep this here for timing sync? I think.
        if ((MISO_Port->IDR & MISO_Pin) != (uint32_t)GPIO_PIN_RESET)
                {}
        delay_us(SPI_DELAY);
        CLK_Port->BRR = CLK_Pin;  // low
    }

    // Write phase
    for (int i = 0; i < 8; i++) {
    	if (to_send[i] == 1){
			MOSI_Port->BSRR = MOSI_Pin;
		}else{
			MOSI_Port->BRR = MOSI_Pin;
		}
    	delay_us(SPI_DELAY);
    	CLK_Port->BSRR = CLK_Pin;  // high
        if ((MISO_Port->IDR & MISO_Pin) != (uint32_t)GPIO_PIN_RESET)
                {}
        delay_us(SPI_DELAY);
        CLK_Port->BRR = CLK_Pin;  // low
    }

    if (sensor == 0x3C) {
        _CS1_HIGH();
    } else {
        _CS2_HIGH();
    }
	MOSI_Port->BRR = MOSI_Pin; // ensure mosi stays low

    return true;
}

/**
 * @brief Reads byte in burst mode from target spi register
 *
 * @param sensor    target sensor
 */
uint8_t spi_read_burst(uint8_t sensor) {
  uint8_t rec[8];
	// Receive phase
	 for (int i = 0; i < 8; i++) {
		 // keeps clock duty cycle at roughly 50%
		if (rec[i] == 1){
			MOSI_Port->BRR = MOSI_Pin;
		}else{
			MOSI_Port->BRR = MOSI_Pin;
		}
		delay_us(SPI_DELAY);
		CLK_Port->BSRR = CLK_Pin;  // high
		if ((MISO_Port->IDR & MISO_Pin) != (uint32_t)GPIO_PIN_RESET)
		{
		  rec[i] = 1;
		}else
		{
			rec[i] = 0;
		}
		delay_us(SPI_DELAY);
		CLK_Port->BRR = CLK_Pin;  // low
	}
	return parse_bits(rec);
}

/*
 * Initializes burst mode on image sensor
 *
 * param:
 * 		sensor: target sensor
 */
void spi_init_burst(uint8_t sensor){
	uint8_t addr = 0x3C; // not i2c address dummy, spi reg x3C
	uint8_t address[8];
	set_bits(address, addr);

	if (sensor == 0x3C) {
		_CS1_LOW(); // VIS sensor is CS1
	} else {
		_CS2_LOW(); // NIR sensor is CS2
	}
	// Send Phase
	for (int i = 0; i < 8; i++) {
		if (address[i] == 1){
			MOSI_Port->BSRR = MOSI_Pin;
		}else{
			MOSI_Port->BRR = MOSI_Pin;
		}
		delay_us(SPI_DELAY);
		CLK_Port->BSRR = CLK_Pin;  // high
		if ((MISO_Port->IDR & MISO_Pin) != (uint32_t)GPIO_PIN_RESET)
		                {}
		delay_us(SPI_DELAY);
		CLK_Port->BRR = CLK_Pin;  // low
	}
	return;
}

/*
 * DeInitializes burst mode on sensor
 */
void spi_deinit_burst(uint8_t sensor) {
    if (sensor == 0x3C) {
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
uint8_t bit_read(uint8_t byte, int j) {
    byte = byte << j;
    if (byte & 0x80) {
        return 1;
    }
    return 0;
}

void set_bits(uint8_t *addr, uint8_t byte){
    for (int i=0; i<8; i++){
        addr[i] = bit_read(byte, i);
    }
}

uint8_t parse_bits(uint8_t *array){
    uint8_t ret = 0x00;
    for (int i=0; i<8; i++){
        if (array[i] == 1) {
            ret = ret << 0x01 | 0b1;
        } else {
            ret = ret << 0x01 | 0b0;
        }
	}
	return ret;
}
void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim2) < us);  // wait for the counter to reach the us input in the parameter
}

void _CS1_LOW() { HAL_GPIO_WritePin(NSS1_Port, NSS1_Pin, GPIO_PIN_RESET); }
void _CS1_HIGH() { HAL_GPIO_WritePin(NSS1_Port, NSS1_Pin, GPIO_PIN_SET); }

void _CS2_LOW() { HAL_GPIO_WritePin(NSS2_Port, NSS2_Pin, GPIO_PIN_RESET); }
void _CS2_HIGH() { HAL_GPIO_WritePin(NSS2_Port, NSS2_Pin, GPIO_PIN_SET); }

