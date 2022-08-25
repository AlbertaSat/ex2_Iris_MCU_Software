#include <iris_system.h>
#include <stdio.h>
#include "command_handler.h"
#include "debug.h"



extern I2C_HandleTypeDef hi2c2;


/**
 * @brief Scans I2C2 (internal) bus for devices. Debug purpose only
 *
 * @return int
 */
int uart_scan_i2c(void) {
    HAL_StatusTypeDef result;
    uint8_t i;
    char buf[64];
    iris_log("Scanning I2C bus 2...\r\n");
    for (i = 1; i < 128; i++) {
        result = HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)(i << 1), 2, 2);
        if (result == HAL_OK) {
            sprintf(buf, "I2C address found: 0x%X\r\n", (uint16_t)(i));
            iris_log(buf);
        }
    }
    iris_log("Scan Complete.\r\n");
    return 0;
}
