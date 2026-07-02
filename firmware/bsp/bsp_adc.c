/**
 * @file    bsp_adc.c
 * @brief   BSP 层 — ADC 电流/温度检测
 */
#include "bsp_adc.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

void BSP_ADC_Init(void)
{
    /* CubeMX 已初始化 ADC1/ADC2 */
}

static uint16_t adc_read(ADC_HandleTypeDef *hadc)
{
    HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, 10);
    return (uint16_t)HAL_ADC_GetValue(hadc);
}

uint16_t BSP_ADC_ReadCurrent(void)
{
    return adc_read(&hadc1);  /* PA0, ADC1_CH0 */
}

uint16_t BSP_ADC_ReadCurrent_mA(void)
{
    uint16_t adc_val = BSP_ADC_ReadCurrent();
    return (uint16_t)(adc_val * 1.91f);  /* I(mA) = ADC × 1.91 */
}

uint16_t BSP_ADC_ReadTemp(void)
{
    return adc_read(&hadc2);  /* PA1, ADC2_CH1 */
}
