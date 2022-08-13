/*
 * logger.h
 *
 *  Created on: Aug 13, 2022
 *      Author: jenish
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

#define LOG_TOTAL_LENGTH 128
#define LOG_HEADER_LENGTH 22                                                       // Including NULL terminator
#define LOG_FOOTER_LENGTH 3                                                        // Including NULL terminator
#define LOG_DATA_LENGTH (LOG_TOTAL_LENGTH - LOG_HEADER_LENGTH - LOG_FOOTER_LENGTH) // Including NULL terminator
#define LOG_BLOCK_SWITCH_MASK 0x01

int logger_create();
int sys_log(const char *log, ...);

#endif /* INC_LOGGER_H_ */
