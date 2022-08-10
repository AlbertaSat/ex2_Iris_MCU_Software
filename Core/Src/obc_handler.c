#include "command_handler.h"
#include "iris_system.h"
#include "arducam.h"
#include "obc_handler.h"
#include "debug.h"
#include "spi_obc.h"
#include "iris_time.h"
#include "spi_bitbang.h"

#include "nand_types.h"
#include "nandfs.h"
#include "nand_m79a_lld.h"

extern SPI_HandleTypeDef hspi1;
extern uint8_t image_count;

uint8_t sensor = VIS_SENSOR; // VIS or NIR, used exclusively in direct transfer mode
uint8_t image_file_infos_queue_iterator = 0;

const uint8_t iris_commands[IRIS_NUM_COMMANDS] = {IRIS_TAKE_PIC,
                                                  IRIS_GET_IMAGE_LENGTH,
                                                  IRIS_TRANSFER_IMAGE,
                                                  IRIS_TRANSFER_LOG,
                                                  IRIS_GET_IMAGE_COUNT,
                                                  IRIS_ON_SENSORS,
                                                  IRIS_OFF_SENSORS,
                                                  IRIS_SEND_HOUSEKEEPING,
                                                  IRIS_UPDATE_SENSOR_I2C_REG,
                                                  IRIS_UPDATE_CURRENT_LIMIT,
                                                  IRIS_SET_TIME,
                                                  IRIS_WDT_CHECK};

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

    for (uint8_t index = 0; index < IRIS_NUM_COMMANDS; index++) {
        if (iris_commands[index] == obc_cmd) {
            transmit_ack = 1;
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
    uint8_t tx_ack = 0xAA;
    uint8_t tx_nack = 0x0F;

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

        uint8_t cur_capture_timestamp_vis[CAPTURE_TIMESTAMP_SIZE];
        uint8_t cur_capture_timestamp_nir[CAPTURE_TIMESTAMP_SIZE];

        set_capture_timestamp(cur_capture_timestamp_vis, VIS_SENSOR);
        set_capture_timestamp(cur_capture_timestamp_nir, NIR_SENSOR);

#ifndef DIRECT_METHOD
        obc_disable_spi_rx();
        transfer_image_to_nand(VIS_SENSOR, cur_capture_timestamp_vis);
        transfer_image_to_nand(NIR_SENSOR, cur_capture_timestamp_nir);
        obc_enable_spi_rx();
#endif
        return 0;
    }
    case IRIS_GET_IMAGE_COUNT: {
        uint8_t cnt;
        get_image_count(&cnt);
        obc_spi_transmit(&cnt, 1);
        return 0;
    }
    case IRIS_TRANSFER_IMAGE: {
#ifdef DIRECT_METHOD
        transfer_image_to_obc_direct_method();
#else
        transfer_images_to_obc_nand_method(image_file_infos_queue_iterator);
        delete_image_file_from_queue(image_file_infos_queue_iterator);
        image_count -= 1;
        image_file_infos_queue_iterator += 1;

        // Once all images are transferred to OBC, image count should
        // be 0, and image_file_infos_queue_iterator will be re-initialize to 0
        if (image_count == 0) {
            image_file_infos_queue_iterator = 0;
        }
#endif
        return 0;
    }
    case IRIS_TRANSFER_LOG: {
        transfer_log_to_obc();
        return 0;
    }
    case IRIS_OFF_SENSORS: {
        turn_off_sensors();
        DBG_PUT("Sensor deactivated\r\n");

        obc_spi_transmit(&tx_ack, 1);
        return 0;
    }
    case IRIS_ON_SENSORS: {
        int ret = 0;

        turn_on_sensors();
        DBG_PUT("Sensor activated\r\n");
        ret = initalize_sensors();
        if (ret < 0) {
            DBG_PUT("Sensor failed to initialized\r\n");
            obc_spi_transmit(&tx_nack, 1);
            return -1;
        } else {
            DBG_PUT("Sensor initialize\r\n");
        }
        set_sensors_config();
        DBG_PUT("Sensors configured\r\n");

        obc_spi_transmit(&tx_ack, 1);
        return 0;
    }
    case IRIS_GET_IMAGE_LENGTH: {
        uint32_t image_length;
        uint8_t packet[IRIS_IMAGE_SIZE_WIDTH];
        memset(packet, 0, IRIS_IMAGE_SIZE_WIDTH);
        int ret;

#ifdef DIRECT_METHOD
        image_length = (uint32_t)read_fifo_length(sensor);
#else
        ret = get_image_length(&image_length, image_file_infos_queue_iterator);
        if (ret < 0) {
            DBG_PUT("Failed to get image length");
            obc_spi_transmit(packet, IRIS_IMAGE_SIZE_WIDTH);
            return -1;
        }
#endif
        packet[0] = (image_length >> (8 * 2)) & 0xff;
        packet[1] = (image_length >> (8 * 1)) & 0xff;
        packet[2] = (image_length >> (8 * 0)) & 0xff;

        obc_spi_transmit(packet, IRIS_IMAGE_SIZE_WIDTH);
        return 0;
    }
    case IRIS_UPDATE_SENSOR_I2C_REG: {
        // update_sensor_I2C_regs();
        obc_spi_transmit(&tx_nack, 1);
        return 0;
    }
    case IRIS_UPDATE_CURRENT_LIMIT: {
        // update_current_limits();
        obc_spi_transmit(&tx_nack, 1);
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
        return 0;
    }
    case IRIS_WDT_CHECK: {
        return 0;
    }
    default:
        iterate_error_num();
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
void transfer_image_to_obc_direct_method() {
    uint8_t image_data[IRIS_IMAGE_TRANSFER_BLOCK_SIZE];
    uint16_t num_transfers;
    uint32_t image_length;

    image_length = (uint32_t)read_fifo_length(sensor);
    num_transfers =
        (uint16_t)((image_length + (IRIS_IMAGE_TRANSFER_BLOCK_SIZE - 1)) / IRIS_IMAGE_TRANSFER_BLOCK_SIZE);

    spi_init_burst(sensor);
    for (int j = 0; j < num_transfers; j++) {
        for (int i = 0; i < IRIS_IMAGE_TRANSFER_BLOCK_SIZE; i++) {
            image_data[i] = (uint8_t)spi_read_burst(sensor);
        }

        obc_spi_transmit(image_data, IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
    }
    spi_deinit_burst(sensor);

    // Once done capturing with current sensor switch to counterpart
    if (sensor == VIS_SENSOR) {
        DBG_PUT("DONE IMAGE TRANSFER (VIS_SENSOR)!\r\n");
        sensor = NIR_SENSOR;
    } else {
        DBG_PUT("DONE IMAGE TRANSFER (NIR_SENSOR)!\r\n");
        sensor = VIS_SENSOR;
    }
}

int transfer_images_to_obc_nand_method(uint8_t image_index) {
    uint8_t page[PAGE_DATA_SIZE];
    int ret;

    NAND_FILE *file = get_image_file_from_queue(image_index);
    if (!file) {
        DBG_PUT("not able to open file %d failed: %d\r\n", file, nand_errno);
        return -1;
    }

    int file_size = file->node.file_size;
    int page_cnt = ((file_size + (PAGE_DATA_SIZE - 1)) / PAGE_DATA_SIZE);

    // below reads out a 2048 byte page, then splits it into 4 512 chunks to transmit over spi
    for (int count = 0; count < page_cnt; count++) {
        ret = NANDfs_read(file, PAGE_DATA_SIZE, page);
        if (ret < 0) {
            DBG_PUT("not able to read file %d failed: %d\r\n", file, nand_errno);
            return -1;
        }
        obc_spi_transmit(page, PAGE_DATA_SIZE);
    }

    uint8_t cnt;
    get_image_count(&cnt);
    DBG_PUT("Image transfer %d/%d completed", image_index + 1, cnt);

    ret = NANDfs_close(file);
    if (ret < 0) {
        DBG_PUT("not able to close file %d failed: %d\r\n", file, nand_errno);
        return -1;
    }

    return 0;
}

int transfer_log_to_obc() {
    PhysicalAddrs addr = {0};
    uint8_t buffer[2048];
    uint8_t char_count = 0;
    char str[128];

    for (uint8_t blk = 0; blk < 2; blk++) {
        for (uint8_t pg = 0; pg < 64; pg++) {
            addr.block = blk;
            addr.page = pg;

            NAND_ReturnType ret = NAND_Page_Read(&addr, 2048, buffer);
            if (ret != Ret_Success) {
                DBG_PUT("read b %d p %d r %d\r\n", blk, pg, ret);
                return ret;
            }
            obc_spi_transmit(buffer, IRIS_LOG_TRANSFER_BLOCK_SIZE);
        }
    }
    return 0;
}
