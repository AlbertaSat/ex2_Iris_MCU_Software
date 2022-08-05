/*
 * command_handler.h
 *
 *  Created on: May 9, 2022
 *      Author: liam
 */

#ifndef INC_COMMAND_HANDLER_H_
#define INC_COMMAND_HANDLER_H_
#include <iris_system.h>
#include "iris_time.h"
#include "arducam.h"
#include "SPI_IT.h"
#include "I2C.h"
#include "housekeeping.h"
#define REG_SYS_CTL0 0x3008 /* System Control */

//#define GET_IMAGE_NUM 0x15
//#define CAPTURE_IMAGE 0x10
//#define COUNT_IMAGES 0x35
//#define SENSOR_IDLE 0x30
//#define SENSOR_ACTIVE 0x40
//#define GET_HK 0x50

extern int format;

extern uint8_t VIS_DETECTED;
extern uint8_t NIR_DETECTED;

typedef struct __attribute__((__packed__)) currentsense_packet_s {
    uint8_t reg;
    uint16_t value;
} currentsense_packet_t;

void uart_handle_command(char *cmd);
void take_image();
void get_image_length(uint32_t *pdata);
void count_images();
void sensor_reset(uint8_t sensor);
void sensor_idle();
void sensor_active();
void set_time(uint32_t obc_unix_time);
void get_time(Iris_Timestamp *timestamp);
void get_housekeeping(housekeeping_packet_t *hk);
void update_sensor_I2C_regs();
void update_current_limits();
void initalize_sensors(void);
uint8_t get_image_num(uint8_t hk);
void sensor_togglepower(int i);
void iterate_image_num();
int uart_scan_i2c(void);
void handle_wdt();
void print_progress(uint8_t count, uint8_t max);
void handle_i2c16_8_cmd(const char *cmd);
uint8_t onboot_sensors(uint8_t sensor);
void help();
// void _initalize_sensor();
void flood_cam_spi();
uint8_t get_image_num_spi(uint8_t *num);

// uart
void uart_get_hk_packet(uint8_t *out);
void uart_handle_capture_cmd(const char *cmd);
void uart_handle_format_cmd(const char *cmd);
void uart_handle_width_cmd(const char *cmd);
void uart_handle_saturation_cmd(const char *cmd, uint8_t sensor);
void init_sensors(void);

#endif /* INC_COMMAND_HANDLER_H_ */
