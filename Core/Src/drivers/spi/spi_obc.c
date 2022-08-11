/*
 * spi_obc.c
 *
 *  Created on: Jul 21, 2022
 *      Author: jenish
 */

#include <iris_system.h>

extern SPI_HandleTypeDef hspi1;

/**
 * @brief
 * 		Transmit data of given size over SPI bus in blocking mode
 * @param
 * 		*tx_data: pointer to transmit data
 * 		data_length: numbers of bytes to be sent
 * @return
 * 		HAL level return status
 */
HAL_StatusTypeDef obc_spi_transmit(uint8_t *tx_data, uint16_t data_length) {
    return HAL_SPI_Transmit(&hspi1, tx_data, data_length, HAL_MAX_DELAY);
}

/**
 * @brief
 * 		Receive data of given size over SPI bus in interrupt mode
 * @param
 * 		*rx_data: pointer to receive data
 * 		data_length: numbers of bytes to be receive
 * @return
 * 		HAL level return status
 */
HAL_StatusTypeDef obc_spi_receive(uint8_t *rx_data, uint16_t data_length) {
    return HAL_SPI_Receive_IT(&hspi1, rx_data, data_length);
}

/**
 * @brief
 * 		Receive data of given size over SPI bus in blocking mode
 * 		Used primarily for receiving data via SPI when in handling
 * 		command mode.
 * @param
 * 		*rx_data: pointer to receive data
 * 		data_length: numbers of bytes to be receive
 * @return
 * 		HAL level return status
 */
HAL_StatusTypeDef obc_spi_receive_blocking(uint8_t *rx_data, uint16_t data_length) {
    return HAL_SPI_Receive(&hspi1, rx_data, data_length, HAL_MAX_DELAY);
}
