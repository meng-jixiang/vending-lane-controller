/**
 * @file    bsp_led.h
 * @brief   BSP 层 — LED 指示灯
 */
#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "main.h"
#include <stdint.h>

void    BSP_LED_Init(void);
void    BSP_LED_On(void);
void    BSP_LED_Off(void);
void    BSP_LED_Toggle(void);
void    BSP_LED_Blink(uint8_t times, uint16_t interval_ms);

#endif
