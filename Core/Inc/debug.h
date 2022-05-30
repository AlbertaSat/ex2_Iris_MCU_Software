#ifndef DEBUG_DEFH
#define DEBUG_DEFH

#include "stm32l0xx_hal.h"
#include <stdio.h>
#include <string.h>
#include "main.h"

extern UART_HandleTypeDef huart1;

static inline void DBG_PUT(char *str) {
#ifdef UART_DEBUG
    HAL_UART_Transmit(&huart1, (uint8_t *) str, strlen(str), 100);
#endif
}

static inline char hex_2_ascii(uint8_t hex) {
    return (hex < 10) ? '0' + hex : 'a' + (hex - 10);
}

#endif // DEBUG_DEFH

