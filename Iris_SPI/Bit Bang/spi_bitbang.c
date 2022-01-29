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

#include "spi_bitbang.h"
#include "printf.h"
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

uint8_t read_byte(uint8_t addr){
	/*	Flow:
	 * 	Start
		Set the SS pin low to begin communication
		Set the pin for Master Out Slave In (MOSI) to the first bit of the data to be sent
		Set the clock pin (SCK) high so data is transmitted by the master and received by the slave
		Read the state of the Master in Slave Out (MISO) to receive the first bit of data from slave
		Set SCK Low, so data can be sent on the next rising edge
		Go to 2 till all data bits have been transmitted.
		Set the SS pin High to stop transmission.
		Stop
*/
	uint8_t rec;
	// CS Low
	_CS_LOW();
	// Send Phase
	for (int i=0; i<8; i++){
		HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read(addr, i));
		_CLK_HIGH();
		HAL_GPIO_ReadPin(MISO_Port, MISO_Pin);
		_CLK_LOW();
	}

	// Recieve phase
	for (int i=0; i<8; i++){
		HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read(0x00, i));
		_CLK_HIGH();
		if (HAL_GPIO_ReadPin(MISO_Port, MISO_Pin) == GPIO_PIN_SET){
			rec = rec << 1 | 0b1;
		}
		else{
			rec = rec << 1 | 0b0;
		}
		_CLK_LOW();
	}

	_CS_HIGH();
	return rec;
}

void write_byte(uint8_t addr, uint8_t byte){
	// CS Low
	_CS_LOW();
	// Send Phase
	for (int i=0; i<8; i++){
		HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read((addr | 0x80), i));
		_CLK_HIGH();
		HAL_GPIO_ReadPin(MISO_Port, MISO_Pin);
		_CLK_LOW();
	}

	// Write phase
	for (int i=0; i<8; i++){
		HAL_GPIO_WritePin(MOSI_Port, MOSI_Pin, bit_read(byte, i));
		_CLK_HIGH();
		HAL_GPIO_ReadPin(MISO_Port, MISO_Pin);
		_CLK_LOW();
	}

	_CS_HIGH();
}

GPIO_PinState bit_read(uint8_t byte, int j){
	byte = byte << j;
	if (byte & 0x80){
		return GPIO_PIN_SET;
	}
	return GPIO_PIN_RESET;
}
void _CS_LOW(){
	HAL_GPIO_WritePin(NSS_Port, NSS_Pin, GPIO_PIN_RESET);
}
void _CS_HIGH(){
	HAL_GPIO_WritePin(NSS_Port, NSS_Pin, GPIO_PIN_SET);
}

void _CLK_LOW(){
	HAL_GPIO_WritePin(CLK_Port, CLK_Pin, GPIO_PIN_RESET);
}
void _CLK_HIGH(){
	HAL_GPIO_WritePin(CLK_Port, CLK_Pin, GPIO_PIN_SET);
}



//void delay_us (TIM_HandleTypeDef timer, uint32_t nus)
//{
//  __HAL_TIM_SET_COUNTER(&timer,0);  // set the counter value a 0
//  while (__HAL_TIM_GET_COUNTER(&timer) < nus);  // wait for the counter to reach the us input in the parameter
//}







