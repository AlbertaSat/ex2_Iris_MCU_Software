#include <spi_command_handler.h>
#include "main.h"

extern SPI_HandleTypeDef hspi1;

static uint8_t cmd;

void spi_transmit(uint8_t *tx_data, uint16_t data_length) {
	HAL_SPI_Transmit(&hspi1, tx_data, data_length, HAL_MAX_DELAY);
}

void spi_receive(uint8_t *rx_data, uint16_t data_length) {
	HAL_SPI_Receive(&hspi1, rx_data, 1, HAL_MAX_DELAY);
}

/**
 * @brief SPI command verifier
 */
int spi_verify_command() {
	uint8_t ack = 0xAA;
	uint8_t nack = 0x55;

	spi_receive(&cmd, 1);

	switch (cmd) {
	case IRIS_SEND_HOUSEKEEPING: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_TAKE_PIC: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_GET_IMAGE_COUNT: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_TRANSFER_IMAGE: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_OFF_SENSOR_IDLE: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_ON_SENSOR_IDLE: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_GET_IMAGE_LENGTH: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_UPDATE_SENSOR_I2C_REG: {
		spi_transmit(&ack, 1);
		return 0;
	}
	case IRIS_UPDATE_CURRENT_LIMIT: {
		spi_transmit(&ack, 1);
		return 0;
	}
	default: {
		spi_transmit(&nack, 1);
		return -1;
	}
	}
}

/**
 * @brief SPI command handler
 */
int spi_handle_command() {
	uint8_t rx_data;
	uint8_t tx_data = 0x69;

	spi_receive(&rx_data, 1);

	switch (cmd) {
	case IRIS_SEND_HOUSEKEEPING:
		//get_housekeeping();
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_TAKE_PIC:
//        take_image(cmd);
//        iterate_image_num();
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_GET_IMAGE_COUNT:
//        get_image_num(0); // 0 for spi return
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_TRANSFER_IMAGE:
		//TODO
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_OFF_SENSOR_IDLE:
//        sensor_active();
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_ON_SENSOR_IDLE:
//        sensor_idle();
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_GET_IMAGE_LENGTH:
//		get_image_length();
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_UPDATE_SENSOR_I2C_REG:
//    	update_sensor_I2C_regs();
		spi_transmit(&tx_data, 1);
		return 0;
	case IRIS_UPDATE_CURRENT_LIMIT:
//    	update_current_limits();
		spi_transmit(&tx_data, 1);
		return 0;
	}
}
