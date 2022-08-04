#include <spi_command_handler.h>
#include <command_handler.h>
#include <iris_system.h>
#include <string.h>
#include <arducam.h>
#include <spi_bitbang.h>
#include "debug.h"
#include <nandfs.h>
#include "nand_types.h"

extern SPI_HandleTypeDef hspi1;
extern uint8_t cam_to_nand_transfer_flag;

uint32_t image_length; // Only here for testing purposes
static uint32_t count = 0x0FFF0000;

uint8_t sensor_mode = 0;

void take_picture();

/**
 * @brief
 * 		Transmit data of given size over SPI bus in blocking mode
 *
 * @param
 * 		*tx_data: pointer to transmit data
 * 		data_length: numbers of bytes to be sent
 */
void spi_transmit(uint8_t *tx_data, uint16_t data_length) {
    HAL_SPI_Transmit(&hspi1, tx_data, data_length, HAL_MAX_DELAY);
}

/**
 * @brief
 * 		Receive data of given size over SPI bus in interrupt mode
 * @param
 * 		*rx_data: pointer to receive data
 * 		data_length: numbers of bytes to be receive
 */
void spi_receive(uint8_t *rx_data, uint16_t data_length) { HAL_SPI_Receive_IT(&hspi1, rx_data, data_length); }

/**
 * @brief
 * 		Receive data of given size over SPI bus in blocking mode
 * @param
 * 		*rx_data: pointer to receive data
 * 		data_length: numbers of bytes to be receive
 */
void spi_receive_blocking(uint8_t *rx_data, uint16_t data_length) {
    HAL_SPI_Receive(&hspi1, rx_data, data_length, HAL_MAX_DELAY);
}

/**
 * @brief
 * 		Verifies if command from OBC is valid or not
 * @param
 * 		obc_cmd: Command from OBC
 * @return
 * 		1 if valid command, 0 if not
 */
int spi_verify_command(uint8_t obc_cmd) {
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
    case IRIS_OFF_SENSOR_IDLE: {
        transmit_ack = 1;
        break;
    }
    case IRIS_ON_SENSOR_IDLE: {
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
        spi_transmit(&ack, 2);
        return 0;
    } else {
        spi_transmit(&nack, 1);
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
int spi_handle_command(uint8_t obc_cmd) {
    uint8_t tx_data = 0x69;

    switch (obc_cmd) {
    case IRIS_SEND_HOUSEKEEPING: {
        housekeeping_packet_t hk;
        get_housekeeping(&hk);

        uint8_t buffer[sizeof(hk)];
        memcpy(buffer, &hk, sizeof(hk));
        spi_transmit(buffer, sizeof(buffer));
        return 0;
    }
    case IRIS_TAKE_PIC: {
        take_picture();
        spi_transfer_image_to_nand();
        return 0;
    }
    case IRIS_GET_IMAGE_COUNT: {
        get_image_num_spi(0);
        spi_transmit(&tx_data, 1);
        return 0;
    }
    case IRIS_TRANSFER_IMAGE: {
        spi_transfer_image_from_nand();
        return 0;
    }
    case IRIS_OFF_SENSOR_IDLE: {
        sensor_idle();
        return 0;
    }
    case IRIS_ON_SENSOR_IDLE: {
        sensor_active();
        // Set resolution for both sensors
        arducam_set_resolution(JPEG, 2592, VIS_SENSOR);
        arducam_set_resolution(JPEG, 2592, NIR_SENSOR);
        DBG_PUT("Sensor activated\r\n");
        return 0;
    }
    case IRIS_GET_IMAGE_LENGTH: {
        NAND_FILE *fd;
        fd = NANDfs_open_latest();
        image_length = (uint32_t)fd->node.file_size;
        NANDfs_close(fd);

        uint8_t packet[3];
        packet[0] = (image_length >> (8 * 2)) & 0xff;
        packet[1] = (image_length >> (8 * 1)) & 0xff;
        packet[2] = (image_length >> (8 * 0)) & 0xff;

        spi_transmit(packet, IRIS_IMAGE_SIZE_WIDTH);
        return 0;
    }
    case IRIS_UPDATE_SENSOR_I2C_REG: {
        //    	update_sensor_I2C_regs();
        spi_transmit(&tx_data, 1);
        return 0;
    }
    case IRIS_UPDATE_CURRENT_LIMIT: {
        //    	update_current_limits();
        spi_transmit(&tx_data, 1);
        return 0;
    }
    case IRIS_SET_TIME: {
        uint32_t obc_unix_time;
        uint8_t iris_unix_time_buffer[IRIS_UNIX_TIME_SIZE];
        spi_receive_blocking(iris_unix_time_buffer, 4);

        obc_unix_time =
            (uint32_t)((uint8_t)iris_unix_time_buffer[0] << 24 | (uint8_t)iris_unix_time_buffer[1] << 16 |
                       (uint8_t)iris_unix_time_buffer[2] << 8 | (uint8_t)iris_unix_time_buffer[3]);

        set_time(obc_unix_time);
    }
    case IRIS_WDT_CHECK: {
        return 0;
    }
    default:
        return -1;
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

void take_picture() {
    //	char buf[64];
    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, VIS_SENSOR); // VSYNC is active HIGH
    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK,
              NIR_SENSOR); // VSYNC is active HIGH
                           //	sprintf(buf, "Single Capture Transfer type %x\r\n", //format); 	DBG_PUT(buf);
    flush_fifo(VIS_SENSOR);
    flush_fifo(NIR_SENSOR);

    clear_fifo_flag(VIS_SENSOR);
    clear_fifo_flag(NIR_SENSOR);

    start_capture(VIS_SENSOR);
    start_capture(NIR_SENSOR);
    while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, VIS_SENSOR) &&
           !get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, NIR_SENSOR)) {
    }
    //	DBG_PUT("JPG");
}

/**
 * @brief
 * 		Dummy function to dump image data to OBC
 */
void spi_transfer_image() {
    static uint8_t image_data[IRIS_IMAGE_TRANSFER_BLOCK_SIZE];
    static uint16_t num_transfers;

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
            // image_data[i] = (uint8_t) image_data_buffer[count];
        }

        spi_transmit(image_data, IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
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

void spi_transfer_image_to_nand() {
    NANDfs_format();
    // get image size
    uint32_t image_size = read_fifo_length(VIS_SENSOR);
    DBG_PUT("Image size: %d bytes\r\n", image_size);

#define PAGE_LEN 2048

    //    char *data = (char *)malloc(PAGE_LEN);
    char data[PAGE_LEN];
    char *sample = data;
    NAND_FILE *fd = NANDfs_create();
    int size_remaining;
    uint8_t image[PAGE_LEN];

    spi_init_burst(VIS_SENSOR);
    //    uint8_t prev = 0, curr = 0;
    //    bool found_header = false;
    uint32_t i = 0;

    int chunks_to_write = ((image_size + (PAGE_LEN - 1)) / PAGE_LEN);

    for (int j = 0; j < chunks_to_write; j++) {
        DBG_PUT("Writing chunk %d / %d\r\n", j + 1, chunks_to_write);
        for (i = 0; i < PAGE_LEN; i++) {
            image[i] = spi_read_burst(VIS_SENSOR);
        }
        size_remaining = i;
        //		memcpy(image, sample, i);
        sample += PAGE_LEN;
        while (size_remaining > 0) {
            int size_to_write = size_remaining > PAGE_LEN ? PAGE_LEN : size_remaining;
            NANDfs_write(fd, size_to_write, image);
            //			memcpy(image, sample, size_to_write);
            sample += size_to_write;
            size_remaining -= size_to_write;
        }
    }

    spi_init_burst(VIS_SENSOR);
    NANDfs_close(fd);
}

void spi_transfer_image_from_nand() {
    NAND_FILE *fd;
    uint8_t page[2048];

    fd = NANDfs_open_latest();
    if (!fd) {
        DBG_PUT("open file %d failed: %d\r\n", fd, nand_errno);
        return;
    }

#define CHUNK_LENGTH 512

    uint8_t to_send[CHUNK_LENGTH];
    int file_size = fd->node.file_size;
    // DBG_PUT("File size: %d\r\n", file_size);
    int page_cnt = ((file_size + (2048 - 1)) / 2048);

    // below reads out a 2048 byte page, then splits it into 4 512 chunks to transmit over spi
    for (int count = 0; count < page_cnt; count++) {
        // DBG_PUT("Reading Page %d / %d.\r\n", count + 1, page_cnt);
        memset(page, 0, PAGE_DATA_SIZE);
        // read 2048B into buffer
        NANDfs_read(fd, PAGE_DATA_SIZE, page);
        for (int k = 0; k < 4; k++) {
            // DBG_PUT("\tReading Page %d Block %d / 4\r\n", count + 1, k + 1);
            for (int i = 0; i < CHUNK_LENGTH; i++) {
                to_send[i] = page[(CHUNK_LENGTH * k) + i];
                // DBG_PUT("0x%x\r\n", page[(512 * k) + i]);
            }
            // TRANSFER SPI CHUNK HERE i think
            spi_transmit(to_send, IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
        }
    }

    DBG_PUT("Image transfer finished");
    NANDfs_close(fd);
}
