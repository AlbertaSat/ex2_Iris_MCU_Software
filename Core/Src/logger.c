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
#include "iris_time.h"

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
    uint8_t output_array[128];
    memset(output_array, 0, 128);

    uint8_t header_buffer[13];
    uint8_t data_buffer[128 - 13 - 2];

    Iris_Timestamp timestamp;

    get_rtc_time(&timestamp);

    // Create header
    sprintf(&output_array[0], "%d", timestamp.Day);
    output_array[1] = '/';
    sprintf(&output_array[2], "%d", timestamp.Month);
    output_array[3] = '/';
    output_array[4] = (timestamp.Year >> (8 * 1)) & 0xff;
    output_array[5] = (timestamp.Year >> (8 * 0)) & 0xff;
    output_array[6] = ' ';
    output_array[7] = timestamp.Hour;
    output_array[8] = ':';
    output_array[9] = timestamp.Minute;
    output_array[10] = ':';
    output_array[11] = timestamp.Second;
    output_array[12] = ':';

    // Create data
    va_list arg;
    va_start(arg, log);
    vsnprintf(data_buffer, 114, log, arg);

    // Combine all buffers
    strcat(output_array, header_buffer);
    strcat(output_array, data_buffer);

    // Create footer
    output_array[128 - 2] = '\n';
    output_array[128 - 1] = '\0';

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
