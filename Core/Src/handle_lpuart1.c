/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : handle_lpuart1.c
  * @brief          : Handles LPUART1 communication.
  ******************************************************************************
  * 
  *
  * Creation date: August 31, 2024
  * Author: Dan Gabbay
  *
  ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "stm32g4xx_hal_uart.h"

const unsigned char HELLO[] = "Message number:";
unsigned char lpuart1_tx_buff[100];
unsigned long msgCounter = 0;
uint8_t OkayToTransmit = 1;
uint8_t userPB;
uint8_t new_char = 0;
uint32_t receiveCounter = 0;
extern uint8_t IWDG_refreshEnabled;

void handle_lpuart1_communication(void)
{
	static char first_time_only = 1;
	static uint32_t systemTickSnapshot = 0;

	if (first_time_only == 1)
	{
		first_time_only = 0;
		HAL_UART_Receive_IT(&hlpuart1, (unsigned char *) &new_char, 1);
	}
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 1)
    {
    	userPB = 1;
    }
    else
    {
    	userPB = 0;
    }

    if (new_char == 'Z')
    {
        IWDG_refreshEnabled = 0;
    }

    if ((HAL_GetTick() - systemTickSnapshot) >= 100)
    {
        systemTickSnapshot = HAL_GetTick();
	    if (OkayToTransmit != 0)
        {
            msgCounter++;
            OkayToTransmit = 0;
            sprintf((char *) lpuart1_tx_buff, "%s %lu, userPB = %d, Last_char = %d, receiveCounter = %lu\n", HELLO, msgCounter, userPB, new_char, receiveCounter);
            HAL_UART_Transmit_IT(&hlpuart1, lpuart1_tx_buff, strlen((const char *)lpuart1_tx_buff));
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    receiveCounter++;

  HAL_UART_Receive_IT(&hlpuart1, (unsigned char *) &new_char, 1);
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback can be implemented in the user file.
   */
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  OkayToTransmit = 1;
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_TxCpltCallback can be implemented in the user file.
   */
}
