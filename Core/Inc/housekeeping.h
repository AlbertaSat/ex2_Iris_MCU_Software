/*
 * housekeeping.h
 *
 *  Created on: Mar. 29, 2022
 *      Author: Liam Droog
 */
#ifndef INC_HOUSEKEEPING_H_
#define INC_HOUSEKEEPING_H_
typedef struct __attribute__((__packed__)) housekeeping_packet_s {
	uint16_t vis_temp;
	uint16_t nir_temp;
	uint16_t flash_temp;
	uint16_t gate_temp;
	uint8_t imagenum;
	uint8_t software_version;
}housekeeping_packet_t;

housekeeping_packet_t get_housekeeping();

#endif /* INC_HOUSEKEEPING_H_ */
