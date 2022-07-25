/*
 * iris_time.h
 *
 *  Created on: Jul 25, 2022
 *      Author: jenish
 */

#ifndef INC_IRIS_TIME_H_
#define INC_IRIS_TIME_H_

#include <iris_system.h>
#include <time.h>

#define LEAP_YEAR(Y) (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))

typedef struct {
    uint8_t Second;
    uint8_t Minute;
    uint8_t Hour;
    uint8_t Wday; // day of week, sunday is day 1
    uint8_t Day;
    uint8_t Month;
    uint8_t Year; // offset from 1970;
} tmElements_t;

static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void breakTime(time_t timeInput);

#endif /* INC_IRIS_TIME_H_ */
