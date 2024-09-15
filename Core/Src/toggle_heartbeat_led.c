 /******************************************************************************
  * @file           : toggle_heartbeat_led.c
  * @brief          : Toggles heartbeat LED periodically once every 500 milliseconds.
  *******************************************************************************
  *
  *
  * Creation date: September 1, 2024
  * Author: Dan Gabbay
  *
  ******************************************************************************/

#include "stm32g4xx_hal.h"

void ToggleHeartbeatLED(void)
{
    static uint32_t systemTickSnapshot = 0;
    static uint8_t toggle_flag = 0;
    // Heartbeat LED is toggled periodically once every 500 milliseconds.
    if ((HAL_GetTick() - systemTickSnapshot) >= 500)
    {
        systemTickSnapshot = HAL_GetTick();

        if (toggle_flag != 0)
        {
      	    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
      	    toggle_flag = 0;
        }
        else
        {
      	    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
      	    toggle_flag = 1;
        }
    }
}
