/**
 * @file    bsp_uart.c
 * @brief   BSP 层 — UART 串口
 */
#include "bsp_uart.h"
#include "board_config.h"
#include <string.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

/* 环形接收缓冲 */
#define RX_BUF_SIZE  64
static uint8_t  s_rx_buf[RX_BUF_SIZE];
static volatile uint8_t s_rx_head = 0;
static volatile uint8_t s_rx_tail = 0;

/* ---- 调试口 ---- */

void BSP_Debug_Init(void)
{
    /* CubeMX 已初始化，此处可加额外配置 */
}

void BSP_Debug_Printf(const char *fmt, ...)
{
    /* 简单实现：通过 USART1 发送（后续可接 printf 重定向） */
    (void)fmt;
}

/* ---- RS-485 口 ---- */

void BSP_RS485_Init(void)
{
    s_rx_head = 0;
    s_rx_tail = 0;
}

void BSP_RS485_Send(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)data, len, 100);
}

void BSP_RS485_RxStart(void)
{
    uint8_t dummy;
    HAL_UART_Receive_IT(&huart3, &dummy, 1);
}

void BSP_RS485_RxCallback(uint8_t byte)
{
    uint8_t next = (s_rx_head + 1) % RX_BUF_SIZE;
    if (next != s_rx_tail) {
        s_rx_buf[s_rx_head] = byte;
        s_rx_head = next;
    }
}

uint8_t BSP_RS485_GetByte(uint8_t *byte)
{
    if (s_rx_head == s_rx_tail) return 0;
    *byte = s_rx_buf[s_rx_tail];
    s_rx_tail = (s_rx_tail + 1) % RX_BUF_SIZE;
    return 1;
}
