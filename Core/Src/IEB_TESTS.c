/*
 * IEB_TESTS.c
 *
 *  Created on: Mar. 21, 2022
 *      Author: Liam
 */
#include "IEB_TESTS.h"
#include "cli.h"
#include "main.h"
#include "tmp421.h"
extern I2C_HandleTypeDef hi2c2;

void CHECK_LED_I2C_SPI_TS(void){

	// Blink IO LED
	DBG_PUT("--------------------\r\n");
	DBG_PUT("Blinking LED\r\n");

	for (int i=0; i<10; i++){
		_toggleLED();
		HAL_Delay(150);
	}
	DBG_PUT("--------------------\r\n\n");

	// Scan I2C Bus
	DBG_PUT("--------------------\r\n");
	DBG_PUT("Testing internal I2C bus\r\n");
	_testScanI2C();
	DBG_PUT("--------------------\r\n\n");

	// Test SPI
	DBG_PUT("--------------------\r\n");
	DBG_PUT("Testing VIS SPI\r\n");

	_testArducamSensor(VIS_SENSOR);
	DBG_PUT("Testing NIR SPI\r\n");

	_testArducamSensor(NIR_SENSOR);
	DBG_PUT("--------------------\r\n\n");

	// Temperature Sensor stuff
	DBG_PUT("--------------------\r\n");
	DBG_PUT("Testing Temperature Sensors (NC)\r\n");
	DBG_PUT("--------------------\r\n");

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
			  DBG_PUT("TEST PASSED: VIS SPI Initialized\r\n");
		}
		else{
			  DBG_PUT("TEST PASSED: NIR SPI Initialized\r\n");
		}
	}
}



void _testScanI2C(){
	 HAL_StatusTypeDef result;
	 uint8_t i;
	 char buf[64];
	 int deviceFound = 0;
	 for (i=1; i<128; i++){
		 result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i<<1), 2, 2);
		 if (result == HAL_OK){
			 if (deviceFound == 0){
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


void testTempSensor(void){
	DBG_PUT("\n");
	uint16_t vis_temp = get_temp(0x4C);
	uint16_t nir_temp = get_temp(0x4D);
	uint16_t nand_temp = get_temp(0x4E);
	uint16_t gate_temp = get_temp(0x4F);
	printTemp(vis_temp, 0x4C);
	printTemp(nir_temp, 0x4D);
	printTemp(nand_temp, 0x4E);
	printTemp(gate_temp, 0x4F);
	DBG_PUT("\n");
	return;

}

void printTemp(uint16_t temp, uint8_t sensor){
	char buf[64];
	float high = (float)(temp >> 8) - 0x40;
	float low = (float)((temp&0xFF) >> 4) * 0.0625;
	sprintf(buf,"Sensor 0x%x Temperature: %2.3f C\n", sensor, high+low);
	DBG_PUT(buf);

}
