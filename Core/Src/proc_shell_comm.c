/*
File name: proc_shell_comm.c
Author: Dan Gabbay
Date: 05 October 2024
*/

//--------------------------------------------------------------------------------------------------
//********** Include files ***************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "proc_shell_comm.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_uart.h"
#include "stm32g4xx_hal_rng.h"
#include "stm32g4xx_hal_rtc.h"
#include "AT24C256B_i2c_eeprom.h"
#include "i2c1.h"
#include "crc32.h"
#include "unix_time_functions.h"
#include "main.h"
/*****************************************/
void Error_Handler(void);
extern struct sys_config sysConfig;
extern uint8_t  I2C1_Error;
uint8_t reading_eeprom_page = 0;
extern uint8_t sysConfigInfoValid;
commandTokens comm_tokens;
extern UART_HandleTypeDef hlpuart1;
uint8_t new_char = 0;
static uint8_t dump_mode = 0;
static uint8_t i2c1_status = 0;
static uint8_t generate_rand_seed = 0;
static uint8_t dialer_state = 0;
static uint16_t dtmf_on_time = 45;
static uint16_t dtmf_off_time = 45;
static uint8_t print_dtmf_symbol = 0;
uint8_t userPB;
const unsigned char HELLO[] = "Message number:";
extern uint32_t adc3_IN3_voltage;
extern uint32_t adc_val;
extern uint8_t IWDG_refreshEnabled;
extern uint32_t CAN_received_messages_counter;
extern uint8_t CAN_Tx_enabled;
extern uint32_t CAN_received_messages_counter;
extern uint8_t RxData[12];
extern FDCAN_RxHeaderTypeDef RxHeader;
extern RNG_HandleTypeDef hrng;
extern uint8_t SendMessage_IWDG_resetOccurred;
extern const uint32_t header;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;

void Transmit_I2C1(void);
void Set_RTC_time_date(void);
void Get_RTC_time_date(void);

static void CommandLineMode (void);
static uint8_t isDtmfChar(uint8_t ch);
static uint8_t IsValidDtmfString(const uint8_t *dtmfStr);

unsigned char ConvertStringToIndex (unsigned char *userCmd,
                                    const unsigned char **commadModeFunctions,
                                    unsigned char tabSize);

void commandLnTokens (commandTokens *commandTokensPtr, unsigned char *commandLnStr, unsigned char rx_buff_size);
static void SwitchToDumpMode(commandTokens *commTokPtr);
static unsigned char isNumber (const char *string);
static unsigned char withInUn32BitRange(const char *string);
static uint8_t ConvertDtmfSymbolToCode(uint8_t ch);
static uint8_t *dialer_str_ptr;

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};


uint8_t lpuart1_tx_buff[200];
uint8_t lpuart1_rx_buff[200];
uint8_t messageReadyToBeProcessed = 0;

static void ReportFirmwareVersion (void);

const char * const commadModeFunctions[NUM_OF_COM]= {
"fver",    // Report firmware version and build timestamp.
"dump",
"Iwdg",
"cantx",
"getseed",
"savesysconfig",
"readsysconfig",
"sn",
"tone",
"dtmf",
"setdtmftm",
"rtc",
"unix"
};

void handle_lpuart1_communication(void)
{
    static uint32_t systemTickSnapshot = 0;
    static unsigned long msgCounter = 0;
    static char first_time_only = 1;
    static uint32_t old_CAN_received_messages_counter = 0;
    uint32_t rnd_num;

    if (first_time_only == 1)
    {
        first_time_only = 0;
        HAL_UART_Receive_IT(&hlpuart1, (unsigned char *) &new_char, 1);
        old_CAN_received_messages_counter = CAN_received_messages_counter;
    }

    i2c1_status = I2C1_status();

    if (i2c1_status == 3)
    {
        I2C1_Error = 0;
        sprintf((char *) lpuart1_tx_buff, "I2C1 error!\n");
        HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
        i2c1_status = 0;
    }
    else
    if (i2c1_status == 2)
    {
        if (reading_eeprom_page == 1)
        {
            reading_eeprom_page = 0;
            if ((sysConfig.crc32 == crc32(CRC32_SEED, (const uint8_t *) &sysConfig, sizeof(sysConfig) - 4)) && (sysConfig.header == header))
            {
                sysConfigInfoValid = 1;
            }
            else
            {
                sysConfigInfoValid = 0;
            }
        }
    }

    if (dump_mode == 0)
    {
        CommandLineMode ();
    }
    else
    {
        switch(dump_mode)
        {
        case 1:
            if ((HAL_GetTick() - systemTickSnapshot) >= 1000)
            {
                if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 1)
                {
                    userPB = 1;
                }
                else
                {
                    userPB = 0;
                }

                systemTickSnapshot = HAL_GetTick();
                msgCounter++;
                sprintf((char *) lpuart1_tx_buff, "%s %lu, userPB = %d, adc3_IN3_voltage = %lu.%03luV, adc_val = %lu\n", HELLO, msgCounter, userPB, adc3_IN3_voltage /1000,adc3_IN3_voltage % 1000 , adc_val);
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            }
                break;
        case 2:
                if (old_CAN_received_messages_counter != CAN_received_messages_counter)
                {
                    // We have a new CAN message
                    sprintf((char *) lpuart1_tx_buff, "CAN message (%lu): ID =  %08X, Payload = %02X %02X %02X %02X %02X %02X %02X %02X \n", CAN_received_messages_counter,(unsigned int) RxHeader.Identifier ,RxData[0], RxData[1], RxData[2], RxData[3], RxData[4], RxData[5], RxData[6], RxData[7]);
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
                old_CAN_received_messages_counter = CAN_received_messages_counter;
                break;
        case 3:
            if ((HAL_GetTick() - systemTickSnapshot) >= 100) // Changing the period to less than 400 milliseconds may trigger a watchdog reset.
            {
                systemTickSnapshot = HAL_GetTick();
                HAL_RNG_GenerateRandomNumber(&hrng, &rnd_num);
                sprintf((char *) lpuart1_tx_buff, "%lu\n", rnd_num);
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            }
            break;
        default:
            dump_mode = 0;
            break;
        }
    }
}

static void CommandLineMode (void)
{
    unsigned char i = 0;
    char *ptr_end;
    uint8_t data;
    uint32_t user_input;
    uint32_t unix_time;
    struct time_stamp tm;
    if (SendMessage_IWDG_resetOccurred == 1)
    {
        SendMessage_IWDG_resetOccurred = 0;
        sprintf((char *) lpuart1_tx_buff, "IWDG reset occurred!\r\n");
        HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
    }

    if (messageReadyToBeProcessed == 1)
    {
        messageReadyToBeProcessed = 0;
        commandLnTokens (&comm_tokens, lpuart1_rx_buff, sizeof(lpuart1_rx_buff));
        i = ConvertStringToIndex (comm_tokens.commandTok[0],(const unsigned char **)commadModeFunctions,NUM_OF_COM);
        switch(i)
        {
        case 0:
            ReportFirmwareVersion();
            break;
        case 1:
            SwitchToDumpMode(&comm_tokens);
            break;
        case 2:
            if (comm_tokens.numOfTokens == 1)
            {
                IWDG_refreshEnabled = 0;
            }
            break;
        case 3: // Enable/ disable CAN transmission
            if (comm_tokens.numOfTokens == 2)
            {
                CAN_Tx_enabled = (int8_t) atoi((const char *) comm_tokens.commandTok[1]);
            }
            break;
        case 4: // getseed
            generate_rand_seed = 1;
            break;
        case 5: // Transmit once via I2C1
            save_sys_config();
            break;
        case 6:
            restore_sys_config();
            break;
        case 7: // sn - serial number
            if (comm_tokens.numOfTokens == 1)
            {
                if (sysConfigInfoValid == 1)
                {
                    sprintf((char *) lpuart1_tx_buff, "Serial number: %lu\r\n", sysConfig.unit_serial_num);
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
                else
                {
                    sprintf((char *) lpuart1_tx_buff, "Could not find valid serial number!\r\n");
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
            }
            else
            if (comm_tokens.numOfTokens == 2)
            {
                if ((isNumber ((const char *) comm_tokens.commandTok[1]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[1]) == 1))
                {
                    sysConfig.unit_serial_num = strtoul((const char *) comm_tokens.commandTok[1], &ptr_end, 10);
                    sprintf((char *) lpuart1_tx_buff, "\"sn\" was set to %lu\r\n", sysConfig.unit_serial_num);
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
                else
                {
                    sprintf((char *) lpuart1_tx_buff, "Number is out of range!\r\n");
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
            }
            break;
        case 8: // tone
            if (comm_tokens.numOfTokens == 2)
            {
                if ((isNumber ((const char *) comm_tokens.commandTok[1]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[1]) == 1))
                {
                    user_input = strtoul((const char *) comm_tokens.commandTok[1], &ptr_end, 10);
                    if (user_input < 64)
                    {
                        data = (uint8_t) user_input;
                        // Sending a code in the range of 0 to 63 to the signal generator chip PCD3312C.
                        if (HAL_I2C_Master_Transmit(&hi2c1, 0x48, (uint8_t *)&data, 1, 200) != HAL_OK) // This is blocking, it takes around 205 microseconds to complete.
                        {
                            sprintf((char *) lpuart1_tx_buff, "I2C1 error!\n");
                            HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                        }
                    }
                    else
                    {
                        sprintf((char *) lpuart1_tx_buff, "Command argument is out of range, must be in the range of 0 to 63.\r\n");
                        HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                    }
                }
                else
                {
                    sprintf((char *) lpuart1_tx_buff, "Command argument is out of range, must be in the range of 0 to 63.\r\n");
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
            }

            break;
        case 9: // dtmf
            if (comm_tokens.numOfTokens == 2)
            {
                if (IsValidDtmfString(comm_tokens.commandTok[1]) == 1)
                {
                    dialer_state = 1; // Start "dtmf" quick dialing.
                    dialer_str_ptr = (uint8_t *) comm_tokens.commandTok[1];
                }
                else
                {
                    sprintf((char *) lpuart1_tx_buff, "Invalid DTMF string!\r\n");
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
            }
            else
            {
                sprintf((char *) lpuart1_tx_buff, "Incorrect number of arguments, must be exactly one argument!\r\n");
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            }
            break;
        case 10: // setdtmftm
            if (comm_tokens.numOfTokens == 3)
            {
                if ((isNumber ((const char *) comm_tokens.commandTok[1]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[1]) == 1) &&
                    (isNumber ((const char *) comm_tokens.commandTok[2]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[2]) == 1))
                {
                    user_input = strtoul((const char *) comm_tokens.commandTok[1], &ptr_end, 10);
                    if ((user_input >= 40) && (user_input <= 1000))
                    {
                        dtmf_on_time = (uint16_t) user_input;
                    }

                    user_input = strtoul((const char *) comm_tokens.commandTok[2], &ptr_end, 10);
                    if ((user_input >= 20) && (user_input <= 500))
                    {
                        dtmf_off_time = (uint16_t) user_input;
                    }
                }
            }
            else
            if (comm_tokens.numOfTokens == 1)
            {
                sprintf((char *) lpuart1_tx_buff, "[dtmf_on_time, dtmf_off_time] = [%d, %d]\r\n", dtmf_on_time, dtmf_off_time);
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            }
            break;
        case 11:
            if (comm_tokens.numOfTokens == 7) // rtc YYYY MON Day Hours (24 mode) MIN SEC
            {
                if ((isNumber ((const char *) comm_tokens.commandTok[1]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[1]) == 1) &&
                    (isNumber ((const char *) comm_tokens.commandTok[2]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[2]) == 1) &&
                    (isNumber ((const char *) comm_tokens.commandTok[3]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[3]) == 1) &&
                    (isNumber ((const char *) comm_tokens.commandTok[4]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[4]) == 1) &&
                    (isNumber ((const char *) comm_tokens.commandTok[5]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[5]) == 1) &&
                    (isNumber ((const char *) comm_tokens.commandTok[6]) == 1) && (withInUn32BitRange((const char *) comm_tokens.commandTok[6]) == 1))
                {
                    sTime.Hours =   (uint8_t) strtoul((const char *) comm_tokens.commandTok[4], &ptr_end, 10);
                    sTime.Minutes = (uint8_t) strtoul((const char *) comm_tokens.commandTok[5], &ptr_end, 10);
                    sTime.Seconds = (uint8_t) strtoul((const char *) comm_tokens.commandTok[6], &ptr_end, 10);
                    sDate.Year =    (uint8_t)(strtoul((const char *) comm_tokens.commandTok[1], &ptr_end, 10) - 2000);
                    sDate.Month =   (uint8_t) strtoul((const char *) comm_tokens.commandTok[2], &ptr_end, 10);
                    sDate.Date =    (uint8_t) strtoul((const char *) comm_tokens.commandTok[3], &ptr_end, 10);
                    Set_RTC_time_date();
                    sprintf((char *) lpuart1_tx_buff, "RTC was set to %04d-%02d-%02d %02d:%02d:%02d UTC\r\n", sDate.Year + 2000, sDate.Month, sDate.Date,
                                                                                                  sTime.Hours, sTime.Minutes, sTime.Seconds);
                    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                }
            }
            else
            if (comm_tokens.numOfTokens == 1) // rtc
            {
                Get_RTC_time_date();
                sprintf((char *) lpuart1_tx_buff, "%04d-%02d-%02d %02d:%02d:%02d UTC\r\n", sDate.Year + 2000, sDate.Month, sDate.Date,
                                                                                              sTime.Hours, sTime.Minutes, sTime.Seconds);
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            }
            break;
        case 12: // unixtime
            Get_RTC_time_date();
            tm.year = sDate.Year + 2000;
            tm.month = sDate.Month;
            tm.day = sDate.Date;
            tm.hr = sTime.Hours;
            tm.min = sTime.Minutes;
            tm.sec = sTime.Seconds;
            unix_time = TimeInSecondsSinceT_ZERO2(&tm);
            sprintf((char *) lpuart1_tx_buff, "%lu\r\n", unix_time);
            HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            break;
        default:
            if (comm_tokens.numOfTokens != 0)
            {
                sprintf((char *) lpuart1_tx_buff, "Unrecognized command!\r\n");
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            }
            break;
        }
    }
}

void Quick_dtmf_dialer(void)
{
    static uint32_t dialer_time_stamp = 0;
    uint8_t dtmfCode;
    if (print_dtmf_symbol == 1)
    {
        print_dtmf_symbol = 0;
        sprintf((char *) lpuart1_tx_buff, "Printing DTMF symbol!\r\n");
        HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
    }
    if (dialer_state == 0)
    {
        return;
    }

    switch(dialer_state)
    {
    case 1:
        dtmfCode = ConvertDtmfSymbolToCode(*dialer_str_ptr);
        if (HAL_I2C_Master_Transmit(&hi2c1, 0x48, (uint8_t *)&dtmfCode, 1, 200) != HAL_OK) // This is blocking, it takes around 205 microseconds to complete.
        {
            sprintf((char *) lpuart1_tx_buff, "I2C1 error!\n");
            HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            dialer_state = 0;
        }
        else
        {
            dialer_time_stamp = HAL_GetTick();
            dialer_state = 2;
            dialer_str_ptr++;
        }
        break;
    case 2:
        if (HAL_GetTick() - dialer_time_stamp > dtmf_on_time)
        {
            dtmfCode = 0; // Turn off
            if (HAL_I2C_Master_Transmit(&hi2c1, 0x48, (uint8_t *)&dtmfCode, 1, 200) != HAL_OK) // This is blocking, it takes around 205 microseconds to complete.
            {
                sprintf((char *) lpuart1_tx_buff, "I2C1 error!\n");
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
                dialer_state = 0;
            }
            else
            {
                if (*dialer_str_ptr != 0)
                {
                    dialer_time_stamp = HAL_GetTick();
                    dialer_state = 3;
                }
                else
                {
                    dialer_state = 0;
                }
            }
        }
        break;
    case 3:
        if (HAL_GetTick() - dialer_time_stamp > dtmf_off_time)
        {
            dialer_state = 1;
        }
        break;
    default:
        dialer_state = 0;
        break;
    }

}

static void SwitchToDumpMode(commandTokens *commTokPtr)
{
    if (comm_tokens.numOfTokens == 2)
    {
        dump_mode = (int8_t) atoi((const char *) comm_tokens.commandTok[1]);
    }
}


static void ReportFirmwareVersion(void)
{
    sprintf((char *) lpuart1_tx_buff, "Firmware version: 1.00, Build timestamp: %s %s \r\n", __DATE__, __TIME__);
    HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
}

static uint8_t IsValidDtmfString(const uint8_t *dtmfStr)
{
    while (*dtmfStr != 0)
    {
        if (isDtmfChar(*dtmfStr) == 0)
        {
            return 0;
        }
        dtmfStr++;
    }
    return 1;
}

static uint8_t isDtmfChar(uint8_t ch)
{
    if (((ch <= 0x39) && (ch >= 0x30)) ||
        ((ch <= 0x44) && (ch >= 0x41)) ||
         (ch == '*')                   ||
         (ch == '#'))
    {
        return 1;
    }
    return 0;
}


static uint8_t ConvertDtmfSymbolToCode(uint8_t ch)
{
    if ((ch <= 0x39) && (ch >= 0x30))
    {
        return ch - 32;
    }

    if ((ch <= 0x44) && (ch >= 0x41))
    {
        return ch - 0x27;
    }

    if (ch == '*')
    {
        return 0x1e;
    }

    if (ch == '#')
    {
        return 0x1f;
    }
    return 0;
}

void Generate_256BIT_RandomSeed(void)
{
    static uint32_t randomSeed[8];
    static uint8_t  indx = 0;
    static uint32_t systemTickSnapshot;
    uint32_t rnd_num;
    uint8_t *ptr = (uint8_t *) &randomSeed[0];

    if (generate_rand_seed == 0)
    {
        return;
    }

    switch (generate_rand_seed)
    {
    case 1:
        HAL_RNG_GenerateRandomNumber(&hrng, &rnd_num);
        randomSeed[indx++] = rnd_num;

        if (indx < 8)
        {
            generate_rand_seed = 2;
            systemTickSnapshot = HAL_GetTick();
        }
        else
        {
            generate_rand_seed = 0;
            indx = 0;
            sprintf((char *) lpuart1_tx_buff, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
                    *(ptr +31), *(ptr +30),  *(ptr +29), *(ptr +28), *(ptr +27), *(ptr +26), *(ptr +25), *(ptr +24), *(ptr +23), *(ptr + 22),
                    *(ptr + 21),*(ptr + 20), *(ptr + 19),*(ptr + 18),*(ptr + 17),*(ptr + 16),*(ptr + 15),*(ptr +14), *(ptr + 13),*(ptr + 12),
                    *(ptr + 11),*(ptr + 10),*(ptr + 9),  *(ptr + 8), *(ptr + 7), *(ptr + 6), *(ptr + 5), *(ptr + 4), *(ptr + 3), *(ptr + 2),
                    *(ptr + 1), *ptr);
            HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
        }
        break;
    case 2:
        if (HAL_GetTick() > systemTickSnapshot + 2)
        {
            generate_rand_seed = 1;
        }
        break;

    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    static uint8_t recIndex = 0;

    HAL_UART_Receive_IT(&hlpuart1, (unsigned char *) &new_char, 1);

    if (new_char == CTRL_C)
    {
        // Terminate "dump" mode.
        dump_mode = 0;
    }
    else
    if(dump_mode == 0)
    {
        if (new_char == BACKSPACE)
        {
            if (recIndex > 0)
            {
                recIndex--;
            }
        }
        else
        if (new_char == CR_ENTER)
        {
            messageReadyToBeProcessed = 1;
            lpuart1_rx_buff[recIndex] = 0; // Terminate the command line string with NULL character.
            recIndex = 0;
        }
        else
        {
            if (recIndex < sizeof(lpuart1_rx_buff) - 1)
            {
                lpuart1_rx_buff[recIndex++] = new_char;
            }
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  //OkayToTransmit = 1;
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_TxCpltCallback can be implemented in the user file.
   */
}


static unsigned char isNumber (const char *string)
{
    unsigned char i=0;
    while (*(string+i))
    {
        if (!((*(string+i)>=48)&&(*(string+i)<=57))) return (0);
        i++;
    }
    return 1;
}


static unsigned char withInUn32BitRange(const char *string)
{
    unsigned char i;
    if ((i=strlen(string))>10) return 0;
    if (isNumber(string)==0) return 0;
    if ((i==10)&&(strcmp(string, "4294967295")>0)) return (0);
    return 1;
}


void Set_RTC_time_date(void)
{
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}
void Get_RTC_time_date(void)
{
    if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);
  if (GPIO_Pin == dtmf_StD_Pin)
  {
      print_dtmf_symbol = 1;
  }

  /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */
}
