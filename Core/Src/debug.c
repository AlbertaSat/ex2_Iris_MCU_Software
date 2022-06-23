
#include "stm32l0xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

extern UART_HandleTypeDef huart1;

void DBG_PUT(const char *format, ...){
#ifdef UART_DEBUG
	static char output_array[128];
	va_list arg;
	va_start(arg, format);
	int chars_written = vsnprintf(output_array, 128, format, arg);
    HAL_UART_Transmit(&huart1, (uint8_t *)output_array, chars_written, 100);
    va_end(arg);
#endif
}
