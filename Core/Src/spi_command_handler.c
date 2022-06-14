#include <spi_command_handler.h>
#include <command_handler.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

extern SPI_HandleTypeDef hspi1;
//		spi_receive(&rx_data, 1);
static uint8_t cmd;

uint8_t * get_image_buffer();

void spi_transmit(uint8_t *tx_data, uint16_t data_length) {
	HAL_SPI_Transmit(&hspi1, tx_data, data_length, HAL_MAX_DELAY);
}

void spi_receive(uint8_t *rx_data, uint16_t data_length) {
	HAL_SPI_Receive(&hspi1, rx_data, data_length, HAL_MAX_DELAY);
}

/**
 * @brief SPI command verifier
 */
int spi_verify_command() {
	uint8_t ack = 0xAA;
	uint8_t nack = 0x0F;

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

	uint8_t ack = 0xAA;
	uint8_t nack = 0x0F;

	spi_receive(&rx_data, 1);

	switch (cmd) {
	case IRIS_SEND_HOUSEKEEPING:
	{
		housekeeping_packet_t hk;
		get_housekeeping(&hk);

		uint8_t buffer[sizeof(hk)];
		memcpy(buffer, &hk, sizeof(hk));
		spi_transmit(buffer, sizeof(buffer));

		return 0;
	}
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
	{
		uint8_t image_length[2];
		spi_receive(image_length, 2);
		uint16_t length = (uint8_t) image_length[1] << 8 || (uint8_t) image_length[0];

//		uint8_t *buffer = get_image_buffer();
//		for (int i = 0; i < rx_data; i++) {
//			spi_transmit(buffer, 512);
//		}

//		spi_receive(&rx_data, 1);
		return 0;
	}
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
	default:
		return -1;
	}
}

/* FOR TESTING IMAGE TRANSFER: which is not currently working" */
//uint8_t * get_image_buffer() {
////	FILE *file;
//	uint8_t  *buffer;
////	uint16_t fileLen;
////
////	//Open file
////	file = fopen("/home/liam/Desktop/ex2_Iris_MCU_Software/Debug/ex2_Iris_MCU_Software.bin", "rb");
////	if (file == NULL) {
////		return NULL;
////	}
////
////	//Get file length
////	fseek(file, 0, SEEK_END);
////	fileLen=ftell(file);
////	fseek(file, 0, SEEK_SET);
////
////	//Allocate memory
//	buffer=(char *)malloc(512);
////
////   fread(buffer,fileLen,sizeof(uint8_t),file);
////   fclose(file);
//
//   for (int i = 0; i < 512; i++) {
//   		buffer[i] = i;
//   	}
//
//   return buffer;
//}
