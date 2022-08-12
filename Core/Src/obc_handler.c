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

//#define DIRECT_METHOD

extern SPI_HandleTypeDef hspi1;
extern uint8_t cam_to_nand_transfer_flag;

static uint32_t count = 0x0FFF0000;
uint8_t sensor_mode = 0;

void transfer_image_to_obc();
int step_transfer();

const uint8_t iris_commands[IRIS_NUM_COMMANDS] = {IRIS_TAKE_PIC,
                                                  IRIS_GET_IMAGE_LENGTH,
                                                  IRIS_TRANSFER_IMAGE,
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

#ifndef DIRECT_METHOD
        obc_disable_spi_rx();
        transfer_image_to_nand();
        obc_enable_spi_rx();
#endif

        return 0;
    }
    case IRIS_GET_IMAGE_COUNT: {
        uint8_t image_count;
        get_image_count(&image_count);

        obc_spi_transmit(&image_count, 1);
        return 0;
    }
    case IRIS_TRANSFER_IMAGE: {
#ifdef DIRECT_METHOD
        transfer_image_to_obc_direct_method();
#else
        transfer_image_to_obc_nand_method();
#endif
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
            return 0;
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
void transfer_image_to_obc_direct_method() {
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

void transfer_image_to_nand() {

    NANDfs_format();
    // get image size
    uint32_t image_size = read_fifo_length(VIS_SENSOR);
    DBG_PUT("Image size: %d bytes\r\n", image_size);

    //    char *data = (char *)malloc(PAGE_LEN);
    char data[PAGE_DATA_SIZE];
    char *sample = data;
    NAND_FILE *fd = NANDfs_create();
    int size_remaining;
    uint8_t image[PAGE_DATA_SIZE];

    spi_init_burst(VIS_SENSOR);
    //    uint8_t prev = 0, curr = 0;
    //    bool found_header = false;
    uint32_t i = 0;

    int chunks_to_write = ((image_size + (PAGE_DATA_SIZE - 1)) / PAGE_DATA_SIZE);

    for (int j = 0; j < chunks_to_write; j++) {
        DBG_PUT("Writing chunk %d / %d\r\n", j + 1, chunks_to_write);
        for (i = 0; i < PAGE_DATA_SIZE; i++) {
            image[i] = spi_read_burst(VIS_SENSOR);
        }
        size_remaining = i;
        //		memcpy(image, sample, i);
        sample += PAGE_DATA_SIZE;
        while (size_remaining > 0) {
            int size_to_write = size_remaining > PAGE_DATA_SIZE ? PAGE_DATA_SIZE : size_remaining;
            NANDfs_write(fd, size_to_write, image);
            //			memcpy(image, sample, size_to_write);
            sample += size_to_write;
            size_remaining -= size_to_write;
        }
    }

    spi_init_burst(VIS_SENSOR);
    NANDfs_close(fd);
}

void transfer_image_to_obc_nand_method() {
    NAND_FILE *fd;
    uint8_t page[PAGE_DATA_SIZE];

    fd = NANDfs_open_latest();
    if (!fd) {
        DBG_PUT("open file %d failed: %d\r\n", fd, nand_errno);
        return;
    }

    uint8_t to_send[IRIS_IMAGE_TRANSFER_BLOCK_SIZE];
    int file_size = fd->node.file_size;
    // DBG_PUT("File size: %d\r\n", file_size);
    int page_cnt = ((file_size + (PAGE_DATA_SIZE - 1)) / PAGE_DATA_SIZE);

    // below reads out a 2048 byte page, then splits it into 4 512 chunks to transmit over spi
    for (int count = 0; count < page_cnt; count++) {
        // DBG_PUT("Reading Page %d / %d.\r\n", count + 1, page_cnt);
        memset(page, 0, PAGE_DATA_SIZE);
        // read 2048B into buffer
        NANDfs_read(fd, PAGE_DATA_SIZE, page);
        obc_spi_transmit(to_send, PAGE_DATA_SIZE);
    }

    DBG_PUT("Image transfer finished");
    NANDfs_close(fd);
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
