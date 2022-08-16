/*
 * logger.c
 *
 *  Created on: Aug 9, 2022
 *      Author: jenish
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "logger.h"
#include "nand_types.h"
#include "nandfs.h"
#include "nand_m79a_lld.h"
#include "debug.h"
#include "iris_time.h"
#include "command_handler.h"

extern SPI_HandleTypeDef hspi1;

uint8_t logger_buffer[PAGE_DATA_SIZE];
uint16_t buffer_pointer;
PhysicalAddrs logger_addr = {0};

uint8_t curr_logger_block;
uint8_t curr_logger_page;

int _fill_buffer(uint8_t *data);
int _logger_write();

int logger_create() {
    NANDfs_format();

    curr_logger_block = 0;
    curr_logger_page = 0;

    logger_addr.block = curr_logger_block;
    logger_addr.page = curr_logger_page;

    buffer_pointer = 0;
}

int iris_log(const char *log_data, ...) {
    uint8_t output_array[LOG_TOTAL_LENGTH];
    memset(output_array, 0, LOG_TOTAL_LENGTH);

    uint8_t header_buffer[LOG_HEADER_LENGTH];
    uint8_t data_buffer[LOG_DATA_LENGTH];
    uint8_t footer_buffer[LOG_FOOTER_LENGTH];

    Iris_Timestamp timestamp;
    get_rtc_time(&timestamp);

    // Create header
    snprintf(header_buffer, LOG_HEADER_LENGTH, "%d/%d/%d %d:%d:%d: ", timestamp.Day, timestamp.Month,
             timestamp.Year, timestamp.Hour, timestamp.Minute, timestamp.Second);

    // Create data
    va_list arg;
    va_start(arg, log_data);
    vsnprintf(data_buffer, LOG_DATA_LENGTH, log_data, arg);

    // Create footer
    snprintf(footer_buffer, LOG_FOOTER_LENGTH, "\r\n");

    // Combine footer
    snprintf(output_array, LOG_TOTAL_LENGTH, "%s%s%s", header_buffer, data_buffer, footer_buffer);

    _fill_buffer(output_array);
}

int _fill_buffer(uint8_t *data) {
    uint8_t count = 0;

    if (buffer_pointer >= PAGE_DATA_SIZE) {
        clear_and_dump_buffer();
    }

    for (int i = buffer_pointer; i < (buffer_pointer + LOG_TOTAL_LENGTH); i++) {
        logger_buffer[i] = data[count];
        count++;
    }

    buffer_pointer += LOG_TOTAL_LENGTH;
}

int _logger_write() {
    NAND_ReturnType ret = NAND_Page_Program(&logger_addr, PAGE_DATA_SIZE, logger_buffer);
    if (ret != Ret_Success) {
        DBG_PUT("Error: prog b %d p %d r %d\r\n", logger_addr.block, logger_addr.page, ret);
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

int clear_and_dump_buffer() {
    _logger_write();
    buffer_pointer = 0;
    memset(logger_buffer, 0, PAGE_DATA_SIZE);
}

int read_from_block(uint8_t block, uint16_t page) {
    PhysicalAddrs addr = {0};
    uint8_t buffer[PAGE_DATA_SIZE];
    uint8_t char_count = 0;
    char str[128];

    addr.block = block;
    addr.page = page;

    NAND_ReturnType ret = NAND_Page_Read(&addr, PAGE_DATA_SIZE, buffer);
    if (ret != Ret_Success) {
        DBG_PUT("read b %d p %d r %d\r\n", block, page, ret);
        return ret;
    }
    for (int j = 0; j < PAGE_DATA_SIZE; j++) {
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
