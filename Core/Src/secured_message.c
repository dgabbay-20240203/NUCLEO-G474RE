/*
File name: secured_message.c
Author: Dan Gabbay
Creation date: 26 Dec 2024
*/

#include "secured_message.h"
#include "unix_time_functions.h"
#include "stm32g4xx_hal_rtc.h"
#include "AT24C256B_i2c_eeprom.h"

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern struct sys_config sysConfig;

struct secured_message secMessage;
void Get_RTC_time_date(void);
void calc_sha_256(uint8_t hash[32], const void* input, size_t len);

void create_secured_msg(uint8_t *msgPayload)
{
	struct secured_message_extended temp;
    struct time_stamp tm;
    uint8_t i;

    Get_RTC_time_date();
    tm.year = sDate.Year + 2000;
    tm.month = sDate.Month;
    tm.day = sDate.Date;
    tm.hr = sTime.Hours;
    tm.min = sTime.Minutes;
    tm.sec = sTime.Seconds;

    temp.seed = TimeInSecondsSinceT_ZERO2(&tm);
    for (i = 0; i < 32; i++)
    {
        temp.secret_key[i] = sysConfig.secret_key[i];
    }

    for (i = 0; i < PAYLOAD_SIZE; i++)
    {
        temp.payload[i] = *(msgPayload + i);
    }

    calc_sha_256(&secMessage.sha256_signature[0], (const void*) &temp, sizeof(temp));
    secMessage.seed = temp.seed;
    for (i = 0; i < PAYLOAD_SIZE; i++)
    {
    	secMessage.payload[i] = temp.payload[i];
    }
}

