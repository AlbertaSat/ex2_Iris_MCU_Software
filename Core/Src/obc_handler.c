#include <command_handler.h>
#include <iris_system.h>
#include <string.h>
#include <arducam.h>
#include <obc_handler.h>
#include <spi_bitbang.h>
#include "debug.h"
#include <spi_obc.h>
#include "iris_time.h"

extern SPI_HandleTypeDef hspi1;
extern uint8_t cam_to_nand_transfer_flag;

static uint32_t count = 0x0FFF0000;
uint8_t sensor_mode = 0;

void transfer_image_to_obc();
int step_transfer();

/*
 * Refactor Iris
 * - No processing of command occurs, delegate it to command_handler.h
 * - spi drivers should be placed in the driver file **
 * - Only verification and facilitation of commands should happen hear
 */

/**
 * @brief
 * 		Receive data of given size over SPI bus in blocking mode
 * @param
 * 		*rx_data: pointer to receive data
 * 		data_length: numbers of bytes to be receive
 */
void spi_receive_blocking(uint8_t *rx_data, uint16_t data_length) {
    uint8_t tx_dummy = 0xFF;
    HAL_SPI_TransmitReceive(&hspi1, &tx_dummy, rx_data, data_length, HAL_MAX_DELAY);
}

/**
 * @brief
 * 		Verifies if command from OBC is valid or not
 * @param
 * 		obc_cmd: Command from OBC
 * @return
 * 		1 if valid command, 0 if not
 */
int obc_verify_command(uint8_t obc_cmd) {
    uint8_t ack = 0xAA;
    uint8_t nack = 0x0F;
    uint8_t transmit_ack;

    transmit_ack = 0;

    switch (obc_cmd) {
    case IRIS_SEND_HOUSEKEEPING: {
        transmit_ack = 1;
        break;
    }
    case IRIS_TAKE_PIC: {
        transmit_ack = 1;
        break;
    }
    case IRIS_GET_IMAGE_COUNT: {
        transmit_ack = 1;
        break;
    }
    case IRIS_TRANSFER_IMAGE: {
        transmit_ack = 1;
        break;
    }
    case IRIS_OFF_SENSORS: {
        transmit_ack = 1;
        break;
    }
    case IRIS_ON_SENSORS: {
        transmit_ack = 1;
        break;
    }
    case IRIS_GET_IMAGE_LENGTH: {
        transmit_ack = 1;
        break;
    }
    case IRIS_UPDATE_SENSOR_I2C_REG: {
        transmit_ack = 1;
        break;
    }
    case IRIS_UPDATE_CURRENT_LIMIT: {
        transmit_ack = 1;
        break;
    }
    case IRIS_WDT_CHECK: {
        transmit_ack = 1;
        break;
    }
    case IRIS_SET_TIME: {
        transmit_ack = 1;
        break;
    }
    default: {
        transmit_ack = 0;
    }
    }

    if (transmit_ack != 0) {
        obc_spi_transmit(&ack, 2);
        return 0;
    } else {
        obc_spi_transmit(&nack, 1);
        return -1;
    }
}

/**
 * @brief
 * 		Handles command from OBC
 * @param
 * 		obc_cmd: Command from OBC
 * @return
 * 		1 if valid command, 0 if not
 */
int obc_handle_command(uint8_t obc_cmd) {
    uint8_t tx_data = 0x69;
    uint8_t tx_ack = 0xAA;

    switch (obc_cmd) {
    case IRIS_SEND_HOUSEKEEPING: {
        housekeeping_packet_t hk;
        get_housekeeping(&hk);

        uint8_t buffer[sizeof(hk)];
        memcpy(buffer, &hk, sizeof(hk));
        obc_spi_transmit(buffer, sizeof(buffer));
        return 0;
    }
    case IRIS_TAKE_PIC: {
        take_image();
        return 0;
    }
    case IRIS_GET_IMAGE_COUNT: {
        get_image_num(0);
        obc_spi_transmit(&tx_data, 1);
        return 0;
    }
    case IRIS_TRANSFER_IMAGE: {
        transfer_image_to_obc();
        return 0;
    }
    case IRIS_OFF_SENSORS: {
        turn_off_sensors();
        DBG_PUT("Sensor de-activated\r\n");
        obc_spi_transmit(&tx_ack, 1);
        return 0;
    }
    case IRIS_ON_SENSORS: {
        turn_on_sensors();
        DBG_PUT("Sensor activated\r\n");
        set_sensors_config();
        DBG_PUT("Sensors configured\r\n");
        obc_spi_transmit(&tx_ack, 1);
        return 0;
    }
    case IRIS_GET_IMAGE_LENGTH: {
        uint32_t image_length;
        uint8_t packet[3];

        if (sensor_mode == 0) {
            get_image_length(&image_length, VIS_SENSOR);
        } else {
            get_image_length(&image_length, NIR_SENSOR);
        }

        packet[0] = (image_length >> (8 * 2)) & 0xff;
        packet[1] = (image_length >> (8 * 1)) & 0xff;
        packet[2] = (image_length >> (8 * 0)) & 0xff;

        obc_spi_transmit(packet, IRIS_IMAGE_SIZE_WIDTH);
        return 0;
    }
    case IRIS_UPDATE_SENSOR_I2C_REG: {
        // update_sensor_I2C_regs();
        obc_spi_transmit(&tx_data, 1);
        return 0;
    }
    case IRIS_UPDATE_CURRENT_LIMIT: {
        // update_current_limits();
        obc_spi_transmit(&tx_data, 1);
        return 0;
    }
    case IRIS_SET_TIME: {
        uint32_t obc_unix_time;
        uint8_t iris_unix_time_buffer[IRIS_UNIX_TIME_SIZE];

        obc_spi_receive_blocking(iris_unix_time_buffer, IRIS_UNIX_TIME_SIZE);

        obc_unix_time =
            (uint32_t)((uint8_t)iris_unix_time_buffer[0] << 24 | (uint8_t)iris_unix_time_buffer[1] << 16 |
                       (uint8_t)iris_unix_time_buffer[2] << 8 | (uint8_t)iris_unix_time_buffer[3]);

        set_rtc_time(obc_unix_time);

        Iris_Timestamp tm = {0};
        get_rtc_time(&tm);
    }
    case IRIS_WDT_CHECK: {
        return 0;
    }
    default:
        iterate_error_num();
        // sys_log("Oh shit! We failed to handle a command!");
        return -1;
    }
}

/**
 * @brief Transfer image data from Iris to OBC
 *
 * Currently using direct register access method to get image data via
 * reading data register from Arducam module.
 *
 * TODO: Update data retrieval once NAND fs is intergrated
 */
void transfer_image_to_obc() {
    uint8_t image_data[IRIS_IMAGE_TRANSFER_BLOCK_SIZE];
    uint16_t num_transfers;
    uint32_t image_length;

    if (sensor_mode == 0) {
        get_image_length(&image_length, VIS_SENSOR);
    } else {
        get_image_length(&image_length, NIR_SENSOR);
    }
    num_transfers =
        (uint16_t)((image_length + (IRIS_IMAGE_TRANSFER_BLOCK_SIZE - 1)) / IRIS_IMAGE_TRANSFER_BLOCK_SIZE);

    if (sensor_mode == 0) {
        spi_init_burst(VIS_SENSOR);
    } else {
        spi_init_burst(NIR_SENSOR);
    }
    for (int j = 0; j < num_transfers; j++) {
        for (int i = 0; i < IRIS_IMAGE_TRANSFER_BLOCK_SIZE; i++) {
            if (sensor_mode == 0) {
                image_data[i] = (uint8_t)spi_read_burst(VIS_SENSOR);
            } else {
                image_data[i] = (uint8_t)spi_read_burst(NIR_SENSOR);
            }
        }

        obc_spi_transmit(image_data, IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
    }
    if (sensor_mode == 0) {
        spi_deinit_burst(VIS_SENSOR);
    } else {
        spi_deinit_burst(NIR_SENSOR);
    }

    if (sensor_mode == 0) {
        DBG_PUT("DONE IMAGE TRANSFER (VIS_SENSOR)!\r\n");
        sensor_mode = 1;
    } else {
        DBG_PUT("DONE IMAGE TRANSFER (NIR_SENSOR)!\r\n");
        sensor_mode = 0;
    }
}

/**
 * @brief
 * 		Dummy function to represent task for transferring
 * 		image data from camera to NAND flash
 * @return
 * 		1 if loop completed, 0 if not
 */
int step_transfer() {

    while (count != 0) {
        HAL_Delay(10);
        --count;
        return 0;
    }

    cam_to_nand_transfer_flag = 0;
    return 1;
}
