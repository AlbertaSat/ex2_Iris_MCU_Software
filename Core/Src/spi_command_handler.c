#include <spi_command_handler.h>
#include <command_handler.h>
#include <iris_system.h>
#include <stdio.h>
#include <string.h>

extern SPI_HandleTypeDef hspi1;
extern enum iris_states iris_state;

static uint8_t cmd;
static uint8_t curr_state;
uint8_t num_transfers_buffer[2];

// static enum local_state {
//	VERIFY_COMMAND,
//	HANDLE_COMMAND,
//};

uint8_t ack = 0xAA;
uint8_t nack = 0x0F;
uint8_t transmit_ack;

void spi_transmit(uint8_t *tx_data, uint16_t data_length) {
    HAL_SPI_Transmit(&hspi1, tx_data, data_length, HAL_MAX_DELAY);
}

void spi_receive(uint8_t *rx_data, uint16_t data_length) { HAL_SPI_Receive_IT(&hspi1, rx_data, data_length); }

int spi_listen() {
    iris_state = IDLE;
    curr_state = LISTENING;
    spi_receive(&cmd, 1);
    return 0;
}

/**
 * @brief SPI command verifier
 */
int spi_verify_command() {
    transmit_ack = 0;

    switch (cmd) {
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
    }

    if (transmit_ack) {
        spi_transmit(&ack, 1);
        return 0;
    } else {
        spi_transmit(&nack, 1);
        return -1;
    }
}

/**
 * @brief SPI command handler
 */
int spi_handle_command() {
    uint8_t rx_data;
    uint8_t tx_data = 0x69;
    spi_receive(&rx_data, 1);

    curr_state = HANDLE_COMMAND;

    switch (cmd) {
    case IRIS_SEND_HOUSEKEEPING: {
        housekeeping_packet_t hk;
        get_housekeeping(&hk);

        uint8_t buffer[sizeof(hk)];
        memcpy(buffer, &hk, sizeof(hk));
        spi_transmit(buffer, sizeof(buffer));

        return 0;
    }
    case IRIS_TAKE_PIC:
        // needs dedicated thought put towards implement
        //        take_image(cmd);
        //        iterate_image_num();
        spi_transmit(&tx_data, 1);
        return 0;
    case IRIS_GET_IMAGE_COUNT:
        get_image_num_spi(&tx_data);
        spi_transmit(&tx_data, 1);
        return 0;

    case IRIS_TRANSFER_IMAGE: {
        iris_state = IDLE;
        spi_receive(num_transfers_buffer, 2);
        return 0;
    }
    case IRIS_OFF_SENSOR_IDLE:
        sensor_idle();
        return 0;
    case IRIS_ON_SENSOR_IDLE:
        sensor_active();
        return 0;
    case IRIS_GET_IMAGE_LENGTH: {
        uint32_t image_length;
        get_image_length(&image_length);
        uint8_t packet[3];
        packet[0] = (image_length >> (8 * 2)) & 0xff;
        packet[1] = (image_length >> (8 * 1)) & 0xff;
        packet[2] = (image_length >> (8 * 0)) & 0xff;

        spi_transmit(packet, 3);
        return 0;
    }
    case IRIS_UPDATE_SENSOR_I2C_REG:
        // t h o n k
        //    	update_sensor_I2C_regs();
        spi_transmit(&tx_data, 1);
        return 0;
    case IRIS_UPDATE_CURRENT_LIMIT:
        //    	update_current_limits();
        spi_transmit(&tx_data, 1);
        return 0;
    default:
        return -1;
    }
}

void spi_transfer_image() {
    uint8_t image_data[512];
    uint16_t num_transfers;

    for (int i = 0; i < 512; i++) {
        image_data[i] = i;
    }

    num_transfers = (uint16_t)((uint8_t)num_transfers_buffer[0] << 8 | (uint8_t)num_transfers_buffer[1]);
    for (int j = 0; j < num_transfers; j++) {
        spi_transmit(image_data, 512);
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (curr_state == LISTENING) {
        spi_verify_command();
        spi_handle_command();

        if (cmd != IRIS_TRANSFER_IMAGE) {
            iris_state = FINISH;
        }
    } else if (curr_state == HANDLE_COMMAND) {
        switch (cmd) {
        case IRIS_TRANSFER_IMAGE:
            spi_transfer_image();
            iris_state = FINISH;
        default: {
            iris_state = FINISH;
        }
        }
    }
}
