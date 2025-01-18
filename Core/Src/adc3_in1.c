#include "main.h"
#include "stm32g4xx_hal_uart.h"

#define SAMPLE_PERIOD 573
uint32_t count_HAL_ADC_ConvCpltCallback = 0;
uint8_t conversionIsReay = 0;
extern ADC_HandleTypeDef hadc3;
uint32_t ADC3_systemTickSnapshot = 0;
uint32_t adc_raw[2];
uint32_t adc3_5_1[2];

void read_adc3_IN1(void)
{
    if (conversionIsReay == 1)
    {
        conversionIsReay = 0;
        adc3_5_1[0] = __HAL_ADC_CALC_DATA_TO_VOLTAGE(3300, adc_raw[0], ADC_RESOLUTION_12B); // 3300 is VREF in mV.
        adc3_5_1[1] = __HAL_ADC_CALC_DATA_TO_VOLTAGE(3300, adc_raw[1], ADC_RESOLUTION_12B); // 3300 is VREF in mV.
        HAL_ADC_Start_IT(&hadc3);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hadc);
    static uint8_t i = 0;
    adc_raw[i] = HAL_ADC_GetValue(&hadc3);
    if (i < 1) i++;
    else
    {
        i = 0;
        conversionIsReay = 1;
    }
}

