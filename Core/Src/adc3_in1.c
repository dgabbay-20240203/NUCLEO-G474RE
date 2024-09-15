#include "main.h"
#include "stm32g4xx_hal_uart.h"

uint32_t count_HAL_ADC_ConvCpltCallback = 0;
uint8_t conversionIsReay = 0;
extern ADC_HandleTypeDef hadc3;
uint32_t ADC3_systemTickSnapshot = 0;
uint32_t adc_val;

void read_adc3_IN1(void)
{

        if ((HAL_GetTick() - ADC3_systemTickSnapshot) >= 1000)
        {
        	 ADC3_systemTickSnapshot = HAL_GetTick();
            if (conversionIsReay == 1)
            {
            	adc_val = HAL_ADC_GetValue(&hadc3);
    	        conversionIsReay = 0;
				HAL_ADC_Start_IT(&hadc3);
            }
        }
}








void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  count_HAL_ADC_ConvCpltCallback++;
  conversionIsReay = 1;

  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_ConvCpltCallback must be implemented in the user file.
   */
}

