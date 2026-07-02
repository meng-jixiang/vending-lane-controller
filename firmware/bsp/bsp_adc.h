/**
 * @file    bsp_adc.h
 * @brief   BSP 层 — ADC 电流/温度检测
 */
#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "main.h"
#include "board_config.h"
#include <stdint.h>

void     BSP_ADC_Init(void);
uint16_t BSP_ADC_ReadCurrent(void);    /* 返回 ADC 原始值 (0~4095) */
uint16_t BSP_ADC_ReadCurrent_mA(void); /* 返回电流值 (mA) */
uint16_t BSP_ADC_ReadTemp(void);       /* 返回 ADC 原始值 */

#endif
