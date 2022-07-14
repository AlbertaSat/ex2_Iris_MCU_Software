/*
 * can.c
 *	// NOT COMPLETE DO NOT USE
 *
 *  Created on: Jul 11, 2022
 *      Author: liam
 */
#include "can.h"
#include "iris_system.h"
extern hcrc;
void test_can_write(uint8_t address) {
    // NOT COMPLETE DO NOT USE

    for (int i = 0; i < 8; i++) {
        if ((address >> i) & 1) {
            CAN_TX_GPIO_Port->BSRR = CAN_TX_Pin;
        } else {
            CAN_TX_GPIO_Port->BRR = CAN_TX_Pin;
        }
    }
}

// BIT STUFFING EH
// not applicable on CRC, ACK, or EOF fields.

void generate_can_header(uint8_t *header, uint16_t id, uint8_t rw, uint8_t size) {
    // NOT COMPLETE DO NOT USE

    uint8_t res = 0x00;
    uint8_t ide = 0x00;
    header[0] = 0x00 | (id >> 4);
    header[1] = (((id & 0xF) << 4) | (rw << 3) | (ide << 2) | (res << 1) | (size >> 3));
    header[2] = (size & 0x7) << 5;
}

void generate_data_frame(uint8_t *byte_array, uint8_t *byte_stuffed_array) {
    // Placeholder. Need to do byte stuffin
    //  NOT COMPLETE DO NOT USE

    byte_stuffed_array = byte_array;
    return;
}

void generate_can_footer(uint8_t *footer, uint8_t *byte_stuffed_array) {
    // make CRC for byte_stuffed_array
    // NOT COMPLETE DO NOT USE

    uint16_t crc = HAL_CRC_Calculate(&hcrc, byte_stuffed_array, sizeof(byte_stuffed_array));
    footer[0] = (crc & 0xFF00) >> 8;
    footer[1] = (crc & 0x00FF);
    footer[2] = 0xC0;
    footer[3] = 0x00;
}

#define CAN_DELAY 1
void send_can_header(uint8_t *header) {
    // NOT COMPLETE DO NOT USE

    int k = 0;
    for (int j = 0; j < sizeof(header); j++) {
        for (int i = 0; i < 8; i++) {
            ERR_GPIO_Port->BRR = ERR_Pin;
            //        	delay_us(CAN_DELAY);
            if ((header[j] >> i) & 1) {
                CAN_TX_GPIO_Port->BRR = CAN_TX_Pin;
            } else {
                CAN_TX_GPIO_Port->BSRR = CAN_TX_Pin;
            }
            ERR_GPIO_Port->BSRR = ERR_Pin;
            //        	delay_us(CAN_DELAY);
            if (k >= 19) {
                // don't send last dummy bytes
                ERR_GPIO_Port->BRR = ERR_Pin;
                CAN_TX_GPIO_Port->BSRR = CAN_TX_Pin;
                return;
            }

            k++;
        }
    }
}
