/*
 * spi_command_handler.h
 *
 *  Created on: Jun 12, 2022
 *      Author: jenish
 */

#ifndef INC_SPI_COMMAND_HANDLER_H_
#define INC_SPI_COMMAND_HANDLER_H_

#include <stdio.h>

/* Iris commands */
#define IRIS_TAKE_PIC 0x10
#define IRIS_GET_IMAGE_LENGTH 0x20
#define IRIS_TRANSFER_IMAGE 0x31
#define IRIS_GET_IMAGE_COUNT 0x30
#define IRIS_ON_SENSOR_IDLE 0x40
#define IRIS_OFF_SENSOR_IDLE 0x41
#define IRIS_SEND_HOUSEKEEPING 0x51
#define IRIS_UPDATE_SENSOR_I2C_REG 0x60
#define IRIS_UPDATE_CURRENT_LIMIT 0x70

#define IRIS_IMAGE_TRANSFER_BLOCK_SIZE 512 // Will change once NAND flash is implemented

void spi_transmit(uint8_t *tx_data, uint16_t data_length);
void spi_receive(uint8_t *rx_data, uint16_t data_length);

int spi_listen();
int spi_verify_command(uint8_t cmd);
int spi_handle_command(uint8_t cmd);
int step_transfer();
void spi_transfer_image();

#endif /* INC_SPI_COMMAND_HANDLER_H_ */
