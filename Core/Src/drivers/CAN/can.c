/*
 * can.c
 *
 *  Created on: Jul 11, 2022
 *      Author: liam
 */
#include "can.h"

void test_can_write(uint8_t address) {
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
    uint8_t res = 0x00;
    uint8_t ide = 0x00;
    header[0] = 0x00 | (id >> 4);
    header[1] = (((id & 0xF) << 4) | (rw << 3) | (ide << 2) | (res << 1) | (size >> 3));
    header[2] = (size & 0x7) << 5;
}

void send_can_header(uint8_t *header) {
    int k = 0;
    for (int j = 0; j < sizeof(header); j++) {
        for (int i = 0; i < 8; i++) {
            if ((header >> i) & 1) {
                CAN_TX_GPIO_Port->BSRR = CAN_TX_Pin;
            } else {
                CAN_TX_GPIO_Port->BRR = CAN_TX_Pin;
            }
            if (k >= 19) {
                // don't send last dummy bytes
                return;
            }

            k++;
        }
    }
}
