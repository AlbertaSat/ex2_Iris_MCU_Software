/*
 * error_counter.h
 *
 *  Created on: Jul. 26, 2022
 *      Author: Liam
 */

#ifndef INC_ERROR_COUNTER_H_
#define INC_ERROR_COUNTER_H_

#include "iris_system.h"

void iterate_error_num(void);
uint8_t get_error_num(void);
void reset_error_num(void);

#endif /* INC_ERROR_COUNTER_H_ */
