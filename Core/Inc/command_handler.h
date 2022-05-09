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

void take_image();
void transfer_image();
void get_image_length();
void count_images();
void sensor_idle();
void sensor_active();
void get_housekeeping();
void update_sensor_I2C_regs();
void update_current_limits();

#endif /* INC_COMMAND_HANDLER_H_ */
