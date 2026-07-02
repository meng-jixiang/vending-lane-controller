/**
 * @file    bsp_595.h
 * @brief   BSP 层 — 74HC595 移位寄存器矩阵驱动
 *
 * P端(行/正极): PB12=DS, PB13=SHCP, PB14=STCP, PB15=MR
 * N端(列/负极): PA4=DS, PA5=SHCP, PA6=STCP, PA7=MR
 */
#ifndef __BSP_595_H
#define __BSP_595_H

#include "main.h"
#include "board_config.h"
#include <stdint.h>

void     BSP_595_Init(void);
void     BSP_595_SelectRow(uint8_t row);       /* 0-29, 0xFF=全关 */
void     BSP_595_SetColumns(uint32_t col_mask); /* bit0~bit29 */
void     BSP_595_AllOff(void);

#endif
