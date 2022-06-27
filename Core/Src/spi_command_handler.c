#include <spi_command_handler.h>
#include <command_handler.h>
#include <iris_system.h>
#include <string.h>

extern SPI_HandleTypeDef hspi1;
extern uint8_t cam_to_nand_transfer_flag;

uint32_t image_length = 0x5F3; // Only here for testing purposes
static uint32_t count = 0x0FFF0000;

uint8_t ack = 0xAA;
uint8_t nack = 0x0F;
uint8_t transmit_ack;

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
 * 		Verifies if command from OBC is valid or not
 * @param
 * 		obc_cmd: Command from OBC
 * @return
 * 		1 if valid command, 0 if not
 */
int spi_verify_command(uint8_t obc_cmd) {
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
        spi_transmit(&ack, 1);
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
    uint8_t rx_data;
    uint8_t tx_data = 0x69;
    spi_receive(&rx_data, 1);

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
        //        	take_image(cmd);
        //        iterate_image_num();
        cam_to_nand_transfer_flag = 1;
        spi_transmit(&tx_data, 1);
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
        return 0;
    }
    case IRIS_GET_IMAGE_LENGTH: {
        get_image_length(&image_length);
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
    uint8_t image_data[IRIS_IMAGE_TRANSFER_BLOCK_SIZE];
    uint16_t num_transfers;

    for (int i = 0; i < IRIS_IMAGE_TRANSFER_BLOCK_SIZE; i++) {
        image_data[i] = i;
    }

    num_transfers =
        (uint16_t)((image_length + (IRIS_IMAGE_TRANSFER_BLOCK_SIZE - 1)) / IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
    for (int j = 0; j < num_transfers; j++) {
        spi_transmit(image_data, IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
        memset(image_data, 0, IRIS_IMAGE_TRANSFER_BLOCK_SIZE);
    }
}
