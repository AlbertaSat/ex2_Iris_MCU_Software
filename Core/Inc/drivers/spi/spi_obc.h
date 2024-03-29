/*
 * spi_obc.h
 *
 *  Created on: Jul 21, 2022
 *      Author: jenish
 */

#ifndef INC_DRIVERS_SPI_SPI_OBC_H_
#define INC_DRIVERS_SPI_SPI_OBC_H_

#include <iris_system.h>

HAL_StatusTypeDef obc_spi_transmit(uint8_t *tx_data, uint16_t data_length);
HAL_StatusTypeDef obc_spi_receive(uint8_t *rx_data, uint16_t data_length);
HAL_StatusTypeDef obc_spi_receive_blocking(uint8_t *rx_data, uint16_t data_length);
void obc_enable_spi_rx();
void obc_disable_spi_rx();

#endif /* INC_DRIVERS_SPI_SPI_OBC_H_ */
