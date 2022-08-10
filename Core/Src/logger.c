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

#define LOG_MESSAGE_LENGTH 125
#define LOG_MESSAGE_FOOTER_LENGTH 3
#define LOG_BLOCK_SWITCH_MASK 0x01

uint8_t logger_buffer[PAGE_LEN];
uint16_t buffer_pointer;
PhysicalAddrs logger_addr = {0};

uint8_t curr_logger_block = 0;
uint8_t curr_logger_page = 0;

int logger_create() {
    NANDfs_format();

    logger_addr.block = 0;
    logger_addr.page = 0;

    buffer_pointer = 0;
}

int sys_log(const char *log, ...) {
    char output_array[LOG_MESSAGE_LENGTH + LOG_MESSAGE_FOOTER_LENGTH];
    memset(output_array, 0, 128);
    va_list arg;
    va_start(arg, log);
    int chars_written = vsnprintf(output_array, 128, log, arg);
    output_array[LOG_MESSAGE_LENGTH + LOG_MESSAGE_FOOTER_LENGTH - 3] = '\r';
    output_array[LOG_MESSAGE_LENGTH + LOG_MESSAGE_FOOTER_LENGTH - 2] = '\n';

    _fill_buffer(output_array);
}

int _fill_buffer(char *data) {
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

    DBG_PUT("prog b %d p %d r %d\r\n", logger_addr.block, logger_addr.page, ret);

    curr_logger_page++;

    if (curr_logger_page >= NUM_PAGES_PER_BLOCK) {
        curr_logger_block = curr_logger_block ^ LOG_BLOCK_SWITCH_MASK;
        curr_logger_page = 0;

        logger_addr.block = curr_logger_block;

        ret = NAND_Block_Erase(&logger_addr);
        if (ret != Ret_Success) {
            DBG_PUT("Erase block %d failed", logger_addr.block);
            return ret;
        }
    }

    logger_addr.page = curr_logger_page;
}

int read_from_block(uint8_t block, uint16_t page) {
    PhysicalAddrs addr = {0};
    uint8_t buffer[PAGE_LEN];
    uint8_t char_count = 0;
    char str[128];

    addr.block = block;
    addr.page = page;

    NAND_ReturnType ret = NAND_Page_Read(&addr, PAGE_LEN, buffer);
    if (ret != Ret_Success) {
        DBG_PUT("read b %d p %d r %d\r\n", block, page, ret);
        return ret;
    }
    for (int j = 0; j < PAGE_LEN; j++) {
        if (char_count < 127) {
            str[char_count] = buffer[j];
            char_count++;
        } else {
            DBG_PUT(str);
            char_count = 0;
            memset(str, 0, 128);
        }
    }
}
