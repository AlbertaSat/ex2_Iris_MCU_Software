/*
 * tmp421.h
 *
 *  Created on: Mar. 24, 2022
 *      Author: Liam Droog
 */

#ifndef INC_TMP421_H_
#define INC_TMP421_H_
#include "I2C.h"
#define VIS_TEMP_SENSOR 0x4C
#define NIR_TEMP_SENSOR 0x4D
#define TEMP3 0x4E
#define TEMP4 0x4F


uint16_t get_temp(uint8_t sensor_addr);
void init_temp_sensors(void);

#endif /* INC_TMP421_H_ */
