/*
 * SPI_IT.c
 *
 *	Interrupt driven SPI driver
 *
 *  Created on: May 9, 2022
 *      Author: liam
 */
#include "SPI_IT.h"

extern SPI_HandleTypeDef hspi1;

void SPI1_IT_Transmit(uint8_t *TX_Data){
	HAL_SPI_Transmit_IT(&hspi1, &TX_Data, sizeof(TX_Data));
	return;
}

void SPI1_IT_Recieve(uint8_t *RX_Data){
	HAL_SPI_Receive_IT(&hspi1, &RX_Data, sizeof(RX_Data));
	return;
}
