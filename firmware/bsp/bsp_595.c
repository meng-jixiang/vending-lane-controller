/**
 * @file    bsp_595.c
 * @brief   BSP 层 — 74HC595 移位寄存器矩阵驱动
 */
#include "bsp_595.h"

/* ---- 内部：P端移位发送 32 位 ---- */
static void p595_shift(uint32_t data)
{
    for (int8_t i = 31; i >= 0; i--) {
        HAL_GPIO_WritePin(P595_DS_PORT, P595_DS_PIN,
                          (data & (1UL << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(P595_SHCP_PORT, P595_SHCP_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(P595_SHCP_PORT, P595_SHCP_PIN, GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(P595_STCP_PORT, P595_STCP_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(P595_STCP_PORT, P595_STCP_PIN, GPIO_PIN_RESET);
}

/* ---- 内部：N端移位发送 32 位 ---- */
static void n595_shift(uint32_t data)
{
    for (int8_t i = 31; i >= 0; i--) {
        HAL_GPIO_WritePin(N595_DS_PORT, N595_DS_PIN,
                          (data & (1UL << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(N595_SHCP_PORT, N595_SHCP_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(N595_SHCP_PORT, N595_SHCP_PIN, GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(N595_STCP_PORT, N595_STCP_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(N595_STCP_PORT, N595_STCP_PIN, GPIO_PIN_RESET);
}

/* ---- 公开 API ---- */

void BSP_595_Init(void)
{
    HAL_GPIO_WritePin(P595_MR_PORT, P595_MR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(P595_MR_PORT, P595_MR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(N595_MR_PORT, N595_MR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(N595_MR_PORT, N595_MR_PIN, GPIO_PIN_SET);
    BSP_595_AllOff();
}

void BSP_595_SelectRow(uint8_t row)
{
    p595_shift((row < BOARD_ROWS) ? (1UL << row) : 0);
}

void BSP_595_SetColumns(uint32_t col_mask)
{
    n595_shift(col_mask);
}

void BSP_595_AllOff(void)
{
    p595_shift(0);
    n595_shift(0);
}
