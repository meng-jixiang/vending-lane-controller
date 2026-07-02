/**
 * @file    motor.c
 * @brief   Device 层 — 电机控制状态机
 *
 * 状态机流程:
 *   IDLE → PRECHECK → RUNNING → STOPPING → DONE
 *
 * 依赖 bsp 层接口，不直接调用 HAL
 */
#include "motor.h"
#include "bsp_595.h"
#include "bsp_gpio.h"
#include "bsp_adc.h"
#include "main.h"
#include <string.h>

/* ======================== 内部变量 ======================== */

static Motor_Ctx_t s_ctx;

/* ======================== 内部：电流判定 ======================== */

static uint8_t check_overcurrent(uint16_t ma)
{
    return (ma > 3250) ? 1 : 0;
}

static uint8_t check_undercurrent(uint16_t ma)
{
    return (ma < 50) ? 1 : 0;
}

/* ======================== 内部：到位检测 ======================== */

static uint8_t has_arrive_signal(void)
{
    /* 只有 row 0-9 有到位检测 */
    if (s_ctx.row < 10) {
        return BSP_DI_GetRow(s_ctx.row);
    }
    return 0;  /* row 10-29 无到位信号 */
}

/* ======================== 内部：电机启停 ======================== */

static void motor_drive_on(void)
{
    /* 选通行列，启动电机 */
    BSP_595_SelectRow(s_ctx.row);
    BSP_595_SetColumns(1UL << s_ctx.col);
}

static void motor_drive_off(void)
{
    BSP_595_AllOff();
}

/* ======================== 内部：状态机处理 ====================== */

static void state_precheck(void)
{
    /* 光幕模式 1/2：自检 */
    if (s_ctx.laser_mode == LASER_MODE_ARRIVE || s_ctx.laser_mode == LASER_MODE_DROP) {
        if (BSP_Laser1_IsBlocked() || BSP_Laser2_IsBlocked()) {
            /* 自检失败：光幕有遮挡 */
            s_ctx.result = MOTOR_RESULT_LASER_FAIL;
            s_ctx.state = MOTOR_STATE_DONE;
            return;
        }
    }
    /* 自检通过，启动电机 */
    s_ctx.start_tick = HAL_GetTick();
    s_ctx.peak_current = 0;
    s_ctx.avg_current = 0;
    s_ctx.run_time_ms = 0;
    s_ctx.laser_time_ms = 0;
    s_ctx.current_sum = 0;
    s_ctx.current_cnt = 0;
    motor_drive_on();
    s_ctx.state = MOTOR_STATE_RUNNING;
}

static void state_running(void)
{
    uint32_t elapsed = HAL_GetTick() - s_ctx.start_tick;
    s_ctx.run_time_ms = elapsed;

    /* 采样电流 (每 10ms 一次) */
    if (elapsed % 10 == 0) {
        uint16_t cur = BSP_ADC_ReadCurrent_mA();
        s_ctx.current_sum += cur;
        s_ctx.current_cnt++;
        if (cur > s_ctx.peak_current) s_ctx.peak_current = cur;

        /* 过流保护 */
        if (check_overcurrent(cur)) {
            s_ctx.result = MOTOR_RESULT_OVERCURRENT;
            s_ctx.state = MOTOR_STATE_STOPPING;
            return;
        }
    }

    /* 超时判定 */
    uint32_t timeout_ms = (s_ctx.timeout == 0) ? 7000 : (uint32_t)s_ctx.timeout * 100;
    if (elapsed >= timeout_ms) {
        s_ctx.result = MOTOR_RESULT_TIMEOUT;
        s_ctx.state = MOTOR_STATE_STOPPING;
        return;
    }

    /* 到位检测 (类型 0x02/0x03/0x05 等) */
    if (s_ctx.type == MOTOR_TYPE_2WIRE || s_ctx.type == MOTOR_TYPE_3WIRE ||
        s_ctx.type == MOTOR_TYPE_2WIRE_COMMON) {
        if (s_ctx.detect_delay > 0 && elapsed >= (uint32_t)s_ctx.detect_delay * 100) {
            if (has_arrive_signal()) {
                s_ctx.result = MOTOR_RESULT_OK;
                s_ctx.state = MOTOR_STATE_STOPPING;
                return;
            }
        }
    }

    /* 光幕模式 2：掉货检测 */
    if (s_ctx.laser_mode == LASER_MODE_DROP) {
        if (BSP_Laser1_IsBlocked() || BSP_Laser2_IsBlocked()) {
            s_ctx.laser_time_ms += 10;  /* 累计遮挡时间 */
            s_ctx.result = MOTOR_RESULT_OK;  /* 检测到掉货 = 成功 */
            s_ctx.state = MOTOR_STATE_STOPPING;
            return;
        }
    }

    /* 光幕模式 1：到位后检测到掉货停止 */
    if (s_ctx.laser_mode == LASER_MODE_ARRIVE) {
        if (BSP_Laser1_IsBlocked()) {
            s_ctx.laser_time_ms += 10;
        }
    }

    /* 无反馈类型 (0x00/0x04/0x08/0x0D)：固定时间后停止 */
    if (s_ctx.type == MOTOR_TYPE_SOLENOID_NOFB || s_ctx.type == MOTOR_TYPE_ELOCK_12S ||
        s_ctx.type == MOTOR_TYPE_ELOCK_TIME || s_ctx.type == MOTOR_TYPE_SOLENOID_COM) {
        /* 超时自动停止 (在上面的超时判定中已处理) */
    }
}

static void state_stopping(void)
{
    motor_drive_off();

    /* 计算平均电流 */
    if (s_ctx.current_cnt > 0) {
        s_ctx.avg_current = (uint16_t)(s_ctx.current_sum / s_ctx.current_cnt);
    }

    /* 欠流判定 (运行时间足够长才判断) */
    if (s_ctx.result == MOTOR_RESULT_OK && s_ctx.run_time_ms > 500) {
        if (check_undercurrent(s_ctx.avg_current)) {
            s_ctx.result = MOTOR_RESULT_UNDERCURRENT;
        }
    }

    s_ctx.state = MOTOR_STATE_DONE;
}

/* ======================== 公开 API ======================== */

void Motor_Init(void)
{
    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.state = MOTOR_STATE_IDLE;
}

uint8_t Motor_Start(uint8_t row, uint8_t col, uint8_t type,
                    uint8_t laser_mode, uint8_t detect_delay,
                    uint8_t timeout, uint8_t lock_time)
{
    if (row >= BOARD_ROWS || col >= BOARD_COLS) return 1;
    if (s_ctx.state != MOTOR_STATE_IDLE && s_ctx.state != MOTOR_STATE_DONE) return 2;
    if (s_ctx.state == MOTOR_STATE_DONE) return 3;

    s_ctx.row          = row;
    s_ctx.col          = col;
    s_ctx.type         = type;
    s_ctx.laser_mode   = laser_mode;
    s_ctx.detect_delay = detect_delay;
    s_ctx.timeout      = timeout;
    s_ctx.lock_time    = lock_time;
    s_ctx.result       = MOTOR_RESULT_OK;

    if (s_ctx.laser_mode != LASER_MODE_NONE) {
        s_ctx.state = MOTOR_STATE_PRECHECK;
    } else {
        s_ctx.start_tick = HAL_GetTick();
        s_ctx.peak_current = 0;
        s_ctx.avg_current = 0;
        s_ctx.run_time_ms = 0;
        s_ctx.current_sum = 0;
        s_ctx.current_cnt = 0;
        motor_drive_on();
        s_ctx.state = MOTOR_STATE_RUNNING;
    }
    return 0;
}

void Motor_GetStatus(uint8_t *state, uint16_t *channel,
                     uint8_t *result, uint16_t *peak_cur,
                     uint16_t *avg_cur, uint16_t *run_time,
                     uint8_t *laser_cnt)
{
    *state     = (uint8_t)s_ctx.state;
    *channel   = (uint16_t)(s_ctx.row * BOARD_COLS + s_ctx.col);
    *result    = s_ctx.result;
    *peak_cur  = s_ctx.peak_current;
    *avg_cur   = s_ctx.avg_current;
    *run_time  = (uint16_t)s_ctx.run_time_ms;
    *laser_cnt = s_ctx.laser_time_ms;
}

void Motor_Process(void)
{
    switch (s_ctx.state) {
    case MOTOR_STATE_PRECHECK: state_precheck(); break;
    case MOTOR_STATE_RUNNING:  state_running();  break;
    case MOTOR_STATE_STOPPING: state_stopping();  break;
    default: break;
    }
}

void Motor_ClearResult(void)
{
    if (s_ctx.state == MOTOR_STATE_DONE) {
        memset(&s_ctx, 0, sizeof(s_ctx));
        s_ctx.state = MOTOR_STATE_IDLE;
    }
}

uint8_t Motor_IsBusy(void)
{
    return (s_ctx.state == MOTOR_STATE_RUNNING || s_ctx.state == MOTOR_STATE_PRECHECK) ? 1 : 0;
}
