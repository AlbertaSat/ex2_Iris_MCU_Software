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

static int fill_buffer(uint8_t *data);
static int write_to_nand();

struct {
    uint8_t buffer[PAGE_DATA_SIZE];
    uint16_t buffer_pointer;
    PhysicalAddrs addr;
} logger;

void logger_create() {
    logger.addr.block = 0;
    logger.addr.page = 0;
    logger.buffer_pointer = 0;
}

int iris_log(const char *log_data, ...) {
    int ret;

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

    ret = fill_buffer(output_array);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

static int fill_buffer(uint8_t *data) {
    uint8_t count = 0;
    int ret;

    if (logger.buffer_pointer >= PAGE_DATA_SIZE) {
        ret = clear_and_dump_buffer();
        if (ret < 0) {
            return -1;
        }
    }

    for (int i = logger.buffer_pointer; i < (logger.buffer_pointer + LOG_TOTAL_LENGTH); i++) {
        logger.buffer[i] = data[count];
        count++;
    }

    logger.buffer_pointer += LOG_TOTAL_LENGTH;
    return 0;
}

static int write_to_nand() {
    NAND_ReturnType ret = NAND_Page_Program(&logger.addr, PAGE_DATA_SIZE, logger.buffer);
    if (ret != Ret_Success) {
        DBG_PUT("Error: prog b %d p %d r %d\r\n", logger.addr.block, logger.addr.page, ret);
        return ret;
    }

    // DBG_PUT("prog b %d p %d r %d\r\n", logger.addr.block, logger.addr.page, ret);

    logger.addr.page++;

    if (logger.addr.page >= NUM_PAGES_PER_BLOCK) {
        logger.addr.block = logger.addr.block ^ LOG_BLOCK_SWITCH_MASK;
        logger.addr.page = 0;

        ret = NAND_Block_Erase(&logger.addr);
        if (ret != Ret_Success) {
            DBG_PUT("Erase block %d failed", logger.addr.block);
            return ret;
        }
    }
    return 0;
}

int clear_and_dump_buffer() {
    int ret;
    ret = write_to_nand();

    if (ret != 0) {
        return -1;
    }

    logger.buffer_pointer = 0;
    memset(logger.buffer, 0, PAGE_DATA_SIZE);

    return 0;
}

int logger_clear() {
    PhysicalAddrs addr = {0};

    for (uint8_t blk = LOG_BLOCK_LOW; blk <= LOG_BLOCK_HIGH; blk++) {
        addr.block = blk;
        addr.page = 0;

        NAND_ReturnType ret = NAND_Block_Erase(&addr);
        if (ret != Ret_Success) {
            DBG_PUT("Error in deleting block b %d p %d r %d\r\n", addr.block, addr.page, ret);
            return ret;
        }
    }
    return 0;
}
