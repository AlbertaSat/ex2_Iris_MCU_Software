#include <spi_command_handler.h>
#include <command_handler.h>
#include <iris_system.h>
#include <string.h>
#include <arducam.h>
#include <spi_bitbang.h>

extern SPI_HandleTypeDef hspi1;
extern uint8_t cam_to_nand_transfer_flag;

uint32_t image_length; // Only here for testing purposes
static uint32_t count = 0x0FFF0000;

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
 * 		Transmit data of given size over SPI bus in blocking mode
 *
 * @param
 * 		*tx_data: pointer to transmit data
 * 		data_length: numbers of bytes to be sent
 */
void spi_transmit_it(uint8_t *tx_data, uint16_t data_length) { HAL_SPI_Transmit_IT(&hspi1, tx_data, data_length); }

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
        // needs dedicated thought put towards implement
        char buf[64];
        write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, VIS_SENSOR); // VSYNC is active HIGH
        sprintf(buf, "Single Capture Transfer type %x\r\n", format);
        DBG_PUT(buf);
        flush_fifo(VIS_SENSOR);
        clear_fifo_flag(VIS_SENSOR);
        start_capture(VIS_SENSOR);
        while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, VIS_SENSOR)) {
        }

        DBG_PUT("JPG");

        //        iterate_image_num();
        // cam_to_nand_transfer_flag = 1;
        return 0;
    }
    case IRIS_GET_IMAGE_COUNT: {
        get_image_num_spi(0);
        spi_transmit(&tx_data, 1);
        return 0;
    }
    case IRIS_TRANSFER_IMAGE: {
        spi_transfer_image();
        return 0;
    }
    case IRIS_OFF_SENSOR_IDLE: {
        sensor_idle();
        return 0;
    }
    case IRIS_ON_SENSOR_IDLE: {
        sensor_active();
        DBG_PUT("Sensor activated\r\n");
        return 0;
    }
    case IRIS_GET_IMAGE_LENGTH: {
        image_length = read_fifo_length(VIS_SENSOR);
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

/**
 * @brief
 * 		Dummy function to dump image data to OBC
 */
void spi_transfer_image() {
    static uint8_t image_data[IRIS_IMAGE_TRANSFER_BLOCK_SIZE];
    static uint16_t num_transfers;

    uint8_t image_data_buffer[4574] = {
        // Offset 0x00000000 to 0x000011DD
        0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01,
        0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0x32, 0x22, 0x25, 0x2C, 0x25, 0x1F, 0x32, 0x2C, 0x29, 0x2C, 0x38,
        0x35, 0x32, 0x3B, 0x4B, 0x7D, 0x51, 0x4B, 0x45, 0x45, 0x4B, 0x99, 0x6D, 0x73, 0x5A, 0x7D, 0xB5, 0x9F, 0xBE,
        0xBB, 0xB2, 0x9F, 0xAF, 0xAC, 0xC8, 0xE1, 0xFF, 0xF3, 0xC8, 0xD4, 0xFF, 0xD7, 0xAC, 0xAF, 0xFA, 0xFF, 0xFD,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xDB, 0x00, 0x43, 0x01, 0x35, 0x38, 0x38, 0x4B, 0x42, 0x4B, 0x93, 0x51, 0x51, 0x93, 0xFF, 0xCE, 0xAF, 0xCE,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x11,
        0x08, 0x01, 0x00, 0x01, 0x00, 0x03, 0x01, 0x22, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF, 0xC4, 0x00,
        0x19, 0x00, 0x00, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xC4, 0x00, 0x36, 0x10, 0x00, 0x02, 0x02, 0x01, 0x03, 0x02, 0x05,
        0x03, 0x02, 0x04, 0x06, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x11, 0x03, 0x12, 0x21, 0x31, 0x41,
        0x51, 0x04, 0x13, 0x22, 0x61, 0x71, 0x32, 0x81, 0x91, 0x42, 0xA1, 0x14, 0x23, 0x33, 0xB1, 0x05, 0x52, 0xC1,
        0xD1, 0xE1, 0xF0, 0x62, 0x72, 0x34, 0x43, 0xF1, 0xFF, 0xC4, 0x00, 0x17, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0xFF, 0xC4, 0x00,
        0x18, 0x11, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x11, 0x01, 0x31, 0x21, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F,
        0x00, 0xF4, 0xC0, 0x00, 0x02, 0x85, 0x43, 0x00, 0x15, 0x05, 0x0C, 0x0A, 0x15, 0x21, 0xD0, 0x58, 0xAC, 0x06,
        0x04, 0xD8, 0xAC, 0x0A, 0xB1, 0x36, 0x4B, 0x90, 0xAC, 0x0A, 0x6C, 0x48, 0x2C, 0x2C, 0x0A, 0x43, 0x22, 0xC6,
        0x98, 0x14, 0x0C, 0x56, 0x30, 0x00, 0x13, 0xD9, 0x12, 0xE5, 0xB0, 0x03, 0x62, 0x42, 0xE4, 0x74, 0x56, 0x4C,
        0x40, 0x20, 0x18, 0x08, 0x0A, 0x18, 0x0A, 0xC0, 0x06, 0x02, 0xB0, 0xB0, 0x18, 0x80, 0x2C, 0x0D, 0x80, 0x9B,
        0x1D, 0x99, 0x68, 0xC0, 0x13, 0xB1, 0xD9, 0x02, 0x6E, 0x85, 0x62, 0x93, 0x33, 0x72, 0x28, 0xB6, 0xC5, 0x66,
        0x6E, 0x44, 0xB9, 0x95, 0x1A, 0xB9, 0x09, 0xC8, 0xCB, 0x58, 0xB5, 0x81, 0xA5, 0x8D, 0x33, 0x1D, 0x63, 0xD6,
        0x11, 0xB6, 0xA0, 0xB3, 0x2D, 0x63, 0xD6, 0x15, 0x63, 0x21, 0x4C, 0x6A, 0x40, 0x5A, 0xD8, 0xB4, 0xCC, 0xD3,
        0x1A, 0x02, 0xF9, 0xE4, 0x1D, 0x09, 0x30, 0xB2, 0x29, 0x3D, 0x89, 0xBB, 0x29, 0xBB, 0x24, 0xAC, 0xE8, 0x00,
        0x02, 0x80, 0x02, 0x86, 0x90, 0x08, 0x28, 0xB4, 0x92, 0x0B, 0x25, 0x22, 0x00, 0xAB, 0x44, 0xB2, 0x84, 0x03,
        0x1E, 0xC1, 0x05, 0x8F, 0xEE, 0x4E, 0x96, 0xB9, 0x15, 0x11, 0xA6, 0x89, 0xD7, 0x51, 0xEA, 0x46, 0x54, 0xC2,
        0x84, 0x2A, 0xED, 0x09, 0xA4, 0xC4, 0x2A, 0x05, 0x0E, 0x28, 0x97, 0x02, 0x80, 0x23, 0x3F, 0x2C, 0x5E, 0x5B,
        0x35, 0x0A, 0x28, 0xC7, 0xCB, 0x61, 0xA1, 0x9A, 0xD0, 0x50, 0x19, 0x68, 0x63, 0x58, 0xD9, 0xAD, 0x00, 0x46,
        0x7E, 0x58, 0xD4, 0x4B, 0x0A, 0x02, 0x68, 0x6A, 0xC6, 0x00, 0x16, 0xC7, 0x60, 0x01, 0x68, 0xB0, 0x00, 0x08,
        0x00, 0x00, 0x28, 0x1A, 0x62, 0x00, 0x1D, 0x8B, 0x90, 0x19, 0x01, 0x42, 0xA1, 0x80, 0x08, 0xAA, 0x10, 0x01,
        0xAC, 0x92, 0x7C, 0x99, 0x34, 0x53, 0x95, 0x82, 0x56, 0x4E, 0x2A, 0x40, 0xAA, 0x16, 0x96, 0x54, 0x89, 0x01,
        0xD0, 0x15, 0x08, 0x06, 0x00, 0x2A, 0x0A, 0x18, 0x50, 0x0A, 0x82, 0x86, 0x00, 0x2A, 0x0A, 0x18, 0x50, 0x08,
        0x07, 0x41, 0x40, 0x63, 0x9F, 0x24, 0xB1, 0xE3, 0x6E, 0x3C, 0x99, 0x78, 0x6C, 0xF3, 0x9E, 0x47, 0x19, 0x75,
        0xFD, 0x8B, 0xF1, 0x6B, 0xF9, 0x66, 0x1E, 0x17, 0xFA, 0xC8, 0x0E, 0xE0, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x10, 0x00, 0x00, 0x50, 0x00, 0x01, 0x00, 0x03, 0x00, 0x10, 0x0C, 0x00, 0x12, 0x29, 0x32, 0x6E, 0x86, 0x98,
        0x69, 0x48, 0x76, 0x45, 0x85, 0x92, 0x15, 0x62, 0x71, 0x44, 0xD8, 0x58, 0x0F, 0x4F, 0xB8, 0xB4, 0xFB, 0x85,
        0x85, 0x80, 0x55, 0x0A, 0x87, 0x62, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x39, 0xFC, 0x57,
        0xF4, 0xDA, 0x30, 0xF0, 0xBF, 0xD7, 0x46, 0xFE, 0x25, 0x37, 0x66, 0x5E, 0x19, 0x7F, 0x39, 0x01, 0xDA, 0x00,
        0x00, 0x00, 0x3A, 0x10, 0x00, 0x01, 0x94, 0xBC, 0x4E, 0x28, 0xDF, 0xAA, 0xEB, 0xB2, 0x03, 0x50, 0x30, 0x5E,
        0x2B, 0x13, 0xEA, 0xD7, 0xCA, 0x37, 0x4D, 0x34, 0x9A, 0xE1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x13, 0xB0, 0x6C, 0x3A, 0x06, 0x80, 0x56, 0x1A, 0x87, 0xA4, 0x28, 0x03, 0x50, 0x58, 0xF4, 0xA0, 0x48, 0x02,
        0xC0, 0x66, 0x59, 0x72, 0x55, 0xC1, 0x7D, 0x55, 0x61, 0x57, 0x63, 0x38, 0xAF, 0x4B, 0x4F, 0xCC, 0xE3, 0xA5,
        0x9D, 0xC1, 0x08, 0x06, 0x00, 0x23, 0x09, 0x78, 0x9F, 0x5A, 0x8C, 0x63, 0x77, 0xD4, 0xDA, 0x72, 0xD3, 0x06,
        0xFB, 0x23, 0xCD, 0x8E, 0xF9, 0xE2, 0xED, 0xF2, 0x07, 0x5B, 0xCD, 0xA9, 0x93, 0x1C, 0xB2, 0x8B, 0x74, 0x90,
        0x97, 0x3F, 0xF2, 0x3F, 0xCF, 0xE4, 0x0A, 0xC8, 0xDC, 0xA2, 0xDF, 0x56, 0x88, 0xC0, 0xAB, 0x2C, 0x7F, 0xEF,
        0x43, 0x46, 0xBF, 0x94, 0xDF, 0xB1, 0x82, 0xD7, 0x76, 0xA5, 0x49, 0x01, 0xDF, 0x42, 0x23, 0xC3, 0x49, 0xBC,
        0x56, 0xDE, 0xA7, 0x7C, 0x91, 0x9F, 0xC5, 0x79, 0x53, 0xD3, 0xA3, 0x56, 0xD7, 0x76, 0x41, 0xD0, 0x93, 0x13,
        0xDB, 0x93, 0x8B, 0xF8, 0xEC, 0x9F, 0xE5, 0x34, 0x8E, 0xAC, 0xB0, 0xD5, 0x2B, 0x8E, 0xA0, 0xAD, 0x3C, 0xFC,
        0x7D, 0x24, 0x9F, 0xB9, 0xC5, 0x92, 0x1A, 0x5B, 0x69, 0xA6, 0x9B, 0xE8, 0x29, 0xAD, 0x33, 0x94, 0x7A, 0x2E,
        0x82, 0xDE, 0x8A, 0xC8, 0x4B, 0x76, 0x7A, 0x98, 0x92, 0xF2, 0x61, 0xFF, 0x00, 0xAA, 0x3C, 0xB5, 0xF5, 0x33,
        0xA9, 0x78, 0xB8, 0x46, 0x0A, 0x2A, 0xF5, 0x24, 0x35, 0x71, 0xD9, 0x2A, 0x33, 0x79, 0x21, 0x17, 0x4E, 0x48,
        0xE2, 0x96, 0x79, 0x65, 0xF4, 0xCA, 0xAB, 0x91, 0x36, 0xD4, 0x9B, 0x5C, 0xB2, 0x71, 0x5D, 0xE9, 0xA7, 0xC3,
        0x19, 0xC1, 0x87, 0x22, 0xC5, 0x91, 0x4A, 0x5B, 0x27, 0xB3, 0x35, 0xCD, 0xE3, 0x23, 0x08, 0x5E, 0x3A, 0x6E,
        0xF8, 0x60, 0x75, 0x01, 0xE6, 0x7F, 0x15, 0x2B, 0x6E, 0x96, 0xEA, 0x8E, 0xEF, 0x0D, 0x91, 0xE4, 0xC2, 0x9B,
        0x54, 0x54, 0x6A, 0x06, 0x7E, 0x74, 0x3B, 0xAF, 0xC9, 0xA9, 0x14, 0x5F, 0xB0, 0x26, 0xBB, 0x19, 0x64, 0xCB,
        0xA5, 0xE9, 0x8A, 0xB6, 0x11, 0xC8, 0xD4, 0x7D, 0x7C, 0xF6, 0x03, 0x47, 0x47, 0x37, 0x8C, 0x75, 0xA3, 0xEE,
        0x74, 0xAA, 0x68, 0xE5, 0xF1, 0x6F, 0xD5, 0x1F, 0x8F, 0x90, 0xB9, 0xD7, 0x2C, 0x96, 0xFF, 0x00, 0xF2, 0x29,
        0x5A, 0xE1, 0xF2, 0x57, 0xEA, 0x5F, 0x1D, 0x89, 0x9F, 0x31, 0xFF, 0x00, 0x6A, 0x23, 0x7B, 0xC4, 0xA9, 0x33,
        0xB3, 0xC3, 0x67, 0x9C, 0xA5, 0x18, 0xCF, 0x87, 0xC1, 0xC9, 0x8E, 0x3E, 0x64, 0x94, 0x53, 0xAA, 0x5B, 0xD9,
        0xD7, 0x8E, 0x30, 0xC7, 0x38, 0xBE, 0xC5, 0x73, 0x75, 0x81, 0x12, 0xCB, 0x14, 0x97, 0x79, 0x70, 0x38, 0xCE,
        0x33, 0x74, 0xB9, 0x5B, 0x81, 0x39, 0xF6, 0xC1, 0x3F, 0x83, 0xCF, 0xC4, 0xDB, 0x94, 0x5F, 0x69, 0x9E, 0x87,
        0x88, 0xFE, 0x84, 0xFE, 0x0F, 0x36, 0x0E, 0xB1, 0xCB, 0x7E, 0xA1, 0x1D, 0x32, 0xB5, 0xBA, 0x5C, 0x7B, 0x0E,
        0x29, 0x49, 0x5D, 0x6D, 0xEF, 0x12, 0x64, 0xFD, 0x0F, 0x82, 0x30, 0x3B, 0x87, 0x28, 0x95, 0xA8, 0xE9, 0x6F,
        0xF9, 0x4E, 0xBA, 0x76, 0x39, 0x33, 0x3F, 0x45, 0x7B, 0x9A, 0x46, 0x5F, 0xD5, 0xDF, 0x8A, 0x32, 0xC8, 0xE2,
        0xE3, 0xCF, 0xE0, 0xD3, 0x2E, 0xCF, 0x01, 0xFF, 0x00, 0xC6, 0xFB, 0xB3, 0x2F, 0x19, 0xBE, 0x5B, 0x5C, 0x50,
        0xB0, 0xB7, 0x1F, 0x0F, 0x0A, 0x75, 0x72, 0x36, 0xBD, 0x99, 0x2A, 0xC7, 0x2E, 0x08, 0xC6, 0xDC, 0xE7, 0xD3,
        0x84, 0xCE, 0x9F, 0x3A, 0xB8, 0xFC, 0x18, 0xE4, 0xFF, 0x00, 0x52, 0x5B, 0x5A, 0x57, 0xFE, 0xC3, 0xA7, 0x0B,
        0x2B, 0xBC, 0xB2, 0x7D, 0xC8, 0x26, 0x72, 0xA9, 0x0E, 0xD1, 0x59, 0xD1, 0x7B, 0xFE, 0x0C, 0xAD, 0xBC, 0x8F,
        0x6D, 0xAC, 0xD3, 0xFE, 0x0B, 0x6D, 0xE8, 0xFB, 0x77, 0x06, 0x27, 0x1B, 0xF5, 0xB5, 0xBD, 0x51, 0xB3, 0x22,
        0x10, 0x8C, 0x56, 0xA6, 0xDB, 0x97, 0x6E, 0x85, 0xB2, 0x6B, 0x58, 0x20, 0xD4, 0x77, 0x6A, 0xF6, 0xEA, 0x13,
        0xF2, 0xF2, 0x45, 0x6A, 0x4F, 0xEC, 0x44, 0x9D, 0x45, 0x82, 0xA9, 0x47, 0xD3, 0xD5, 0x75, 0x03, 0x14, 0xB5,
        0x34, 0xBB, 0x9D, 0xB8, 0xB2, 0x3C, 0x18, 0x54, 0x52, 0x52, 0x39, 0x63, 0x8A, 0x7A, 0xAE, 0xB8, 0x36, 0x6B,
        0xD2, 0xB6, 0x5F, 0xD8, 0xBA, 0x66, 0x2F, 0x64, 0xA3, 0xBE, 0xEF, 0xA5, 0x1D, 0xC7, 0x9B, 0x19, 0xEA, 0x51,
        0xD3, 0x76, 0xBB, 0xB2, 0xBF, 0x8A, 0x9C, 0x92, 0x8D, 0x6F, 0x7B, 0x33, 0x2B, 0xAD, 0x14, 0xB5, 0x78, 0x9E,
        0x6F, 0xD4, 0x3C, 0xFF, 0x00, 0x5F, 0x4F, 0xBC, 0xA8, 0x58, 0xD4, 0x9E, 0x99, 0x28, 0xDF, 0xB8, 0x4B, 0x1E,
        0x59, 0x3B, 0xAD, 0x57, 0xF0, 0x05, 0xC9, 0xD7, 0x84, 0xB4, 0xEB, 0x7E, 0x87, 0x3E, 0x5F, 0x4A, 0x8A, 0xEF,
        0xDC, 0x79, 0x31, 0x67, 0x4D, 0xE9, 0xDE, 0x3D, 0x93, 0x31, 0x79, 0x25, 0x37, 0xA6, 0x4A, 0x9A, 0x55, 0x40,
        0xCD, 0x8D, 0xA3, 0x8B, 0x7B, 0x97, 0x1E, 0xC6, 0x7E, 0x22, 0x0E, 0x29, 0x49, 0x2D, 0xAF, 0x82, 0x25, 0x92,
        0x4E, 0x2A, 0x3D, 0xBA, 0xA2, 0xF1, 0x5E, 0x48, 0xFA, 0x9C, 0x9A, 0x5D, 0x96, 0xE0, 0xA9, 0xC5, 0x71, 0x83,
        0x69, 0x54, 0xDF, 0x57, 0xD8, 0xD5, 0x64, 0xB6, 0xE9, 0x7E, 0xC4, 0x64, 0xC5, 0x28, 0x35, 0x72, 0xE4, 0x72,
        0xC3, 0x91, 0x4A, 0xB9, 0x5D, 0xD2, 0x2A, 0x36, 0x8E, 0x47, 0x25, 0x17, 0x2D, 0xB4, 0xB2, 0xA3, 0x3D, 0x12,
        0x6E, 0x32, 0x8E, 0xEB, 0xA9, 0xCC, 0xEE, 0x32, 0xD2, 0x9B, 0x6B, 0xDD, 0x6E, 0x53, 0x7C, 0xF3, 0xF6, 0x22,
        0xBA, 0x72, 0xE6, 0x8C, 0xB0, 0x49, 0x36, 0xB5, 0x35, 0xD0, 0xE0, 0x8F, 0xF4, 0xE5, 0xF2, 0x6B, 0x91, 0xBF,
        0x2E, 0x5C, 0xFD, 0xCE, 0x6C, 0x4D, 0xB5, 0x2B, 0xFD, 0xCA, 0x8E, 0xC9, 0x7D, 0x2E, 0xDA, 0x49, 0xF6, 0xDC,
        0xCA, 0x29, 0x45, 0xF2, 0x3C, 0x92, 0xB8, 0x74, 0xDB, 0xB1, 0x0E, 0xDB, 0x5D, 0xD9, 0x15, 0x69, 0xFA, 0x66,
        0xAF, 0x9D, 0xAE, 0x8C, 0x7A, 0x1D, 0x3E, 0x4C, 0xFC, 0xB4, 0xF4, 0xAF, 0x57, 0x5B, 0x30, 0xCD, 0x86, 0x78,
        0x54, 0x75, 0x6F, 0x7D, 0x80, 0xE9, 0x82, 0x4B, 0x04, 0x15, 0xF5, 0x34, 0xE8, 0xCE, 0x18, 0x49, 0xA9, 0x2F,
        0x63, 0xA2, 0x39, 0xAE, 0xD5, 0x7B, 0x72, 0x03, 0xC9, 0xFE, 0xA4, 0xCB, 0xE9, 0xE3, 0xA9, 0x33, 0x9A, 0xD4,
        0xAE, 0xF7, 0x60, 0xDD, 0xC7, 0x9E, 0xA5, 0x46, 0x39, 0x3E, 0xA3, 0x48, 0xE2, 0x73, 0xA6, 0xDD, 0x22, 0xB1,
        0xB4, 0xAD, 0xF5, 0x5D, 0x47, 0x8F, 0xEB, 0x14, 0x89, 0x78, 0x9A, 0x7B, 0x6E, 0x89, 0x7B, 0x2D, 0xCE, 0x8E,
        0xA6, 0x53, 0xC9, 0x8A, 0xED, 0xA6, 0xDB, 0xE4, 0x52, 0x43, 0x4F, 0xFB, 0x0E, 0x57, 0x44, 0x29, 0x42, 0x4F,
        0xD0, 0xDF, 0x1D, 0x4A, 0x97, 0x1C, 0x85, 0x46, 0x47, 0xE9, 0xF9, 0x2F, 0x17, 0xD2, 0x97, 0xB7, 0xC9, 0x13,
        0x5A, 0x95, 0x5A, 0xFB, 0x86, 0x3D, 0xA9, 0x10, 0x68, 0xDF, 0xF3, 0x14, 0x6B, 0x6A, 0xEC, 0x39, 0x47, 0xD3,
        0xD2, 0xBD, 0xC5, 0x6F, 0xA1, 0xA2, 0xFE, 0x9D, 0xF5, 0xEE, 0x07, 0x36, 0x1C, 0x8E, 0x1F, 0x2C, 0xEA, 0xC7,
        0x8A, 0x3E, 0x1A, 0x0F, 0x26, 0x59, 0x5B, 0x7B, 0xD5, 0x07, 0x87, 0xC3, 0x1F, 0x0F, 0x0F, 0x37, 0x22, 0xF5,
        0x7E, 0x95, 0xD8, 0xC2, 0x79, 0x72, 0x65, 0x9D, 0xEC, 0xD3, 0x7C, 0x35, 0x74, 0x11, 0xDB, 0x14, 0xF2, 0xB5,
        0x27, 0x26, 0xA2, 0xBA, 0x51, 0x8E, 0x7C, 0xCE, 0x7E, 0x98, 0x3A, 0x8A, 0x75, 0xB7, 0x53, 0x5C, 0xEF, 0xCA,
        0xF0, 0xAE, 0x3D, 0x6B, 0xA1, 0xC9, 0xCC, 0x7D, 0xA9, 0x05, 0x68, 0xEE, 0x71, 0xBD, 0xD4, 0xD7, 0x0D, 0x3A,
        0xB4, 0x12, 0xFE, 0x64, 0x7D, 0x6B, 0x4B, 0xE9, 0x3E, 0xDF, 0x22, 0x4D, 0x24, 0x9F, 0xC0, 0xED, 0x46, 0x6D,
        0x39, 0x2A, 0x7B, 0xD0, 0x1C, 0xD9, 0x31, 0x4A, 0x0D, 0xEA, 0xE4, 0xE9, 0xF0, 0x8A, 0x51, 0x4D, 0x49, 0x6F,
        0xB7, 0xF7, 0x14, 0xE3, 0x19, 0xA5, 0x8F, 0x52, 0xDB, 0x78, 0xBF, 0xF4, 0x1E, 0x1C, 0xAB, 0xCC, 0x71, 0x7B,
        0x4B, 0x65, 0xB8, 0x43, 0xF1, 0x4F, 0x78, 0xFC, 0x9D, 0x09, 0xFF, 0x00, 0xA9, 0x87, 0x88, 0x7B, 0xC6, 0xFB,
        0x9B, 0x2E, 0x9F, 0x70, 0xAE, 0x2F, 0x10, 0xA5, 0xFC, 0x42, 0x75, 0xE9, 0xA5, 0xFD, 0x86, 0xDD, 0xDF, 0x1F,
        0x9A, 0x1E, 0x57, 0x5E, 0x26, 0x0B, 0xA6, 0xDC, 0x8F, 0x2E, 0x57, 0x8A, 0x69, 0x5A, 0x7F, 0x08, 0x08, 0xC9,
        0xB4, 0x1F, 0x07, 0x3B, 0x95, 0x71, 0xC9, 0xDE, 0xAE, 0x6A, 0x2F, 0x52, 0x6A, 0x5E, 0xC6, 0x52, 0x8A, 0x94,
        0x72, 0xDA, 0x8A, 0xD1, 0xEC, 0x11, 0xCF, 0x17, 0x70, 0x76, 0xBA, 0x9B, 0xAC, 0x75, 0x38, 0xEF, 0xC2, 0x4F,
        0x83, 0x0D, 0x9F, 0xC9, 0x70, 0x93, 0x52, 0xE7, 0x61, 0x47, 0xA1, 0x1D, 0xF0, 0x63, 0xFB, 0x99, 0xF8, 0xAA,
        0xAC, 0x69, 0xDF, 0x1D, 0xE8, 0xB8, 0x34, 0xF0, 0x63, 0x5B, 0x5E, 0xFB, 0x18, 0x7F, 0x88, 0x45, 0xC9, 0x62,
        0x49, 0xD3, 0xA0, 0xAC, 0xE5, 0x1F, 0x4B, 0xD9, 0xED, 0xDE, 0x56, 0x44, 0x3F, 0x57, 0xC8, 0x96, 0x29, 0xC5,
        0xFA, 0xE4, 0x92, 0xFE, 0xE5, 0x28, 0xB4, 0xDF, 0x67, 0x7B, 0xA0, 0x62, 0x33, 0x6D, 0x5B, 0x93, 0xAE, 0xE2,
        0x91, 0x79, 0x5D, 0x6D, 0xA7, 0x67, 0xDC, 0xC9, 0x57, 0xC0, 0x37, 0xAD, 0xF1, 0xA8, 0x4B, 0x78, 0xCA, 0x9B,
        0xEE, 0x3C, 0x5F, 0x51, 0xCD, 0xFA, 0x92, 0x5B, 0x9D, 0x18, 0xEA, 0x2E, 0xDF, 0x40, 0x63, 0x6E, 0xA7, 0x13,
        0xD4, 0x9B, 0x6B, 0x8B, 0x3B, 0x53, 0xB7, 0xD0, 0xE4, 0x6E, 0xA7, 0x4E, 0xD3, 0xF7, 0x06, 0xAF, 0x1D, 0x45,
        0x5B, 0x7C, 0xF6, 0x2E, 0x53, 0xC7, 0x55, 0xBD, 0x98, 0xCB, 0xBB, 0xB6, 0xBD, 0x91, 0x31, 0x49, 0xC9, 0x95,
        0x1D, 0x1B, 0x49, 0x7A, 0x6B, 0xEE, 0x64, 0xE3, 0x28, 0x27, 0xEA, 0x56, 0xBB, 0x14, 0xA4, 0xF1, 0x46, 0x9C,
        0x5D, 0xBE, 0xE6, 0x8E, 0x74, 0xDA, 0xA7, 0xB1, 0x15, 0x18, 0xDD, 0xAB, 0x6C, 0xE8, 0x5F, 0xD1, 0x30, 0x8C,
        0x74, 0xDC, 0x55, 0xEC, 0xAF, 0x74, 0x69, 0xAD, 0xAC, 0x6D, 0x72, 0xD7, 0xB0, 0x0B, 0x3F, 0x88, 0x94, 0xF2,
        0xB6, 0xA6, 0xE3, 0x15, 0xD8, 0xAF, 0x05, 0x29, 0xCF, 0x34, 0x56, 0xB9, 0x34, 0xB7, 0x67, 0x3C, 0xB2, 0xBD,
        0x52, 0xDA, 0x3B, 0x7F, 0xE2, 0x8E, 0xDF, 0xF0, 0xF6, 0xE5, 0x8E, 0x52, 0x92, 0x8A, 0xE9, 0xB2, 0xAF, 0xFB,
        0xD0, 0x21, 0x78, 0xAC, 0x90, 0x96, 0x5D, 0x32, 0xBD, 0xBB, 0x33, 0x3A, 0xC7, 0x69, 0x7A, 0xBF, 0x26, 0x33,
        0x9E, 0xAC, 0x9A, 0xAF, 0x99, 0x37, 0xF6, 0x05, 0x25, 0x27, 0x0E, 0x97, 0xBF, 0xFD, 0xFC, 0x01, 0xD0, 0xA3,
        0x8B, 0xBC, 0xED, 0x32, 0x27, 0x8D, 0x4F, 0x4B, 0xF5, 0x76, 0xD8, 0x95, 0x3D, 0x2B, 0xE1, 0x1A, 0xA9, 0xAF,
        0x2E, 0xDD, 0xF3, 0xD0, 0x82, 0x23, 0x8D, 0x28, 0xDD, 0xC9, 0xD6, 0xCF, 0x61, 0xD2, 0xC9, 0x1D, 0x7F, 0xA9,
        0x73, 0xEE, 0x56, 0x09, 0x45, 0xF1, 0x7B, 0x84, 0x9A, 0xC6, 0xE3, 0xCD, 0xFB, 0x95, 0x49, 0xE1, 0x73, 0x8A,
        0x51, 0xC8, 0x9C, 0x96, 0xF4, 0xC6, 0xE7, 0x28, 0x2F, 0x54, 0xDF, 0xC9, 0x8C, 0xE2, 0xB1, 0x64, 0x6D, 0x5D,
        0x35, 0xB3, 0x66, 0x59, 0x24, 0xE4, 0xB6, 0xDC, 0xC8, 0xDF, 0x4A, 0xC8, 0xEF, 0x52, 0x6F, 0xDC, 0x9F, 0x2B,
        0x94, 0xD3, 0x6F, 0xB9, 0x84, 0x25, 0x25, 0xC1, 0xD1, 0x0C, 0x89, 0xDE, 0xAD, 0x91, 0x3D, 0xC1, 0x4B, 0x1A,
        0xC7, 0x16, 0xD2, 0x7A, 0x97, 0x1B, 0x91, 0xE6, 0x46, 0x58, 0x9C, 0x74, 0xFA, 0x9E, 0xCC, 0xA7, 0x93, 0x1A,
        0x7A, 0x5D, 0xDB, 0xEA, 0x67, 0x49, 0x5B, 0x5F, 0xB1, 0x68, 0x89, 0x63, 0xA9, 0x69, 0x8D, 0xF0, 0x2F, 0x2E,
        0x6A, 0x37, 0x1A, 0xF8, 0x46, 0xD1, 0x9B, 0x4E, 0x9B, 0x1E, 0xB4, 0xD3, 0x42, 0xA3, 0x3F, 0x0F, 0x39, 0x79,
        0x91, 0xD5, 0xB2, 0xEE, 0x75, 0xF8, 0x95, 0x8F, 0x2C, 0x63, 0x27, 0x26, 0xB4, 0xAD, 0xAB, 0xA9, 0xCB, 0xA5,
        0xED, 0x4E, 0xCA, 0xCB, 0x27, 0xA2, 0x31, 0x4A, 0x3B, 0x05, 0x4E, 0x4C, 0xCB, 0x55, 0x25, 0xB7, 0x35, 0xD8,
        0xA4, 0xA4, 0xD3, 0x94, 0x56, 0xF7, 0x46, 0x5A, 0xAB, 0x7A, 0x56, 0x68, 0xA7, 0x1D, 0x0F, 0x53, 0xE1, 0x6C,
        0x51, 0xD3, 0x99, 0xE3, 0xDA, 0xA3, 0x74, 0xF7, 0xEB, 0x66, 0x32, 0x58, 0xF4, 0xBD, 0x2A, 0x9B, 0x33, 0xD7,
        0xA9, 0x33, 0x34, 0xFC, 0xBB, 0xD4, 0x93, 0x7F, 0x20, 0x69, 0x70, 0xAA, 0x92, 0xA6, 0xBB, 0x19, 0x5D, 0x36,
        0x93, 0xE4, 0xA9, 0x65, 0x4D, 0x71, 0xB8, 0xA1, 0x04, 0xD5, 0xC9, 0xF3, 0xD0, 0x23, 0x48, 0x2B, 0x4B, 0x4C,
        0xB5, 0x3E, 0xC2, 0x9E, 0x2A, 0x95, 0xBB, 0x4F, 0xB3, 0x07, 0x08, 0xF6, 0xAA, 0xE1, 0xA1, 0x46, 0x5C, 0xB7,
        0x6D, 0xF7, 0x7D, 0x42, 0xB5, 0x8C, 0xD4, 0x62, 0xA1, 0xBB, 0x68, 0xCA, 0x58, 0xE7, 0xAD, 0xDC, 0x24, 0xCA,
        0xD6, 0xB5, 0x26, 0xFA, 0x1B, 0x2C, 0xB5, 0xF5, 0x63, 0x8F, 0xCC, 0x76, 0x03, 0x9F, 0x27, 0x99, 0x39, 0x26,
        0xE1, 0x25, 0x5C, 0x6C, 0x6D, 0x91, 0x36, 0xE7, 0xB5, 0xF0, 0x5A, 0xCD, 0x1A, 0xFF, 0x00, 0xEC, 0x8F, 0xC3,
        0xB2, 0x96, 0x58, 0xBE, 0x72, 0x7D, 0xA5, 0x12, 0x8C, 0xE5, 0x0D, 0xF2, 0x6D, 0xCC, 0x45, 0xBE, 0x9C, 0x9E,
        0xD1, 0x46, 0xEE, 0x49, 0xAD, 0x9E, 0x37, 0xF7, 0xA2, 0x5C, 0xB5, 0x5F, 0xF4, 0xFD, 0xFD, 0x40, 0x71, 0x42,
        0x38, 0x9C, 0x5D, 0xE5, 0x6A, 0xDF, 0xF9, 0x4F, 0x4A, 0x09, 0x61, 0xF0, 0x4D, 0xC5, 0xDE, 0xCD, 0xD9, 0xE5,
        0x46, 0x0D, 0xE9, 0x4D, 0x35, 0xB9, 0xEA, 0x78, 0xB4, 0xE1, 0xE0, 0x94, 0x62, 0xAF, 0x64, 0x82, 0x3C, 0xE9,
        0x6D, 0xBF, 0xF9, 0x76, 0x0B, 0xD3, 0xAB, 0xFF, 0x00, 0x12, 0xBC, 0xA9, 0x39, 0x4D, 0x74, 0x74, 0x39, 0x62,
        0x6D, 0xBE, 0x1A, 0x97, 0xFA, 0x0A, 0x12, 0x57, 0x27, 0x7C, 0x29, 0x2B, 0x37, 0x51, 0x4F, 0x03, 0x52, 0x95,
        0x2E, 0x5B, 0xE6, 0xB7, 0x32, 0x50, 0x93, 0xED, 0xBA, 0x7D, 0x4D, 0xF1, 0x6D, 0x8D, 0xEA, 0xDF, 0x6B, 0xD8,
        0x09, 0xC3, 0xA6, 0x2D, 0xD6, 0x44, 0xE9, 0xEF, 0xB5, 0x1A, 0x3C, 0x71, 0x97, 0x32, 0x4E, 0x99, 0x8C, 0x94,
        0x67, 0x6E, 0x16, 0x9B, 0x57, 0xC1, 0xD1, 0x08, 0xE9, 0x86, 0xEF, 0x90, 0xAC, 0xF2, 0x63, 0xD5, 0x1D, 0x0B,
        0x76, 0xB7, 0x8B, 0x38, 0xE1, 0x69, 0x3D, 0x71, 0x7F, 0x83, 0xD0, 0x92, 0xDA, 0xD7, 0x2B, 0x80, 0xF4, 0xC9,
        0x29, 0x35, 0xB3, 0xE8, 0x80, 0xF3, 0xE4, 0xD4, 0x25, 0x4C, 0x52, 0x95, 0x26, 0xFB, 0x9A, 0x66, 0x8C, 0x56,
        0x46, 0x9A, 0xD9, 0x70, 0xB8, 0x23, 0x26, 0x3D, 0x55, 0xA1, 0x46, 0x97, 0xB9, 0x11, 0x9C, 0x5E, 0xA9, 0x7E,
        0xE6, 0x8A, 0x4D, 0x2F, 0x82, 0x63, 0x8E, 0x5A, 0xB8, 0xE7, 0xB1, 0xAC, 0x56, 0x95, 0x4E, 0x0F, 0x57, 0x7A,
        0x03, 0x37, 0x39, 0xA9, 0xED, 0x1E, 0x7D, 0x83, 0xCD, 0x9F, 0xF9, 0x3F, 0x63, 0x79, 0x54, 0x2B, 0xD0, 0xF7,
        0x2A, 0x38, 0xDB, 0x5A, 0xA1, 0x74, 0xBF, 0x4A, 0x03, 0x28, 0xB9, 0xC9, 0x7D, 0x24, 0x4B, 0x24, 0x9C, 0x77,
        0x8F, 0x26, 0xEA, 0x39, 0x53, 0x7F, 0xCB, 0x7F, 0x83, 0x4D, 0x0E, 0x93, 0x92, 0x5F, 0x8E, 0x00, 0xE4, 0x51,
        0x9C, 0xEF, 0x4C, 0x78, 0x25, 0xE3, 0x9D, 0x25, 0xA5, 0x9D, 0xB4, 0xD6, 0xDA, 0x92, 0x32, 0x9C, 0xDC, 0x67,
        0x5A, 0xBE, 0x4A, 0x46, 0x4A, 0x32, 0x87, 0xD4, 0xB9, 0xFD, 0x8D, 0x9E, 0x28, 0x52, 0x7A, 0xB6, 0xBE, 0xD6,
        0x38, 0x4D, 0xC9, 0xFD, 0x5F, 0x91, 0xCB, 0x53, 0xD9, 0xFE, 0x42, 0x97, 0x95, 0x8E, 0x95, 0x35, 0xBB, 0xE5,
        0x20, 0x58, 0x22, 0xE7, 0xB6, 0x5D, 0xFB, 0x50, 0x93, 0xA7, 0x5B, 0x7C, 0x94, 0x9B, 0x4E, 0xF5, 0x2B, 0x4F,
        0xA8, 0x12, 0xF1, 0x26, 0x9B, 0x59, 0x22, 0xEB, 0xBA, 0x62, 0x50, 0x96, 0xAF, 0xA9, 0x3A, 0xE8, 0x68, 0xD4,
        0xBB, 0xA4, 0x62, 0xE7, 0x2B, 0xBD, 0xBF, 0x24, 0x03, 0xC4, 0xED, 0x7A, 0x91, 0x7A, 0x62, 0xED, 0x27, 0x55,
        0xDC, 0x5E, 0x65, 0xB5, 0xD3, 0x7D, 0xCA, 0x82, 0x53, 0x4E, 0x9F, 0x20, 0xC4, 0xB8, 0x3F, 0x9F, 0x8D, 0xC1,
        0xA6, 0x91, 0x4E, 0x12, 0x4D, 0xAA, 0xDF, 0xD8, 0x4D, 0xC9, 0x24, 0x9D, 0xD0, 0x51, 0x25, 0xE9, 0xE0, 0x85,
        0x15, 0x1E, 0x11, 0xAA, 0x95, 0xC5, 0x5A, 0x5F, 0x51, 0x92, 0xC9, 0xE6, 0x41, 0xBD, 0x31, 0x8D, 0x35, 0xC0,
        0x1B, 0x62, 0xCD, 0xAB, 0x24, 0x61, 0xE5, 0xC3, 0x7E, 0xB4, 0x75, 0x78, 0xD7, 0xE8, 0x8A, 0xEE, 0xCE, 0x7F,
        0x0D, 0x86, 0xB3, 0xC5, 0xB7, 0xC7, 0x44, 0x74, 0xF8, 0xAA, 0xB8, 0xDF, 0xB9, 0x51, 0xC7, 0x55, 0x8E, 0xF6,
        0x4D, 0x0B, 0x0E, 0xF8, 0xD3, 0x96, 0xDA, 0x5F, 0xE4, 0x5E, 0x2A, 0x6E, 0x30, 0x51, 0xE8, 0xF9, 0x48, 0x8F,
        0x05, 0xF5, 0x35, 0xCB, 0x68, 0x21, 0xE2, 0x96, 0xBC, 0xCD, 0x6F, 0xA6, 0x3C, 0x34, 0xBA, 0x1D, 0x10, 0x51,
        0x8A, 0x49, 0x1C, 0xF8, 0x22, 0x97, 0x8C, 0xCA, 0xA4, 0xF4, 0xD1, 0xD7, 0x78, 0xE2, 0xAF, 0x4D, 0xA5, 0xD6,
        0x41, 0x49, 0x5B, 0xE3, 0xF6, 0x29, 0x63, 0x9F, 0x2E, 0xA3, 0xEE, 0xCC, 0xE7, 0xE2, 0xE3, 0x1D, 0x94, 0x9B,
        0xF6, 0x8E, 0xC8, 0xE6, 0x9F, 0x8B, 0x9B, 0xFA, 0x52, 0x5E, 0xFC, 0x81, 0xDC, 0xA0, 0xAF, 0x79, 0xDF, 0xB2,
        0x56, 0x4D, 0xC6, 0x36, 0xA3, 0x2E, 0x7A, 0x37, 0xD4, 0xF3, 0x65, 0x93, 0x24, 0xFE, 0xA9, 0xB6, 0xBB, 0x36,
        0x38, 0x7A, 0x40, 0xE8, 0x92, 0xC0, 0xA4, 0xE4, 0xDB, 0x93, 0xF6, 0x56, 0x1E, 0x76, 0x28, 0xAB, 0x8E, 0x26,
        0xFD, 0xDE, 0xC1, 0xAD, 0x4B, 0x75, 0x7B, 0xF2, 0x8C, 0x25, 0x2A, 0x8B, 0x87, 0xE9, 0x4F, 0xA0, 0x03, 0xCA,
        0xFC, 0xEB, 0x8C, 0x12, 0x7D, 0x99, 0x6F, 0x3E, 0x66, 0xB9, 0xAF, 0x84, 0x60, 0xD3, 0xE6, 0xFE, 0xE6, 0xF8,
        0x5B, 0x51, 0xB6, 0xEA, 0xFF, 0x00, 0x70, 0x23, 0x54, 0xE7, 0xF5, 0x39, 0x3F, 0x92, 0xE2, 0xA6, 0xB8, 0x52,
        0x35, 0x73, 0x94, 0x5F, 0x1B, 0x76, 0x17, 0x9B, 0x7D, 0x6B, 0xDC, 0x08, 0x4B, 0x27, 0xB8, 0xE3, 0x2C, 0x9A,
        0xA9, 0x39, 0x17, 0xAD, 0xBD, 0xEE, 0x91, 0xCF, 0xE6, 0xB5, 0x91, 0xB5, 0x7F, 0x24, 0x47, 0x4B, 0x96, 0x49,
        0xEC, 0xDB, 0xA5, 0xDC, 0xE5, 0xC8, 0xDE, 0xBE, 0xFF, 0x00, 0x06, 0x8E, 0x6D, 0xC5, 0xEE, 0x60, 0xE4, 0xF5,
        0xD9, 0x30, 0x5E, 0x39, 0x49, 0xCA, 0xD7, 0xEE, 0x76, 0xC7, 0x75, 0x74, 0x8E, 0x6C, 0x6E, 0x0F, 0x23, 0x7B,
        0x55, 0xD6, 0xC7, 0x56, 0x59, 0xE3, 0x8E, 0x39, 0x56, 0xCD, 0x2E, 0x13, 0x2A, 0x93, 0x8A, 0x33, 0xC9, 0xE9,
        0x7D, 0x2B, 0xA9, 0x38, 0xFC, 0x42, 0x4F, 0xD6, 0x9B, 0x8F, 0xB7, 0x41, 0x67, 0xCB, 0x09, 0x49, 0xA8, 0xF0,
        0xFB, 0x81, 0x0B, 0x32, 0x53, 0x71, 0xE9, 0x7B, 0x31, 0x3A, 0xA3, 0x36, 0x95, 0xEF, 0xD7, 0xF6, 0x2E, 0x29,
        0x28, 0xEE, 0xF7, 0x22, 0x1D, 0xDC, 0x5A, 0xFD, 0xC5, 0x8D, 0xCA, 0x32, 0x7B, 0xD5, 0xF6, 0x17, 0x4E, 0x43,
        0x1B, 0xE4, 0x2B, 0xA2, 0x39, 0x32, 0x27, 0xDD, 0x15, 0x1C, 0xE9, 0xCA, 0x4A, 0x6B, 0x6E, 0x89, 0x18, 0x39,
        0xDA, 0x6B, 0xF7, 0x32, 0x52, 0x7B, 0xD6, 0xE0, 0x77, 0x39, 0x62, 0x6A, 0xA2, 0xDD, 0x27, 0x66, 0x71, 0xC5,
        0x8E, 0x31, 0x69, 0x49, 0xD3, 0x39, 0xE0, 0xE4, 0xA7, 0xBE, 0xCB, 0xB1, 0x59, 0x26, 0xEA, 0xBA, 0x16, 0x8F,
        0x4B, 0xC3, 0xC5, 0xAC, 0xAA, 0xD3, 0x43, 0xF1, 0x69, 0xBD, 0xD3, 0x5B, 0x2E, 0xA2, 0xC5, 0x93, 0xEA, 0xC9,
        0x25, 0x4E, 0x31, 0xE1, 0x23, 0x93, 0xC4, 0x78, 0x95, 0x92, 0x4A, 0x54, 0xFE, 0x0A, 0x0C, 0xF3, 0xC7, 0x17,
        0xBA, 0x8B, 0x75, 0xC9, 0x3E, 0x15, 0xC6, 0x52, 0xB8, 0xC2, 0x31, 0x6B, 0x8D, 0xEC, 0xCF, 0x2C, 0xE3, 0x92,
        0x3C, 0x55, 0x15, 0xE1, 0x9A, 0x8C, 0xEB, 0x9F, 0x7A, 0xB0, 0x2E, 0x33, 0x4B, 0xC4, 0x4F, 0x65, 0x7B, 0xEF,
        0xDC, 0x32, 0xCD, 0x4B, 0x79, 0x46, 0xFE, 0x4C, 0xD4, 0x97, 0xF1, 0x33, 0x7F, 0x23, 0x9B, 0xD4, 0x01, 0x3D,
        0x29, 0x7D, 0x08, 0xC2, 0x94, 0x9B, 0xDE, 0xBD, 0x8D, 0x1E, 0xE4, 0x4D, 0x57, 0x4F, 0xB8, 0x09, 0x25, 0xD1,
        0xBB, 0x29, 0x24, 0xB9, 0x64, 0xA9, 0x52, 0xE0, 0x1C, 0x93, 0x5B, 0x7C, 0x99, 0xA8, 0xD7, 0x52, 0x50, 0xBB,
        0x27, 0x55, 0x75, 0x5F, 0x04, 0x7E, 0x9E, 0x76, 0xEC, 0x5A, 0xC6, 0xD6, 0x1D, 0x4D, 0x27, 0xBF, 0x1D, 0x4A,
        0xA9, 0x71, 0x8B, 0x96, 0xCE, 0xBD, 0xCD, 0x71, 0x42, 0x35, 0xAB, 0xAF, 0x76, 0x61, 0x28, 0xA7, 0x2A, 0x8B,
        0xD9, 0xAB, 0x2E, 0x2D, 0xC5, 0xDE, 0xAB, 0xAE, 0x10, 0x1D, 0x2A, 0x35, 0xDC, 0x4E, 0x3B, 0xF5, 0xB3, 0x28,
        0xE5, 0xFE, 0x6D, 0xCB, 0x83, 0x69, 0xE4, 0x50, 0x97, 0x1E, 0x97, 0xBD, 0x81, 0x9C, 0xD4, 0x9F, 0x64, 0xBD,
        0x88, 0xD2, 0xD1, 0xB4, 0xF2, 0xC6, 0x95, 0x2D, 0x9F, 0x24, 0xC2, 0x51, 0x73, 0xA7, 0xF4, 0x81, 0x8B, 0xB6,
        0xF9, 0xFC, 0x12, 0xD3, 0xBB, 0x2F, 0x24, 0x54, 0x5B, 0xA9, 0x6D, 0xDC, 0xCE, 0x9B, 0x54, 0x01, 0x17, 0xEA,
        0x7F, 0xDC, 0xD2, 0x53, 0x52, 0xDD, 0x26, 0x9F, 0x5E, 0xC4, 0x42, 0x0F, 0xCC, 0x57, 0xB2, 0x36, 0x9A, 0xAE,
        0x1D, 0xED, 0xD4, 0x0C, 0x6F, 0x4B, 0xEC, 0xF9, 0x13, 0x97, 0x5A, 0xE4, 0x72, 0xBE, 0x37, 0x4D, 0x09, 0x46,
        0xF9, 0xB5, 0x60, 0x54, 0x69, 0xAB, 0xAD, 0xD1, 0x55, 0xBA, 0x1E, 0x3C, 0x2D, 0xBB, 0x6F, 0x62, 0x9A, 0xD2,
        0xF6, 0x33, 0x44, 0x38, 0x6C, 0xC4, 0xA3, 0x48, 0xBE, 0x76, 0x42, 0x6F, 0xA1, 0x42, 0x6F, 0xFF, 0x00, 0xC2,
        0x16, 0xEE, 0xCA, 0xB1, 0xAA, 0x00, 0x7B, 0x2B, 0x21, 0xBE, 0x7B, 0x16, 0xF7, 0x4C, 0xCD, 0x2B, 0xFB, 0x91,
        0x1D, 0x93, 0x9C, 0xB4, 0xB4, 0xA7, 0x56, 0x8C, 0xA2, 0xAD, 0x77, 0x7D, 0xC8, 0x6E, 0xE2, 0xF9, 0x34, 0xC4,
        0xAE, 0x1C, 0x3F, 0x9B, 0x28, 0xCE, 0x7E, 0x9D, 0x56, 0xEE, 0xCA, 0xF0, 0xD9, 0x54, 0x72, 0xB7, 0xB2, 0x4D,
        0x55, 0xB5, 0x66, 0x33, 0xE6, 0x41, 0x0B, 0x52, 0x55, 0xCF, 0xB9, 0x55, 0xA3, 0x6B, 0xCF, 0x93, 0x6F, 0xEE,
        0x3B, 0x4E, 0xFD, 0x5F, 0xB0, 0x42, 0x37, 0xE2, 0x1C, 0x5D, 0x6D, 0x7D, 0x0D, 0xDC, 0x12, 0x5F, 0x4A, 0xFC,
        0x13, 0x74, 0x28, 0x43, 0x1C, 0xA2, 0xDF, 0x9B, 0xBF, 0xBA, 0x39, 0xA5, 0x2B, 0x7A, 0x64, 0xF8, 0xEC, 0x74,
        0x46, 0x71, 0xEC, 0x8C, 0xB2, 0xAD, 0x4E, 0xF6, 0x48, 0x95, 0x18, 0x49, 0xDF, 0xD8, 0x51, 0xBE, 0x96, 0x0F,
        0x6E, 0xA7, 0x5E, 0x08, 0xA8, 0xC6, 0xDA, 0xAE, 0xF6, 0x5D, 0xD8, 0xAC, 0x31, 0xC2, 0x72, 0x83, 0xD2, 0xB6,
        0x74, 0x6A, 0xE2, 0xED, 0x45, 0x45, 0xA7, 0xC5, 0x9B, 0xAD, 0x31, 0x5B, 0x0E, 0x2E, 0x3D, 0xA8, 0xCD, 0x23,
        0x96, 0x58, 0x25, 0x16, 0xDA, 0xDE, 0xC7, 0xE5, 0xB8, 0xAB, 0x96, 0xD7, 0xC2, 0x36, 0x9C, 0xA9, 0xF3, 0xF6,
        0x33, 0x9B, 0xD4, 0xAF, 0xB1, 0x68, 0x86, 0xA4, 0xED, 0xD2, 0x4B, 0xFB, 0x82, 0x7A, 0xE3, 0x5B, 0xFF, 0x00,
        0xB0, 0x27, 0xD3, 0xB0, 0xDB, 0xBF, 0x96, 0x11, 0x09, 0x36, 0xB6, 0xDC, 0xA8, 0xE3, 0x72, 0x8E, 0xEE, 0xBD,
        0x87, 0x75, 0xB8, 0x37, 0xD4, 0xAA, 0x1A, 0xA6, 0xAF, 0x74, 0x91, 0x51, 0xC8, 0x9F, 0xE9, 0x5E, 0xE4, 0x4D,
        0xF1, 0xFD, 0x87, 0x8D, 0x74, 0x5F, 0x82, 0x06, 0xD4, 0x7F, 0xCB, 0xFB, 0x8E, 0xE9, 0x27, 0x4E, 0xFB, 0x09,
        0xFA, 0x57, 0xB8, 0xB5, 0x6D, 0xC8, 0x03, 0x4E, 0x79, 0x2D, 0xEC, 0xBF, 0x05, 0x79, 0x70, 0x6B, 0xA7, 0x23,
        0xF2, 0xE3, 0xD5, 0xB6, 0xF9, 0xD8, 0xB4, 0x95, 0x5A, 0xE7, 0xDC, 0x9B, 0xA2, 0x5B, 0x8A, 0xD9, 0x36, 0x0E,
        0x49, 0xA1, 0x49, 0x6F, 0xBA, 0xAF, 0x61, 0xA5, 0x63, 0x30, 0x28, 0xC2, 0xDF, 0xB7, 0xC0, 0xA7, 0x8D, 0x2E,
        0x1B, 0xBE, 0xC5, 0x5D, 0x2E, 0x37, 0x05, 0x2E, 0x47, 0xA3, 0x16, 0x87, 0xF4, 0xAD, 0xC7, 0x39, 0x29, 0x48,
        0x4E, 0xBB, 0x9A, 0x0B, 0x56, 0xF4, 0x0E, 0xD2, 0x6A, 0x48, 0x5B, 0x3D, 0x91, 0xA5, 0x58, 0x44, 0xCA, 0x57,
        0x8F, 0x4D, 0x72, 0xCB, 0xC3, 0x2A, 0xC1, 0x24, 0xED, 0xD7, 0x43, 0x27, 0x6D, 0x24, 0xF9, 0x2A, 0x2B, 0x67,
        0xD2, 0xBF, 0x70, 0x33, 0x94, 0x9B, 0x4D, 0x70, 0x91, 0xAE, 0x28, 0x69, 0x9A, 0x4D, 0xA7, 0x6B, 0xA1, 0x2E,
        0x0F, 0x62, 0xE3, 0xF5, 0x2E, 0xE9, 0x74, 0x1A, 0xA2, 0x0E, 0xBC, 0x5B, 0x7F, 0x3B, 0x9B, 0x4A, 0x69, 0xBA,
        0x5C, 0x7B, 0x9C, 0xF1, 0x4B, 0xCE, 0xBF, 0x9D, 0xCB, 0x9A, 0xDF, 0x67, 0x64, 0xD1, 0x1B, 0xEB, 0xA5, 0xB8,
        0x4E, 0x12, 0x71, 0xB5, 0xD3, 0x92, 0x96, 0xD1, 0x77, 0x1D, 0xFB, 0x99, 0xA6, 0xEF, 0x97, 0xB8, 0x46, 0x6A,
        0x37, 0x2A, 0x7B, 0x5F, 0x53, 0xAA, 0x37, 0xA5, 0x5B, 0xE0, 0xCA, 0x29, 0x69, 0xAB, 0x2D, 0xB6, 0xA2, 0x5D,
        0x53, 0x72, 0xA4, 0x85, 0xAA, 0xFF, 0x00, 0xE0, 0xCD, 0xC9, 0x3D, 0x98, 0xD3, 0xA2, 0x0A, 0x9C, 0xBF, 0x62,
        0x77, 0x7F, 0x71, 0xCB, 0xD5, 0xCF, 0x41, 0x36, 0x97, 0xB2, 0x00, 0xE1, 0xD8, 0xB5, 0x6F, 0xC8, 0x29, 0x5A,
        0x26, 0x4A, 0xBE, 0xE5, 0x45, 0x37, 0x6F, 0xD9, 0x83, 0x62, 0x82, 0xBE, 0x45, 0x27, 0xEA, 0xE4, 0x0A, 0xBF,
        0xC0, 0xD3, 0xED, 0xC8, 0xAB, 0xD3, 0x43, 0x52, 0x4B, 0x64, 0x15, 0x52, 0xDF, 0x81, 0x69, 0xB9, 0x0D, 0x3B,
        0xBE, 0xC3, 0x8E, 0xFF, 0x00, 0x28, 0x0A, 0xDD, 0x73, 0x43, 0xD9, 0xA5, 0xEC, 0x67, 0x39, 0xA7, 0xD0, 0x70,
        0xE7, 0xB2, 0x6B, 0x92, 0x01, 0xBD, 0x4E, 0xCA, 0xD5, 0xB1, 0x3C, 0x84, 0xB7, 0x87, 0x1C, 0x14, 0x09, 0xDB,
        0xF9, 0x2B, 0x85, 0xC1, 0x9A, 0x95, 0x22, 0xD4, 0xB8, 0xAF, 0xC0, 0x10, 0xA3, 0x4B, 0x7F, 0xDC, 0x86, 0xEC,
        0xDA, 0x52, 0xF6, 0xDC, 0xCD, 0x4B, 0xEC, 0x04, 0x25, 0xEA, 0xBB, 0xD8, 0xA9, 0x4A, 0xB8, 0x1F, 0xB9, 0x2D,
        0x04, 0x6B, 0x77, 0x2D, 0xD1, 0x4A, 0x2F, 0x26, 0x55, 0x14, 0xD2, 0x64, 0xD8, 0xE2, 0xEE, 0x6B, 0x6E, 0x18,
        0x55, 0x38, 0xD3, 0x6B, 0xAA, 0x15, 0x6F, 0xF2, 0x35, 0x1D, 0x3B, 0x22, 0x6E, 0xB6, 0xAE, 0x3A, 0x94, 0x0A,
        0xB5, 0xAF, 0x72, 0xA3, 0x15, 0xAA, 0xFF, 0x00, 0x63, 0x3A, 0x6A, 0x6A, 0xFA, 0x1A, 0x2B, 0x69, 0xB2, 0x07,
        0x91, 0x27, 0x06, 0x91, 0x84, 0x62, 0xF9, 0xEC, 0x6F, 0x57, 0x1A, 0x0D, 0x34, 0xA8, 0x0C, 0x69, 0x27, 0xD7,
        0x71, 0x5A, 0xE3, 0x51, 0x6E, 0x3A, 0x9B, 0xF6, 0x46, 0x4E, 0x2D, 0x26, 0xFF, 0x00, 0x2C, 0x07, 0x57, 0x26,
        0xA8, 0x9F, 0xD4, 0x8A, 0x8B, 0x5D, 0xB7, 0x43, 0x9D, 0xB7, 0x40, 0x1A, 0xAC, 0x99, 0x53, 0x8F, 0x25, 0x28,
        0xD2, 0x7B, 0x19, 0x36, 0x05, 0x26, 0xBB, 0x09, 0xC8, 0x8B, 0x2D, 0x53, 0x54, 0xFA, 0x15, 0x06, 0xA2, 0xEE,
        0xD3, 0x33, 0x5B, 0x36, 0x84, 0xA5, 0xD0, 0x41, 0x6D, 0xE9, 0xEA, 0x38, 0xEE, 0xF8, 0xD9, 0x12, 0xDD, 0xFC,
        0x95, 0xAA, 0xD1, 0x06, 0x8B, 0x8F, 0xF6, 0x1A, 0xE7, 0x72, 0x62, 0xF6, 0xDF, 0x90, 0xBA, 0xE3, 0x90, 0xAA,
        0x71, 0x8D, 0x6C, 0x3D, 0xA2, 0xB9, 0xB6, 0x4C, 0x1A, 0x92, 0xA7, 0xB3, 0x1F, 0x32, 0xAE, 0x8F, 0xB0, 0x04,
        0x56, 0xDF, 0x72, 0x92, 0x5D, 0xAB, 0xDC, 0x6E, 0xAB, 0xB5, 0x03, 0x74, 0x80, 0x95, 0x1A, 0x6E, 0xEB, 0xEE,
        0x29, 0xB5, 0x14, 0xAD, 0x2A, 0xE8, 0x5A, 0x77, 0x7C, 0x0B, 0x69, 0x24, 0x9F, 0xE1, 0x90, 0x65, 0x28, 0xDF,
        0xA9, 0x70, 0x4E, 0xAA, 0xB4, 0x6B, 0x2C, 0x6D, 0xF1, 0x7B, 0xF4, 0x1A, 0xC3, 0x1A, 0x57, 0xCF, 0xB8, 0x18,
        0x29, 0x57, 0x28, 0xAD, 0x69, 0xF4, 0xDC, 0x79, 0x20, 0xE1, 0xB7, 0x2B, 0xB9, 0x9A, 0x5C, 0xBE, 0xC5, 0x47,
        0xFF, 0xD9};

    num_transfers =
        (uint16_t)((image_length + (IRIS_IMAGE_TRANSFER_BLOCK_SIZE - 1)) / IRIS_IMAGE_TRANSFER_BLOCK_SIZE);

    spi_init_burst(VIS_SENSOR);
    for (int j = 0; j < num_transfers; j++) {
        for (int i = 0; i < IRIS_IMAGE_TRANSFER_BLOCK_SIZE; i++) {
            image_data[i] = (uint8_t)spi_read_burst(VIS_SENSOR); // reg_addr: 0x3D, sensor: 0 -> VIS_SENSOR
            // image_data[i] = (uint8_t) image_data_buffer[count];
        }

        spi_transmit(image_data, IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
    }
    spi_deinit_burst(VIS_SENSOR);

    DBG_PUT("DONE IMAGE TRANSFER!\r\n");
}
