/*
 * command_handler.h
 *
 *  Created on: May 9, 2022
 *      Author: liam
 */

#ifndef INC_COMMAND_HANDLER_H_
#define INC_COMMAND_HANDLER_H_
#include "main.h"
#include "arducam.h"
#include "SPI_IT.h"
#include "I2C.h"
#include "housekeeping.h"

#define REG_SYS_CTL0 0x3008 /* System Control */
#define GET_IMAGE_NUM	0x15
#define CAPTURE_IMAGE 	0x10
#define COUNT_IMAGES 	0x35
#define SENSOR_IDLE 	0x30
#define SENSOR_ACTIVE	0x40
#define GET_HK			0x50
#define I2C_COMPLEX_SHIT 0x69

extern int format;

extern int VIS_DETECTED;
extern int NIR_DETECTED;

void spi_handle_command(uint8_t cmd);
void uart_handle_command(char *cmd);

void take_image();
void get_image_length();
void count_images();

void sensor_reset(uint8_t sensor);
void sensor_idle();
void sensor_active();
void get_housekeeping();
void update_sensor_I2C_regs();
void update_current_limits();
void _initialize_sensor(uint8_t sensor);
uint8_t get_image_num(uint8_t hk);
void iterate_image_num();

int scan_i2c(void);

void print_progress(uint8_t count, uint8_t max);

//void _initalize_sensor();

#endif /* INC_COMMAND_HANDLER_H_ */
