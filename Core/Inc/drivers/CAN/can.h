/*
 * can.h
 *
 *  Created on: Jul 11, 2022
 *      Author: liam
 */

#ifndef INC_DRIVERS_CAN_CAN_H_
#define INC_DRIVERS_CAN_CAN_H_

#include "main.h"

#define DOMINANT 0
#define RECESSIVE 1

#define CAN_HEADER_LEN 3
#define CAN_FOOTER_LEN 4

// don't forget stuffin
// typedef struct __attribute__((__packed__)) CAN_packet_s {
//	uint16_t START : 1;
//	uint16_t IDENTIFIER: 11;	// looks like it takes 11 LSB ie, 0x1234 -> 0x234
//	uint16_t RTR : 1;		// 0 read; 1 write
//	uint16_t IDE : 1;
//	uint16_t RESERVE : 1;
//	uint16_t SIZE : 4;
//	uint16_t DATA[8];
//	uint16_t CHECK : 16;
//	uint16_t ACK : 2;
//	uint16_t END: 7;
//} CAN_packet_t;

void send_can_frame(uint32_t * frame, unsigned int frame_len_bits);

#endif /* INC_DRIVERS_CAN_CAN_H_ */
