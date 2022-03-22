/*
 * IEB_TESTS.c
 *
 *  Created on: Mar. 21, 2022
 *      Author: Liam
 */
#include "IEB_TESTs.h"
#include "cli.h"
#include "main.h"
extern I2C_HandleTypeDef hi2c2;

void CHECK_LED_I2C_SPI_TS(void){

	// Blink IO LED
	for (int i=0; i<10; i++){
		_toggleLED();
		HAL_Delay(150);
	}

	// Scan I2C Bus
	scan_i2c();

	// Test SPI
	_testArducamSensor(VIS_SENSOR);
	_testArducamSensor(NIR_SENSOR);
	_testOBCSPI();

	// Temperature Sensor stuff
	HAL_Delay(1000);
}

void _toggleLED(void){
	HAL_GPIO_TogglePin(TEST_OUT1_GPIO_Port, TEST_OUT1_Pin);
}

void _testArducamSensor(uint8_t sensor){
	arducam_wait_for_ready(sensor);
	write_reg(AC_REG_RESET, 1, sensor);
	write_reg(AC_REG_RESET, 1, sensor);
	HAL_Delay(100);
	write_reg(AC_REG_RESET, 0, sensor);
	HAL_Delay(100);
	if (!arducam_wait_for_ready(sensor)) {
		if (sensor == VIS_SENSOR){
			  DBG_PUT("TEST FAILED: VIS Camera: SPI Error\r\n");
		}
		else{
			  DBG_PUT("TEST:FAILED: NIR Camera: SPI Error\r\n");
		}
	}
	else{
		if (sensor == VIS_SENSOR){
			  DBG_PUT("VIS Camera: SPI Initialized\r\n");
		}
		else{
			  DBG_PUT("NIR Camera: SPI Initialized\r\n");
		}
	}
}

void _testOBCSPI(void){
	return;
}

void _testScanI2C(){
	 HAL_StatusTypeDef result;
	 uint8_t i;
	 char buf[64];
	 int deviceFound = 0;
	 DBG_PUT("Scanning I2C bus 2...\r\n");
	 for (i=1; i<128; i++){
		 result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i<<1), 2, 2);
		 if (result == HAL_OK){
			 if (deviceFound != 0){
				 deviceFound = 1; 	// Janky but works for asserting that I2C bus is operational
			 }
			 sprintf(buf,"I2C address found: 0x%X\r\n", (uint16_t)(i));
			 DBG_PUT(buf);
		 	 }
	  	}
	 DBG_PUT("Scan Complete.\r\n");
	 if (deviceFound == 1){
		 DBG_PUT("I2C TEST PASSED\r\n");
	 }
	 else{
		 DBG_PUT("I2C TEST FAILED\r\n");
	 }
}

