/*
 * spi_command_handler.h
 *
 *  Created on: Jun 12, 2022
 *      Author: jenish
 */

#ifndef INC_OBC_HANDLER_H_
#define INC_OBC_HANDLER_H_

#include <stdio.h>
#include <command_handler.h>

/* Iris commands */
#define IRIS_TAKE_PIC 0x10
#define IRIS_GET_IMAGE_LENGTH 0x20
#define IRIS_TRANSFER_IMAGE 0x31
#define IRIS_TRANSFER_LOG 0x34
#define IRIS_GET_IMAGE_COUNT 0x30
#define IRIS_ON_SENSORS 0x40
#define IRIS_OFF_SENSORS 0x41
#define IRIS_SEND_HOUSEKEEPING 0x51
#define IRIS_UPDATE_SENSOR_I2C_REG 0x60
#define IRIS_UPDATE_CURRENT_LIMIT 0x70
#define IRIS_SET_TIME 0x05
#define IRIS_WDT_CHECK 0x80

#define IRIS_IMAGE_TRANSFER_BLOCK_SIZE 512 // Will change once NAND flash is implemented
#define IRIS_LOG_TRANSFER_BLOCK_SIZE 2048
#define IRIS_IMAGE_SIZE_WIDTH 3 // Image size represented in 3 bytes
#define IRIS_UNIX_TIME_SIZE 4
#define IRIS_NUM_COMMANDS 12

int obc_verify_command(uint8_t cmd);
int obc_handle_command(uint8_t cmd);

void transfer_image_to_obc_direct_method();
int transfer_images_to_obc_nand_method(uint8_t image_index);
int transfer_log_to_obc();

#endif /* INC_OBC_HANDLER_H_ */
