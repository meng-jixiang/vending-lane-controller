/**
 * @file    laser.c
 * @brief   Device 层 — 光幕控制
 */
#include "laser.h"
#include "bsp_gpio.h"
#include "main.h"

static uint32_t s_detect_start = 0;
static uint16_t s_block_time  = 0;

void Laser_Init(void)
{
    s_detect_start = 0;
    s_block_time = 0;
}

uint8_t Laser_SelfCheck(void)
{
    return (BSP_Laser1_IsBlocked() || BSP_Laser2_IsBlocked()) ? 1 : 0;
}

uint8_t Laser1_IsBlocked(void) { return BSP_Laser1_IsBlocked(); }
uint8_t Laser2_IsBlocked(void) { return BSP_Laser2_IsBlocked(); }

void Laser_StartDetect(void)
{
    s_detect_start = HAL_GetTick();
    s_block_time = 0;
}

uint16_t Laser_GetBlockTime_ms(void)
{
    if (BSP_Laser1_IsBlocked()) {
        s_block_time += 10;
    }
    return s_block_time;
}
