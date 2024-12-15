/*
File name: unix_time_functions.h
Author: Dan Gabbay
Date: 15 Dec 2024
*/

#ifndef UNIX_TIME_FUNCTIONS_H
#define UNIX_TIME_FUNCTIONS_H
#include "stm32g4xx_hal.h"
struct time_stamp {
   uint16_t year;
   uint8_t  month;
   uint8_t  day;
   uint8_t  hr;
   uint8_t  min;
   uint8_t  sec;
   uint8_t  dayOfWeek;
};

void Convert_UNIX_Time_Stamp_ToHumanReadableFormat(unsigned long seconds, struct time_stamp *ts);
uint32_t TimeInSecondsSinceT_ZERO2(const struct time_stamp *ts);

#endif

