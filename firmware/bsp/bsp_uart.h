/**
 * @file    bsp_uart.h
 * @brief   BSP 层 — UART 串口
 *
 * USART1 = 调试日志 (115200bps)
 * USART3 = RS-485 通讯 (9600bps, DMA)
 */
#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "main.h"
#include <stdint.h>

/* 调试口 (USART1) */
void    BSP_Debug_Init(void);
void    BSP_Debug_Printf(const char *fmt, ...);

/* RS-485 口 (USART3) */
void    BSP_RS485_Init(void);
void    BSP_RS485_Send(const uint8_t *data, uint16_t len);
void    BSP_RS485_RxStart(void);           /* 启动 1 字节接收中断 */
void    BSP_RS485_RxCallback(uint8_t byte); /* HAL 回调调用 */

/* 接收缓冲 */
uint8_t BSP_RS485_GetByte(uint8_t *byte);  /* 取 1 字节, 0=空 */

#endif
