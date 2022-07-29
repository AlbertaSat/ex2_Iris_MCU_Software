/*
 * iris_time.c
 *
 *  Created on: Jul 25, 2022
 *      Author: jenish
 */

#include <iris_system.h>
#include "iris_time.h"
#include "time.h"

void convertUnixToUTC(time_t timeInput, Iris_Timestamp *timestamp) {
    // break the given time_t into time components
    // this is a more compact version of the C library localtime function
    // note that year is offset from 1970 !!!

    uint8_t year;
    uint8_t month, monthLength;
    uint32_t time;
    unsigned long days;

    time = (uint32_t)timeInput;
    timestamp->Second = time % 60;
    time /= 60; // now it is minutes
    timestamp->Minute = time % 60;
    time /= 60; // now it is hours
    timestamp->Hour = time % 24;
    time /= 24;                             // now it is days
    timestamp->Wday = ((time + 4) % 7) + 1; // Sunday is day 1

    year = 0;
    days = 0;
    while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
        year++;
    }
    timestamp->Year = year; // year is offset from 1970

    days -= LEAP_YEAR(year) ? 366 : 365;
    time -= days; // now it is days in this year, starting at 0

    days = 0;
    month = 0;
    monthLength = 0;
    for (month = 0; month < 12; month++) {
        if (month == 1) { // february
            if (LEAP_YEAR(year)) {
                monthLength = 29;
            } else {
                monthLength = 28;
            }
        } else {
            monthLength = monthDays[month];
        }

        if (time >= monthLength) {
            time -= monthLength;
        } else {
            break;
        }
    }
    timestamp->Month = month + 1; // jan is month 1
    timestamp->Day = time + 1;    // day of month
}
