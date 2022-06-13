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
    uint8_t errornum;
    uint16_t MAX_5V_voltage;
    uint16_t MAX_5V_power;
    uint16_t MAX_3V_voltage;
    uint16_t MAX_3V_power;
    uint16_t MIN_5V_voltage;
    uint16_t MIN_3V_voltage;

} housekeeping_packet_t;

housekeeping_packet_t _get_housekeeping();
void decode_hk_packet(housekeeping_packet_t hk);

#endif /* INC_HOUSEKEEPING_H_ */
