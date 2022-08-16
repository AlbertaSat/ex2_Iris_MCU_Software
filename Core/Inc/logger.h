/*
 * logger.h
 *
 *  Created on: Aug 13, 2022
 *      Author: jenish
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

// Logger gets two blocks to save its data. These blocks
// can be arbitrarily chosen, but they must be consecutive
#define LOG_BLOCK_LOW 0
#define LOG_BLOCK_HIGH 1

#if LOG_BLOCK_HIGH - LOG_BLOCK_LOW != 1
#error "Log blocks must be consecutive to each other"
#endif

#define LOG_TOTAL_LENGTH 128
#define LOG_HEADER_LENGTH 22                                                       // Including NULL terminator
#define LOG_FOOTER_LENGTH 3                                                        // Including NULL terminator
#define LOG_DATA_LENGTH (LOG_TOTAL_LENGTH - LOG_HEADER_LENGTH - LOG_FOOTER_LENGTH) // Including NULL terminator
#define LOG_BLOCK_SWITCH_MASK 0x01

int logger_create();
int iris_log(const char *log_data, ...);
int clear_and_dump_buffer();

#endif /* INC_LOGGER_H_ */
