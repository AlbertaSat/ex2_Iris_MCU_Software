#include <stdio.h>
#include "main.h"
#include "command_handler.h"
#include "debug.h"

uint8_t VIS_DETECTED = 0;
uint8_t NIR_DETECTED = 0;

extern I2C_HandleTypeDef hi2c2;


/**
 * @brief Resets target sensor to default parameters
 *
 * @param sensor
 */
void sensor_reset(uint8_t sensor) {
    char buf[64];

    arducam_wait_for_ready(sensor);

    // Reset the CPLD
    write_reg(AC_REG_RESET, 1, sensor);
    HAL_Delay(100);
    write_reg(AC_REG_RESET, 0, sensor);
    HAL_Delay(100);

    if (!arducam_wait_for_ready(sensor)) {
        DBG_PUT("VIS Camera: SPI Unavailable\r\n");
    } else {
        DBG_PUT("VIS Camera: SPI Initialized\r\n");
    }

    // Change MCU mode
    write_reg(ARDUCHIP_MODE, 0x0, sensor);
    wrSensorReg16_8(0xff, 0x01, sensor);

    uint8_t vid = 0, pid = 0;
    rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid, sensor);
    rdSensorReg16_8(OV5642_CHIPID_LOW, &pid, sensor);

    int detected = VIS_DETECTED;
    if (sensor == NIR_SENSOR) {
        detected = NIR_DETECTED;
    } else if (sensor != VIS_SENSOR) {
        sprintf(buf, "illegal sensor %d\r\n", sensor);
        DBG_PUT(buf);
    }

    if (vid != 0x56 || pid != 0x42) {
        sprintf(buf, "Camera I2C Address: Unknown\r\nVIS not available\r\n\n");
        DBG_PUT(buf);
        detected = 0;
    } else {
        detected = 1;
        format = JPEG;
        Arduino_init(format, sensor);
        sprintf(buf, "Camera Mode: JPEG\r\n");
        DBG_PUT(buf);
    }

    HAL_Delay(1000);
}

/**
 * @brief Scans I2C2 (internal) bus for devices. Debug purpose only
 *
 * @return int
 */
int uart_scan_i2c(void) {
    HAL_StatusTypeDef result;
    uint8_t i;
    char buf[64];
    DBG_PUT("Scanning I2C bus 2...\r\n");
    for (i = 1; i < 128; i++) {
        result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i << 1), 2, 2);
        if (result == HAL_OK) {
            sprintf(buf, "I2C address found: 0x%X\r\n", (uint16_t)(i));
            DBG_PUT(buf);
        }
    }
    DBG_PUT("Scan Complete.\r\n");
    return 0;
}
