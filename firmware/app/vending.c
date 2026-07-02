/**
 * @file    vending.c
 * @brief   App 层 — 出货主流程
 */
#include "vending.h"
#include "motor.h"
#include "laser.h"
#include "fault_log.h"
#include "bsp_led.h"
#include "main.h"

void Vending_Init(void)
{
    Motor_Init();
    Laser_Init();
    FaultLog_Init();
}

void Vending_Process(void)
{
    Motor_Process();

    /* 故障自动记录 */
    uint8_t  state, result, laser_cnt;
    uint16_t channel, peak, avg, run_time;
    Motor_GetStatus(&state, &channel, &result, &peak, &avg, &run_time, &laser_cnt);

    if (state == 2 && result != 0) {  /* DONE + 非成功 */
        FaultLog_Add(result, channel, peak);
    }
}

uint32_t Vending_GetUptime_s(void)
{
    return HAL_GetTick() / 1000;
}
