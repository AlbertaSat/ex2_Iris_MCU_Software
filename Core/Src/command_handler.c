/*
 * command_handler.c
 *
 *  Created on: May 9, 2022
 *      Author: Liam
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "command_handler.h"
#include "debug.h"
#include "arducam.h"
#include "IEB_TESTS.h"
#include "iris_time.h"
#include "time.h"
#include "nandfs.h"
#include "nand_types.h"
#include "iris_system.h"
#include "nand_errno.h"
#include "logger.h"

extern uint8_t VIS_DETECTED;
extern uint8_t NIR_DETECTED;

extern SPI_HandleTypeDef hspi1;
extern RTC_HandleTypeDef hrtc;

extern int format;
extern int width;

extern const struct sensor_reg OV5642_JPEG_Capture_QSXGA[];
extern const struct sensor_reg OV5642_QVGA_Preview[];

extern uint8_t turn_off_logger_flag;
extern uint8_t direct_method_flag;

#ifdef UART_HANDLER
// Only used for UART operations
uint8_t VIS_DETECTED = 0;
uint8_t NIR_DETECTED = 0;
#endif

uint8_t sensor_status = SENSORS_OFF;

housekeeping_packet_t hk;
char buf[128];

FileInfo_t image_file_infos_queue[MAX_IMAGE_FILES] = {0};
uint8_t image_count = 0;

/******************************************************************************
 *                      		SPI Operations
 *****************************************************************************/
/**
 * @brief Return the housekeeping object
 * @param Housekeeping packet type variable
 */
void get_housekeeping(housekeeping_packet_t *hk) { *(hk) = _get_housekeeping(); }

/**
 * @brief Initialize appropriate sensors' registers and start capture
 */
void take_image() {
    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, VIS_SENSOR); // VSYNC is active HIGH
    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, NIR_SENSOR);

    flush_fifo(VIS_SENSOR);
    flush_fifo(NIR_SENSOR);

    clear_fifo_flag(VIS_SENSOR);
    clear_fifo_flag(NIR_SENSOR);

    start_capture(VIS_SENSOR);
    start_capture(NIR_SENSOR);

    while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, VIS_SENSOR) &&
           !get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, NIR_SENSOR)) {
    }
}

/**
 * @brief Counts number of images stored in the flash file system and transmits it over SPI
 *
 * @param image_count: Pointer to variable containing number of images in NAND fs
 * Untested
 */
void get_image_count(uint8_t *cnt) { *(cnt) = image_count; }

/**
 * @brief Get the image length
 *
 * @param image_length: Pointer to variable containing length of image
 *
 * Currently image length is directly obtained from Arducam registers, although
 * it is desired to extract image lengths from NAND fs
 */
int get_image_length(uint32_t *image_length, uint8_t index) {
    int ret;
    NAND_FILE *file;
    file = NANDfs_open(image_file_infos_queue[index].file_id);

    if (!file) {
        iris_log("open file %d failed: %d\r\n", file, nand_errno);
        return -1;
    }
    *(image_length) = file->node.file_size;

    ret = NANDfs_close(file);
    if (ret < 0) {
        iris_log("not able to close file %d failed: %d\r\n", file, nand_errno);
        return -1;
    }

    return 0;
}

/**
 * @brief Sends Arducam sensors into an idle (re: powered off) state by turning off FET driver pin
 */
void turn_off_sensors() {
    HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_RESET);
    iris_log("Sensor Power Disabled.\r\n");
    sensor_status = SENSORS_OFF;
}

/**
 * @brief Sends Arducam sensors into an active (re: powered on) state by turning on FET driver pin
 */
void turn_on_sensors() {
    HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_SET);
    iris_log("Sensor Power Enabled\r\n");
    sensor_status = SENSORS_ON;
}

/**
 * @brief Once sensors are on, initialize configurations (i.e. resolution,
 * saturation, etc) for both of the sensors
 *
 * Future developers can add or modify different configuration
 */
void set_configurations(Iris_config *config) {
    // These flags toggle different software methods
    turn_off_logger_flag = config->toggle_iris_logger;
    direct_method_flag = config->toggle_direct_method;

    // Format NAND flash
    uint8_t format_nand_flash = config->format_iris_nand;
    if (format_nand_flash == 1) {
        NANDfs_format();
    }

    if (sensor_status == SENSORS_ON) {
        // Set resolution for both sensors
        arducam_set_resolution(JPEG, (int)config->set_resolution, VIS_SENSOR);
        arducam_set_resolution(JPEG, (int)config->set_resolution, NIR_SENSOR);

        // Set saturation for both sensors
        arducam_set_saturation((int)config->set_saturation, VIS_SENSOR);
        arducam_set_saturation((int)config->set_saturation, NIR_SENSOR);
    }

    HAL_Delay(500);
}

/**
 * @brief   Floods camera SPI with dummy data to non-addressable regs to init sensors
 */
void flood_cam_spi() {
    for (uint8_t i = 0x60; i < 0x6F; i++) {
        read_spi_reg(i, VIS_SENSOR);
        read_spi_reg(i, NIR_SENSOR);
    }
}

/*
 * @brief Initializes sensors to our chosen defaults as defined in main.c
 */
int initalize_sensors() {
    int res;

    res = onboot_sensors(VIS_SENSOR);
    if (res == 1) {
        program_sensor(format, VIS_SENSOR);
        iris_log("VIS Camera Mode: JPEG\r\nI2C address: 0x3C");
#ifdef UART_HANDLER
        VIS_DETECTED = 1;
#endif
    }
    if (res == -1) {
        // need some error handling eh
        iterate_error_num();
        iris_log("VIS init failed.");
        return -1;
    }

    res = onboot_sensors(NIR_SENSOR);
    if (res == 1) {
        program_sensor(format, NIR_SENSOR);
        iris_log("NIR Camera Mode: JPEG\r\nI2C address: 0x3D");
#ifdef UART_HANDLER
        NIR_DETECTED = 1;
#endif
    } else {
        iris_log("NIR initialization failed.");
        return -1;
    }
    if (res == -1) {
        // need some error handling eh
        iterate_error_num();
        iris_log("NIR init failed.");
        return -1;
    }

    HAL_Delay(100);
    return 0;
}

/*
 * @brief Lower level wrapper function to initialize sensors on boot
 *
 * @param sensor: Integer sensor identifier
 */
int onboot_sensors(uint8_t sensor) {
    // Reset the CPLD

    // Make sure camera is listening over SPI
    arducam_wait_for_ready(sensor);
    // reset I2C regs
    write_reg(AC_REG_RESET, 1, sensor);
    write_reg(AC_REG_RESET, 1, sensor);
    HAL_Delay(100);
    write_reg(AC_REG_RESET, 0, sensor);
    HAL_Delay(100);

    if (!arducam_wait_for_ready(sensor)) {
        iris_log("Camera %x: SPI Unavailable", sensor);
    } else {
        iris_log("Camera %x: SPI Initialized", sensor);
    }

    // Change MCU mode
    write_reg(ARDUCHIP_MODE, 0x0, sensor);
    wrSensorReg16_8(0xff, 0x01, sensor);

    uint8_t vid = 0, pid = 0;
    // checks sensor id to ensure proper i2c performance
    rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid, sensor);
    rdSensorReg16_8(OV5642_CHIPID_LOW, &pid, sensor);

    if (vid != 0x56 || pid != 0x42) {
        iris_log("Camera %x I2C Address: Unknown\r\nVIS not available\r\n\n", sensor);
        return -1;
    } else {
        return 1;
    }
}

/*
 * @brief Initialize internal RTC with respect to OBC's RTC
 *
 * @param unix_time: Time integer in unix format
 */
void set_rtc_time(uint32_t unix_time) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    Iris_Timestamp timestamp = {0};

    convertUnixToUTC((time_t)unix_time, &timestamp);

    sTime.Hours = timestamp.Hour;     // set hours
    sTime.Minutes = timestamp.Minute; // set minutes
    sTime.Seconds = timestamp.Second; // set seconds
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        Error_Handler();
    }
    sDate.WeekDay = timestamp.Wday; // day
    sDate.Month = timestamp.Month;  // month
    sDate.Date = timestamp.Day;     // date
    sDate.Year = timestamp.Year;    // year
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        Error_Handler();
    }
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2); // backup register
}

/*
 * @brief Get current time from RTC in UTC format
 *
 * **For debugging we are printing out the time, FM
 * will return a non-void return type
 */
void get_rtc_time(Iris_Timestamp *timestamp) {
    RTC_DateTypeDef gDate;
    RTC_TimeTypeDef gTime;

    /* Get the RTC current Time */
    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
    /* Get the RTC current Date */
    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

    timestamp->Hour = gTime.Hours;
    timestamp->Minute = gTime.Minutes;
    timestamp->Second = gTime.Seconds;
    timestamp->Wday = 0; // Not needed
    timestamp->Day = gDate.Date;
    timestamp->Month = gDate.Month;
    timestamp->Year = 1970 + gDate.Year;

#ifdef DEBUG_OUTPUT
    /* Display time Format: hh:mm:ss */
    iris_log("%02d:%02d:%02d\r\n", timestamp->Hour, timestamp->Minute, timestamp->Second);
    /* Display date Format: dd-mm-yy */
    iris_log("%02d-%02d-%2d\r\n", timestamp->Day, timestamp->Month, timestamp->Year);
#endif
}

int transfer_image_to_nand(uint8_t sensor, uint8_t *file_timestamp) {
    int ret = 0;
    HAL_Delay(100);

    uint32_t image_size;
    image_size = read_fifo_length(sensor);

    NAND_FILE *file = NANDfs_create();
    if (!file) {
        iris_log("not able to create file %d failed: %d", file, nand_errno);
        return -1;
    }

    int size_remaining;
    uint8_t image[PAGE_DATA_SIZE];
    uint32_t i = 0;
    int chunks_to_write = ((image_size + (PAGE_DATA_SIZE - 1)) / PAGE_DATA_SIZE);

    spi_init_burst(sensor);
    for (int j = 0; j < chunks_to_write; j++) {
        for (i = 0; i < PAGE_DATA_SIZE; i++) {
            image[i] = spi_read_burst(sensor);
        }
        size_remaining = i;
        while (size_remaining > 0) {
            int size_to_write = size_remaining > PAGE_DATA_SIZE ? PAGE_DATA_SIZE : size_remaining;
            ret = NANDfs_write(file, size_to_write, image);
            if (ret < 0) {
                iris_log("not able to write to file %d failed: %d", file, nand_errno);
                return -1;
            }
            size_remaining -= size_to_write;
        }
    }
    spi_deinit_burst(sensor);
    file->node.file_name = file_timestamp;

    image_file_infos_queue[image_count].file_id = file->node.id;
    image_file_infos_queue[image_count].file_name = file->node.file_name;
    image_file_infos_queue[image_count].file_size = file->node.file_size;

    ret = NANDfs_close(file);
    if (ret < 0) {
        iris_log("not able to close file %d failed: %d", file, nand_errno);
        return -1;
    }

    iris_log("%d|%s|%d", image_file_infos_queue[image_count].file_id,
             image_file_infos_queue[image_count].file_name, image_file_infos_queue[image_count].file_size);

    image_count += 1;
    return 0;
}

int delete_image_file_from_queue(uint16_t index) {
    int ret;

    ret = NANDfs_delete(image_file_infos_queue[index].file_id);
    if (ret < 0) {
        iris_log("not able to delete file %d failed: %d\r\n", image_file_infos_queue[index].file_id, nand_errno);
        return -1;
    }
    image_file_infos_queue[index].file_id = -1;
    return 0;
}

NAND_FILE *get_image_file_from_queue(uint8_t index) {
    NAND_FILE *file = NANDfs_open(image_file_infos_queue[index].file_id);
    if (!file) {
        iris_log("not able to open file %d failed: %d\r\n", file, nand_errno);
    }
    return file;
}

/*
 * @brief Get timestamp for image capture
 */
void set_capture_timestamp(uint8_t *capture_timestamp, uint8_t sensor) {
    Iris_Timestamp timestamp = {0};
    get_rtc_time(&timestamp);

    if (sensor == VIS_SENSOR) {
        snprintf(capture_timestamp, CAPTURE_TIMESTAMP_SIZE, "%d_%d_%d_%d_%d_%d_vis.jpg", timestamp.Hour,
                 timestamp.Minute, timestamp.Second, timestamp.Day, timestamp.Month, timestamp.Year);
    } else if (sensor == NIR_SENSOR) {
        snprintf(capture_timestamp, CAPTURE_TIMESTAMP_SIZE, "%d_%d_%d_%d_%d_%d_nir.jpg", timestamp.Hour,
                 timestamp.Minute, timestamp.Second, timestamp.Day, timestamp.Month, timestamp.Year);
    }
}

/*
 * @brief Store information from existing files from NAND to
 * 		  RAM (buffer)
 *
 * 		  The purpose of transferring file info (file_id, file_name,
 * 		  file_size) from NAND to RAM during on-boot is to set up a
 * 		  quick lookup table for image transfer APIs to quickly
 * 		  extract file information, instead of accessing NAND structures
 * 		  directly.
 */
int store_file_infos_in_buffer() {
    uint8_t index;
    DIRENT cur_node;
    NAND_DIR *cur_dir;
    int ret = 0;

    index = 0;
    cur_dir = NANDfs_opendir();

    do {
        cur_node = *(NANDfs_getdir(cur_dir));

        ret = NANDfs_nextdir(cur_dir);
        if (ret < 0) {
            if (nand_errno == NAND_EBADF) {
                iris_log("Reached end of last inode. Total image files: %d\r\n", image_count);
                return 0;
            } else {
                iris_log("Moving to next directory entry failed: %d\r\n", nand_errno);
                return -1;
            }
        }

        image_file_infos_queue[index].file_id = cur_node.id;
        image_file_infos_queue[index].file_name = cur_node.file_name;
        image_file_infos_queue[index].file_size = cur_node.file_size;

        image_count++;
        index += 1;
    } while (ret != 0);

    return 0;
}

/******************************************************************************
 *                      UART Operations (Not used in flight)
 *****************************************************************************/

static inline const char *next_token(const char *ptr) {
    /* move to the next space */
    while (*ptr && *ptr != ' ')
        ptr++;
    /* move past any whitespace */
    while (*ptr && isspace(*ptr))
        ptr++;

    return (*ptr) ? ptr : NULL;
}

/**
 * @brief UART command handler
 *
 * @param pointer to char command from main
 */
void uart_handle_command(char *cmd) {
    uint8_t in[sizeof(housekeeping_packet_t)];
    switch (*cmd) {
    case 'c':
        uart_handle_capture_cmd(cmd);
        //        take_image();
        break;
    case 'f':
        uart_handle_format_cmd(cmd);
        break;
    case 'w':
        uart_handle_width_cmd(cmd);
        break;
    case 's':
        switch (*(cmd + 1)) {
        case 'c':
            uart_scan_i2c();
            break;

        case 'a': {
            const char *c = next_token(cmd);
            switch (*c) {
            case 'v':
                uart_handle_saturation_cmd(c, VIS_SENSOR);
                break;
            case 'n':
                uart_handle_saturation_cmd(c, NIR_SENSOR);
                break;
            default:
                iris_log("Target Error\r\n");
                break;
            }
        } break;
        }
        break;
    case 'p': {
        const char *p = next_token(cmd);
        switch (*(p + 1)) {
        case 'n':
            turn_on_sensors();
            break;
        case 'f':
            turn_off_sensors();
            break;
        default:
            iris_log("Use either on or off\r\n");
            break;
        }
    } break;
    case 'i':
        switch (*(cmd + 1)) {
        case '2':
            handle_i2c16_8_cmd(cmd); // needs to handle 16 / 8 bit stuff
            break;
        default:;
            const char *i = next_token(cmd);
            switch (*i) {
            case 's':
                initalize_sensors();
                break;
            }
        }
        break;
    case 'h':
        switch (*(cmd + 1)) {
        case 'k':
            uart_get_hk_packet(in);
            break;
        default:
            help();
            break;
        }
        break;
    case 'n':
        uart_handle_nand_commands(cmd);
        break;

    default:
        help();
        break;
    }
}
