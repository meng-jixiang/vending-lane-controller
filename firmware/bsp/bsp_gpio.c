/**
 * @file    bsp_gpio.c
 * @brief   BSP 层 — GPIO 扩展
 */
#include "bsp_gpio.h"

/* 到位检测引脚表 (row 0~9) */
static const struct { GPIO_TypeDef *port; uint16_t pin; } s_di_table[10] = {
    { DI_ROW0_PORT, DI_ROW0_PIN },   /* PC13 */
    { DI_ROW1_PORT, DI_ROW1_PIN },   /* PC14 */
    { DI_ROW2_PORT, DI_ROW2_PIN },   /* PC15 */
    { DI_ROW3_PORT, DI_ROW3_PIN },   /* PB3  */
    { DI_ROW4_PORT, DI_ROW4_PIN },   /* PB4  */
    { DI_ROW5_PORT, DI_ROW5_PIN },   /* PB5  */
    { DI_ROW6_PORT, DI_ROW6_PIN },   /* PB6  */
    { DI_ROW7_PORT, DI_ROW7_PIN },   /* PB7  */
    { DI_ROW8_PORT, DI_ROW8_PIN },   /* PB8  */
    { DI_ROW9_PORT, DI_ROW9_PIN },   /* PB9  */
};

/* NMOS 引脚表 */
static const struct { GPIO_TypeDef *port; uint16_t pin; } s_nmos_table[4] = {
    { NMOS1_PORT, NMOS1_PIN },
    { NMOS2_PORT, NMOS2_PIN },
    { NMOS3_PORT, NMOS3_PIN },
    { NMOS4_PORT, NMOS4_PIN },
};

/* ---- 到位检测 ---- */

uint8_t BSP_DI_GetRow(uint8_t row)
{
    if (row > 9) return 0;
    return (HAL_GPIO_ReadPin(s_di_table[row].port, s_di_table[row].pin) == GPIO_PIN_RESET) ? 1 : 0;
}

uint32_t BSP_DI_GetAll(void)
{
    uint32_t result = 0;
    for (uint8_t i = 0; i < 10; i++) {
        if (BSP_DI_GetRow(i)) result |= (1UL << i);
    }
    return result;
}

/* ---- 光幕 ---- */

uint8_t BSP_Laser1_IsBlocked(void)
{
    return (HAL_GPIO_ReadPin(LASER1_PORT, LASER1_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}

uint8_t BSP_Laser2_IsBlocked(void)
{
    return (HAL_GPIO_ReadPin(LASER2_PORT, LASER2_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}

/* ---- NMOS 输出 ---- */

void BSP_NMOS_On(uint8_t ch)
{
    if (ch < 4) {
        HAL_GPIO_WritePin(s_nmos_table[ch].port, s_nmos_table[ch].pin, GPIO_PIN_SET);
    }
}

void BSP_NMOS_Off(uint8_t ch)
{
    if (ch < 4) {
        HAL_GPIO_WritePin(s_nmos_table[ch].port, s_nmos_table[ch].pin, GPIO_PIN_RESET);
    }
}

void BSP_NMOS_AllOff(void)
{
    for (uint8_t i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(s_nmos_table[i].port, s_nmos_table[i].pin, GPIO_PIN_RESET);
    }
}
