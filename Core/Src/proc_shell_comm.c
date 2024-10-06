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
/*****************************************/

commandTokens comm_tokens;
extern UART_HandleTypeDef hlpuart1;
uint8_t new_char = 0;
static uint8_t dump_mode = 0;
uint8_t userPB;
const unsigned char HELLO[] = "Message number:";
extern uint32_t adc3_IN3_voltage;
extern uint32_t adc_val;
extern uint8_t IWDG_refreshEnabled;
extern uint32_t CAN_received_messages_counter;
extern uint8_t CAN_Tx_enabled;

static void CommandLineMode (void);

unsigned char ConvertStringToIndex (unsigned char *userCmd,
                                    const unsigned char **commadModeFunctions,
                                    unsigned char tabSize);

void commandLnTokens (commandTokens *commandTokensPtr, unsigned char *commandLnStr, unsigned char rx_buff_size);
static void SwitchToDumpMode(commandTokens *commTokPtr);

uint8_t lpuart1_tx_buff[200];
uint8_t lpuart1_rx_buff[200];
uint8_t messageReadyToBeProcessed = 0;

static void ReportFirmwareVersion (void);

const char * const commadModeFunctions[NUM_OF_COM]= {
"fver",    // Report firmware version and build timestamp.
"dump",
"Iwdg",
"cantx"
};

void handle_lpuart1_communication(void)
{
    static uint32_t systemTickSnapshot = 0;
    static unsigned long msgCounter = 0;
	static char first_time_only = 1;

	if (first_time_only == 1)
	{
		first_time_only = 0;
		HAL_UART_Receive_IT(&hlpuart1, (unsigned char *) &new_char, 1);
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
                sprintf((char *) lpuart1_tx_buff, "%s %lu, userPB = %d, adc3_IN3_voltage = %lu.%luV, adc_val = %lu, CAN_received_messages_counter = %lu\n", HELLO, msgCounter, userPB, adc3_IN3_voltage /100,adc3_IN3_voltage % 100 , adc_val, CAN_received_messages_counter);
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
                sprintf((char *) lpuart1_tx_buff, "Forcing IWDG reset.\r\n");
                HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
            }
            break;
        case 3: // Enable/ disable CAN transmission
        	if (comm_tokens.numOfTokens == 2)
        	{
        		CAN_Tx_enabled = (int8_t) atoi((const char *) comm_tokens.commandTok[1]);
        	}
        	break;
        default:

        	break;
        }
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

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  //OkayToTransmit = 1;
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_TxCpltCallback can be implemented in the user file.
   */
}

