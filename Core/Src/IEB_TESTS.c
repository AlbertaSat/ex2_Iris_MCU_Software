/*
 * IEB_TESTS.c
 *
 *  Created on: Mar. 21, 2022
 *      Author: Liam
 */
#include "IEB_TESTS.h"
#include "main.h"
#include "tmp421.h"
#include "nand_m79a.h"
#include "command_handler.h"

extern I2C_HandleTypeDef hi2c2;

static void testTempSensor(void);

/**
 * @brief Tests basic lifesigns of Iris board
 *
 */
void CHECK_LED_I2C_SPI_TS(void) {

    // Blink IO LED
    DBG_PUT("--------------------\r\n");
    DBG_PUT("Blinking LED\r\n");

    for (int i = 0; i < 10; i++) {
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
    DBG_PUT("Ensuring sensors are powered\r\n");
    sensor_active();
    DBG_PUT("Testing VIS SPI\r\n");

    _testArducamSensor(VIS_SENSOR);
    DBG_PUT("Testing NIR SPI\r\n");

    _testArducamSensor(NIR_SENSOR);
    DBG_PUT("--------------------\r\n\n");

    // Temperature Sensor stuff
    DBG_PUT("--------------------\r\n");
    DBG_PUT("Testing Temperature Sensors\r\n");
    testTempSensor();
    DBG_PUT("--------------------\r\n");

    // NAND Flash
    DBG_PUT("--------------------\r\n");
    DBG_PUT("Testing NAND Flash\r\n");

    NAND_ReturnType res = NAND_Init();
    if (res == Ret_Success) {
        DBG_PUT("TEST PASSED: NAND Flash Initialized\r\n");
    } else {
        DBG_PUT("TEST FAILED: NAND Flash Error\r\n");
    }
    DBG_PUT("--------------------\r\n\n");

    HAL_Delay(1000);
}

/**
 * @brief toggles the test LED
 *
 */
void _toggleLED(void) { HAL_GPIO_TogglePin(TEST_OUT1_GPIO_Port, TEST_OUT1_Pin); }

/**
 * @brief Tests arducam sensor for use with lifesign testing
 *
 * @param sensor target sensor
 */
void _testArducamSensor(uint8_t sensor) {
    arducam_wait_for_ready(sensor);
    write_reg(AC_REG_RESET, 1, sensor);
    write_reg(AC_REG_RESET, 1, sensor);
    HAL_Delay(100);
    write_reg(AC_REG_RESET, 0, sensor);
    HAL_Delay(100);
    if (!arducam_wait_for_ready(sensor)) {
        if (sensor == VIS_SENSOR) {
            DBG_PUT("TEST FAILED: VIS Camera: SPI Error\r\n");
        } else {
            DBG_PUT("TEST:FAILED: NIR Camera: SPI Error\r\n");
        }
    } else {
        if (sensor == VIS_SENSOR) {
            DBG_PUT("TEST PASSED: VIS SPI Initialized\r\n");
        } else {
            DBG_PUT("TEST PASSED: NIR SPI Initialized\r\n");
        }
    }
}

/**
 * @brief Scans the I2C bus for devices. Prints out if at least one is found
 *
 */
void _testScanI2C() {
    HAL_StatusTypeDef result;
    uint8_t i;
    char buf[64];
    int deviceFound = 0;
    for (i = 1; i < 128; i++) {
        result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i << 1), 2, 2);
        if (result == HAL_OK) {
            if (deviceFound == 0) {
                deviceFound = 1; // Janky but works for asserting that I2C bus is operational
            }
            sprintf(buf, "I2C address found: 0x%X\r\n", (uint16_t)(i));
            DBG_PUT(buf);
        }
    }
    DBG_PUT("Scan Complete.\r\n");
    if (deviceFound == 1) {
        DBG_PUT("TEST PASSED: I2C Operational\r\n");
    } else {
        DBG_PUT("TEST FAILED: I2C Unoperational\r\n");
    }
}

/**
 * @brief Tests temp sensors; parses for non-zero readout from the temperature regs
 *
 */
static void testTempSensor(void) {
    DBG_PUT("\n");
    uint16_t vis_temp, nir_temp, nand_temp, gate_temp;
    vis_temp = nir_temp = nand_temp = gate_temp = 0;
    vis_temp = get_temp(0x4C);
    nir_temp = get_temp(0x4D);
    nand_temp = get_temp(0x4E);
    gate_temp = get_temp(0x4F);

    if (vis_temp && nir_temp && nand_temp && gate_temp) {
        DBG_PUT("TEST PASSED: Temp sensors operational\r\n");
        printTemp(vis_temp, 0x4C);
        printTemp(nir_temp, 0x4D);
        printTemp(nand_temp, 0x4E);
        printTemp(gate_temp, 0x4F);
    } else {
        DBG_PUT("TEST FAILED: Temp sensors unoperational\r\n");
    }
    return;
}

/**
 * @brief Prints temperature from sensor in degrees Celsius format without
 *        using floats.
 *
 * @param temp 2 byte temperature value from sensor
 * @param sensor I2C (7 bit) address of temperature sensor
 */
void printTemp(uint16_t temp, uint8_t sensor) {
    char buf[64];
    sprintf(buf, "Sensor 0x%x Temperature: %d.%04d C\r\n", sensor, (temp >> 8) - 64, ((temp & 0xFF) >> 4) * 625);
    DBG_PUT(buf);
}
