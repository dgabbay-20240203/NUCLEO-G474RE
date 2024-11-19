/*
File name: i2c1.c
Author: Dan Gabbay
Creation date: 17 Nov 2024
 */

#include "stm32g4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

uint8_t  I2C1_Error = 0;
static uint8_t  I2C1_Transmit_done = 1;
static uint8_t  I2C1_Receive_done = 1;
static uint8_t  I2C1_action = 0;
static uint8_t*  I2C1_transmit_buff_ptr;
static uint8_t*  I2C1_receive_buff_ptr;

static uint16_t I2C1_DevAddress;
static uint16_t I2C1_Msg_size;

void handleI2c1(void)
{
    if (I2C1_action == 0)
    {
        return;
    }

    switch(I2C1_action)
    {
    case 1:
    	HAL_I2C_Master_Transmit_IT(&hi2c1, I2C1_DevAddress, I2C1_transmit_buff_ptr, I2C1_Msg_size);
    	I2C1_action = 0;
    	break;
    case 2:
    	HAL_I2C_Master_Receive_IT(&hi2c1, I2C1_DevAddress, I2C1_receive_buff_ptr, I2C1_Msg_size);
    	I2C1_action = 0;
    	break;

    default:
    	I2C1_action = 0;
    }
}


void I2C1_tx_rx(uint16_t I2C1_DevAddr, uint16_t I2C1_msgSize, uint8_t opCode, uint8_t* buf_ptr)
{
	I2C1_DevAddress = I2C1_DevAddr;
	if (opCode == 1) // WRITE
    {
		I2C1_transmit_buff_ptr = buf_ptr;
		I2C1_Msg_size = I2C1_msgSize;
        I2C1_action = 1;
    }
	else
    if(opCode == 2) // READ
    {
    	I2C1_receive_buff_ptr = buf_ptr;
        I2C1_Msg_size = I2C1_msgSize;
    	I2C1_action = 2;
    }
}

uint8_t I2C1_status(void)
{
	if (I2C1_Error == 1)
	{
		I2C1_Error = 0;
		return 3;
	}
	else
    if (I2C1_Transmit_done == 1)
    {
    	I2C1_Transmit_done = 0;
    	return 1;
    }
    else
    if (I2C1_Receive_done == 1)
    {
    	I2C1_Receive_done = 0;
    	return 2;
    }

	return 0;
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);
  I2C1_Transmit_done = 1;

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_MasterTxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Master Rx Transfer completed callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);
  I2C1_Receive_done = 1;
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_MasterRxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  I2C error callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hi2c);
  I2C1_Error = 1;

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_I2C_ErrorCallback could be implemented in the user file
   */
}

