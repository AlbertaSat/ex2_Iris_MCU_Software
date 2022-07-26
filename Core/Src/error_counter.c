/*
 * error_counter.c
 *
 *  Created on: Jul. 26, 2022
 *      Author: Liam
 */

#include "error_counter.h"

uint8_t num_of_errors = 0;

void iterate_error_num(void) {
    num_of_errors++;
    return;
}

uint8_t get_error_num(void) { return num_of_errors; }

void reset_error_num(void) {
    num_of_errors = 0;
    return;
}
