/*
 * logger.c
 *
 *  Created on: Aug 9, 2022
 *      Author: jenish
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "nand_types.h"
#include "nandfs.h"
#include "nand_m79a_lld.h"
#include "debug.h"

#define PAGE_LEN 2048

uint8_t logger_buffer[PAGE_LEN];
uint16_t buffer_pointer;
PhysicalAddrs logger_addr = {0};

int logger_create() {
    logger_addr.block = 0;
    logger_addr.page = 0;

    buffer_pointer = 0;
}

int sys_log(const char *log, ...) {
    char output_array[128];
    memset(output_array, 0, 128);
    va_list arg;
    va_start(arg, log);
    int chars_written = vsnprintf(output_array, 128, log, arg);

    fill_buffer(output_array);
}

int fill_buffer(char *data) {
    uint8_t count = 0;

    if (buffer_pointer >= PAGE_LEN) {
        _logger_write();
        buffer_pointer = 0;
        memset(logger_buffer, 0, PAGE_LEN);
    }

    for (int i = buffer_pointer; i < (buffer_pointer + 128); i++) {
        logger_buffer[i] = data[count];
        count++;
    }

    buffer_pointer += 128;
}

int _logger_write() {
    NAND_ReturnType ret = NAND_Page_Program(&logger_addr, PAGE_LEN, logger_buffer);
    if (ret != Ret_Success) {
        DBG_PUT("prog b %d p %d r %d\r\n", logger_addr.block, logger_addr.page, ret);
        return ret;
    }
}

int read_from_block(uint16_t page) {
    PhysicalAddrs addr = {0};
    uint8_t buffer[PAGE_LEN];
    char *log;

    uint8_t block = 0;
    addr.block = block;

    NAND_ReturnType ret = NAND_Page_Read(&addr, PAGE_LEN, buffer);
    if (ret != Ret_Success) {
        DBG_PUT("read b %d p %d r %d\r\n", block, page, ret);
        return ret;
    }
    for (int j = 0; j < PAGE_LEN; j++) {
        DBG_PUT("buff b %d p %d d %d\n\r", block, page, buffer[j]);
    }
}
