/*
 * IEB_TESTS.h
 *
 *  Created on: Mar. 21, 2022
 *      Author: Liam
 */

#ifndef INC_IEB_TESTS_H_
#define INC_IEB_TESTS_H_

#include "spi_bitbang.h"
#include "debug.h"
#include "I2C.h"
void CHECK_LED_I2C_SPI_TS_NAND(void);

void _toggleLED(void);
void _testArducamSensor(uint8_t sensor);
void _testScanI2C();
void testTempSensor(void);
void printTemp(uint16_t temp, uint8_t sensor);
void test_nand_flash(void);
#endif /* INC_IEB_TESTS_H_ */
