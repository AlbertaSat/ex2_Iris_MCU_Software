/*
 * IEB_TESTS.c
 *
 *  Created on: Mar. 21, 2022
 *      Author: Liam
 */
#include "iris_system.h"
#include "IEB_TESTS.h"
#include "tmp421.h"
#include "command_handler.h"

extern I2C_HandleTypeDef hi2c2;

static void testTempSensor(void);

/**
 * @brief Tests basic lifesigns of Iris board
 *
 */
void CHECK_LED_I2C_SPI_TS(void) {

    // Blink IO LED
    iris_log("--------------------\r\n");
    iris_log("Blinking LED\r\n");

    for (int i = 0; i < 10; i++) {
        _toggleLED();
        HAL_Delay(150);
    }
    iris_log("--------------------\r\n\n");

    // Scan I2C Bus
    iris_log("--------------------\r\n");
    iris_log("Testing internal I2C bus\r\n");
    _testScanI2C();
    iris_log("--------------------\r\n\n");

    // Test SPI
    iris_log("--------------------\r\n");
    iris_log("Ensuring sensors are powered\r\n");
    sensor_active();
    iris_log("Testing VIS SPI\r\n");

    _testArducamSensor(VIS_SENSOR);
    iris_log("Testing NIR SPI\r\n");

    _testArducamSensor(NIR_SENSOR);
    iris_log("--------------------\r\n\n");

    // Temperature Sensor stuff
    iris_log("--------------------\r\n");
    iris_log("Testing Temperature Sensors\r\n");
    testTempSensor();
    iris_log("--------------------\r\n");

    // NAND Flash
    iris_log("--------------------\r\n");
    iris_log("Testing NAND Flash\r\n");

    // TODO: Test nand

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
            iris_log("TEST FAILED: VIS Camera: SPI Error\r\n");
        } else {
            iris_log("TEST:FAILED: NIR Camera: SPI Error\r\n");
        }
    } else {
        if (sensor == VIS_SENSOR) {
            iris_log("TEST PASSED: VIS SPI Initialized\r\n");
        } else {
            iris_log("TEST PASSED: NIR SPI Initialized\r\n");
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
            iris_log(buf);
        }
    }
    iris_log("Scan Complete.\r\n");
    if (deviceFound == 1) {
        iris_log("TEST PASSED: I2C Operational\r\n");
    } else {
        iris_log("TEST FAILED: I2C Unoperational\r\n");
    }
}

/**
 * @brief Tests temp sensors; parses for non-zero readout from the temperature regs
 *
 */
static void testTempSensor(void) {
    iris_log("\n");
    uint16_t vis_temp, nir_temp, nand_temp, gate_temp;
    vis_temp = nir_temp = nand_temp = gate_temp = 0;
    vis_temp = get_temp(0x4C);
    nir_temp = get_temp(0x4D);
    nand_temp = get_temp(0x4E);
    gate_temp = get_temp(0x4F);

    if (vis_temp && nir_temp && nand_temp && gate_temp) {
        iris_log("TEST PASSED: Temp sensors operational\r\n");
        printTemp(vis_temp, 0x4C);
        printTemp(nir_temp, 0x4D);
        printTemp(nand_temp, 0x4E);
        printTemp(gate_temp, 0x4F);
    } else {
        iris_log("TEST FAILED: Temp sensors unoperational\r\n");
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
    iris_log(buf);
}

void test_clocksignal() {
    for (;;) {
        //	HAL_GPIO_WritePin(CLK_Port, CLK_Pin, GPIO_PIN_SET);
        CLK_Port->BSRR = CLK_Pin;   // high
        MOSI_Port->BSRR = MOSI_Pin; // high
        CLK_Port->BRR = CLK_Pin;    // low
        MOSI_Port->BRR = MOSI_Pin;  // high
    }
}
