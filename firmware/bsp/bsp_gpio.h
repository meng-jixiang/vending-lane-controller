/**
 * @file    bsp_gpio.h
 * @brief   BSP 层 — GPIO 扩展（到位检测、光幕、NMOS输出）
 */
#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "main.h"
#include "board_config.h"
#include <stdint.h>

/* 到位检测 (10路，低电平=到位) */
uint8_t BSP_DI_GetRow(uint8_t row);         /* 返回 1=到位, 0=未到位 */
uint32_t BSP_DI_GetAll(void);               /* 返回 bit0~bit9 状态 */

/* 光幕 (2路，低电平=掉货) */
uint8_t BSP_Laser1_IsBlocked(void);         /* 1=有遮挡 */
uint8_t BSP_Laser2_IsBlocked(void);         /* 1=有遮挡 */

/* NMOS 输出 (4路) */
void    BSP_NMOS_On(uint8_t ch);            /* ch=0~3 */
void    BSP_NMOS_Off(uint8_t ch);
void    BSP_NMOS_AllOff(void);

#endif
