/*
File name: unix_time_functions.c
Author: Dan Gabbay
Date: 15 Dec 2024
*/


#include "unix_time_functions.h"

#define YEARZERO   1970
#define DAYZERO    4 /* Thursday, 01 Jan 1970 */

static const unsigned char monthsDays[12] =     { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const unsigned char monthsDaysLeap[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static unsigned int DayNumberInAyear2(unsigned int year, unsigned char month, unsigned char dayOfMonth);
static unsigned char IsLeapYear(unsigned int year);

void Convert_UNIX_Time_Stamp_ToHumanReadableFormat(unsigned long seconds, struct time_stamp *ts)
{
    unsigned int i = YEARZERO;
    unsigned long timeOfDay;
    unsigned long totalNumOfDays = 0;
    unsigned char numOfDaysInMonth;
    timeOfDay = seconds % 86400;
    seconds -= timeOfDay;
    while (1)
    {
        if (IsLeapYear(i))
        {
            if (seconds >= 31622400)
            {
                seconds -= 31622400;
                totalNumOfDays += 366;
            }

            else
                break;
        }
        else
        {
            if (seconds >= 31536000)
            {
                seconds -= 31536000;
                totalNumOfDays += 365;
            }
            else
                break;
        }
        i++;
    }
    ts->year = i;

    i = 0;
    numOfDaysInMonth = 31;
    while (seconds >= 86400 * (unsigned long)numOfDaysInMonth)
    {

        seconds -= 86400 * (unsigned long)numOfDaysInMonth;
        totalNumOfDays += numOfDaysInMonth;
        i++;

        if (IsLeapYear(ts->year) == 0) numOfDaysInMonth = monthsDays[i];
        else
            numOfDaysInMonth = monthsDaysLeap[i];
    }

    ts->month = (unsigned char)i + 1;
    ts->day = (unsigned char)(seconds / 86400 + 1);
    ts->hr = (unsigned char)(timeOfDay / 3600);
    ts->min = (unsigned char)((timeOfDay - (unsigned long)(ts->hr) * 3600) / 60);
    ts->sec = (unsigned char)(timeOfDay % 60);

    totalNumOfDays += ts->day;
    ts->dayOfWeek = (totalNumOfDays + DAYZERO - 1) % 7;
}


// T=0 is defined as January 1, YEARZERO (defined at the top of this file) 00:00:00 UTC (Coordinated Universal Time, also known as GMT)
// The function returns the elapsed time in seconds from T=0 
uint32_t TimeInSecondsSinceT_ZERO2(const struct time_stamp *ts)
{
    unsigned long seconds = 0;
    unsigned int i;

    if (ts->year<YEARZERO) return (0);

    for (i = YEARZERO; i< ts->year; i++)
    {
        if (IsLeapYear(i))
        {
            seconds += 366;
        }
        else
        {
            seconds += 365;
        }
    }

    seconds += DayNumberInAyear2(ts->year, ts->month, ts->day) - 1;
    seconds = seconds * 86400 + (unsigned long)ts->hr * 3600 + (unsigned long)ts->min * 60 + (unsigned long)ts->sec;
    return (seconds);
}

static unsigned int DayNumberInAyear2(unsigned int year, unsigned char month, unsigned char dayOfMonth)
{
    unsigned int i, numOfday = 0;
    for (i = 0; i< (unsigned int) month - 1; i++)
    {
        numOfday += (unsigned int)monthsDays[i];
    }
    numOfday += dayOfMonth;
    if ((month>2) && (IsLeapYear(year))) numOfday++;
    return (numOfday);
}

static unsigned char IsLeapYear(unsigned int year)
{
    if (year % 4)      return (0);
    if (year % 100)    return (1);
    if (year % 400 == 0) return (1);
    return (0);
}
