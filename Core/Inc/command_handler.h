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
#include "nand_types.h"

#define REG_SYS_CTL0 0x3008 /* System Control */

extern int format;

extern uint8_t VIS_DETECTED;
extern uint8_t NIR_DETECTED;

typedef struct __attribute__((__packed__)) currentsense_packet_s {
    uint8_t reg;
    uint16_t value;
} currentsense_packet_t;

typedef struct {
    uint32_t file_id;
    uint8_t *file_name;
    uint32_t file_size;
} FileInfo_t;

#define SENSORS_OFF 0
#define SENSORS_ON 1

#define MAX_IMAGE_FILES 20        // Maximum images that can be stored on NAND
#define CAPTURE_TIMESTAMP_SIZE 33 // In bytes

void get_housekeeping(housekeeping_packet_t *hk);
void take_image();
void get_image_count(uint8_t *cnt);
int get_image_length(uint32_t *image_length, uint8_t index);
void turn_off_sensors();
void turn_on_sensors();
void set_sensors_config();
int initalize_sensors();
int onboot_sensors(uint8_t sensor);
void set_rtc_time(uint32_t obc_unix_time);
void get_rtc_time(Iris_Timestamp *timestamp);
int transfer_image_to_nand(uint8_t sensor, uint8_t *file_timestamp);
int delete_image_file_from_queue(uint16_t index);
NAND_FILE *get_image_file_from_queue(uint8_t index);
void set_capture_timestamp(uint8_t *file_timestamp, uint8_t sensor);
int store_file_infos_in_buffer();
void flood_cam_spi();

// uart
void uart_get_hk_packet(uint8_t *out);
void uart_handle_capture_cmd(const char *cmd);
void uart_handle_format_cmd(const char *cmd);
void uart_handle_width_cmd(const char *cmd);
void uart_handle_saturation_cmd(const char *cmd, uint8_t sensor);
void init_sensors(void);

#endif /* INC_COMMAND_HANDLER_H_ */
