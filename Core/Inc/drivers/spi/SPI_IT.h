/*
 * SPI_IT.h
 *
 *  Created on: May 9, 2022
 *      Author: liam
 */

#ifndef INC_SPI_IT_H_
#define INC_SPI_IT_H_
#include <iris_system.h>

void SPI1_IT_Transmit(uint8_t *TX_Data);
void SPI1_IT_Recieve(uint8_t *RX_Data);

#endif /* INC_SPI_IT_H_ */
