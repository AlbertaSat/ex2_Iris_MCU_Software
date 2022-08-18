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
#include "I2C.h"
#include "housekeeping.h"
#include "iris_time.h"

#define REG_SYS_CTL0 0x3008 /* System Control */

extern int format;

extern uint8_t VIS_DETECTED;
extern uint8_t NIR_DETECTED;

typedef struct __attribute__((__packed__)) currentsense_packet_s {
    uint8_t reg;
    uint16_t value;
} currentsense_packet_t;

#define SENSORS_OFF 0
#define SENSORS_ON 1

#define FILE_TIMESTAMP_SIZE 24 // In bytes

void get_housekeeping(housekeeping_packet_t *hk);
void take_image();
void get_image_count(uint8_t *image_count);
void get_image_length(uint32_t *image_length, uint8_t sensor_mode);
void turn_off_sensors();
void turn_on_sensors();
void set_sensors_config();
int initalize_sensors();
int onboot_sensors(uint8_t sensor);
void set_rtc_time(uint32_t obc_unix_time);
void get_rtc_time(Iris_Timestamp *timestamp);
void get_file_timestamp(uint8_t *file_timestamp);
void flood_cam_spi();

// uart
void uart_get_hk_packet(uint8_t *out);
void uart_handle_capture_cmd(const char *cmd);
void uart_handle_format_cmd(const char *cmd);
void uart_handle_width_cmd(const char *cmd);
void uart_handle_saturation_cmd(const char *cmd, uint8_t sensor);
void init_sensors(void);

#endif /* INC_COMMAND_HANDLER_H_ */
