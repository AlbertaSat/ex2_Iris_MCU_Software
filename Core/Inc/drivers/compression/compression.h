/*
 * compression.h
 *
 *  Created on: Jul 18, 2022
 *      Author: liam
 */

#ifndef INC_DRIVERS_COMPRESSION_COMPRESSION_H_
#define INC_DRIVERS_COMPRESSION_COMPRESSION_H_

#include "iris_system.h"

#define COMPRESSION_ECC_TRUE 1
#define COMPRESSION_EXX_FALSE 0

unsigned int rle_compress(unsigned char *arr, unsigned int insize, unsigned char *outarr, uint8_t ecc);
void test_compression();

#endif /* INC_DRIVERS_COMPRESSION_COMPRESSION_H_ */
