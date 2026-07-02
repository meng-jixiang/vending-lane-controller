/**
 * @file    motor.h
 * @brief   Device 层 — 电机控制状态机
 *
 * 兼容 M109E 的 14 种电机类型，支持到位检测、电流保护、光幕联动。
 * 依赖: bsp_595, bsp_gpio, bsp_adc (不直接调用 HAL)
 */
#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>

/* ======================== 电机类型 (兼容 M109E) ======================== */
#define MOTOR_TYPE_SOLENOID_NOFB   0x00   /* 无反馈电磁铁 */
#define MOTOR_TYPE_SOLENOID_FB     0x01   /* 两线有反馈电磁铁 */
#define MOTOR_TYPE_2WIRE           0x02   /* 两线制电机 */
#define MOTOR_TYPE_3WIRE           0x03   /* 三线制电机 */
#define MOTOR_TYPE_ELOCK_12S       0x04   /* 12秒电插锁 */
#define MOTOR_TYPE_2WIRE_COMMON    0x05   /* 共正型两线电机 */
#define MOTOR_TYPE_BELT_H_L_H      0x06   /* 皮带货道 H→L→H */
#define MOTOR_TYPE_BELT_L_H_L      0x07   /* 皮带货道 L→H→L */
#define MOTOR_TYPE_ELOCK_TIME      0x08   /* 时间控制型电磁锁 */
#define MOTOR_TYPE_ELOCK_FB        0x09   /* 三线有反馈电磁锁 */
#define MOTOR_TYPE_2WIRE_REV       0x0A   /* 反向两线 */
#define MOTOR_TYPE_3WIRE_REV       0x0B   /* 反向三线 */
#define MOTOR_TYPE_SOLENOID_REV    0x0C   /* 反向无反馈电磁铁 */
#define MOTOR_TYPE_SOLENOID_COM    0x0D   /* 共正型无反馈电磁锁 */
#define MOTOR_TYPE_BELT_V2         0x0E   /* 皮带货道 V2 */

/* ======================== 光幕模式 ======================== */
#define LASER_MODE_NONE            0      /* 不参考光幕 */
#define LASER_MODE_ARRIVE          1      /* 自检+到位停 */
#define LASER_MODE_DROP            2      /* 自检+掉货停 */

/* ======================== 状态机状态 ======================== */
typedef enum {
    MOTOR_STATE_IDLE,       /* 空闲 */
    MOTOR_STATE_PRECHECK,   /* 光幕自检 */
    MOTOR_STATE_RUNNING,    /* 电机运行中 */
    MOTOR_STATE_STOPPING,   /* 停止中 */
    MOTOR_STATE_DONE        /* 执行完毕，等待上位机读取 */
} Motor_State_t;

/* ======================== 执行结果 ======================== */
#define MOTOR_RESULT_OK            0x00   /* 成功 */
#define MOTOR_RESULT_OVERCURRENT   0x01   /* 过流 */
#define MOTOR_RESULT_UNDERCURRENT  0x02   /* 欠流 */
#define MOTOR_RESULT_TIMEOUT       0x03   /* 超时 */
#define MOTOR_RESULT_LASER_FAIL    0x04   /* 光幕自检失败 */
#define MOTOR_RESULT_DOOR_OPEN     0x05   /* 门未开 */
#define MOTOR_RESULT_SWITCH_FAIL   0x0A   /* 微动开关未按下 */

/* ======================== 运行上下文 ======================== */
typedef struct {
    Motor_State_t state;
    uint8_t       type;           /* 电机类型 */
    uint8_t       row;            /* 行号 0-29 */
    uint8_t       col;            /* 列号 0-29 */
    uint8_t       laser_mode;     /* 光幕模式 */
    uint8_t       detect_delay;   /* 微动检测延时 (×0.1s) */
    uint8_t       timeout;        /* 到位超时 (×0.1s) */
    uint8_t       lock_time;      /* 时间锁动作时间 (×0.1s) */

    uint8_t       result;         /* 执行结果 */
    uint16_t      peak_current;   /* 峰值电流 (mA) */
    uint16_t      avg_current;    /* 平均电流 (mA) */
    uint32_t      run_time_ms;    /* 运行时间 (ms) */
    uint8_t       laser_time_ms;  /* 光幕遮挡时间 (ms) */

    uint32_t      start_tick;     /* 启动时刻 */
    uint32_t      current_sum;    /* 电流累加 (用于求平均) */
    uint32_t      current_cnt;    /* 采样次数 */
} Motor_Ctx_t;

/* ======================== API ======================== */

/**
 * @brief 电机模块初始化
 */
void Motor_Init(void);

/**
 * @brief 启动指定通道的电机
 * @return 0=已启动, 1=无效通道, 2=其他电机运行中, 3=上次结果未清除
 */
uint8_t Motor_Start(uint8_t row, uint8_t col, uint8_t type,
                    uint8_t laser_mode, uint8_t detect_delay,
                    uint8_t timeout, uint8_t lock_time);

/**
 * @brief 查询电机状态
 */
void Motor_GetStatus(uint8_t *state, uint16_t *channel,
                     uint8_t *result, uint16_t *peak_cur,
                     uint16_t *avg_cur, uint16_t *run_time,
                     uint8_t *laser_cnt);

/**
 * @brief 主循环调用，驱动状态机 (每 1ms 调用)
 */
void Motor_Process(void);

/**
 * @brief 清除上次执行结果 (上位机读取后调用)
 */
void Motor_ClearResult(void);

/**
 * @brief 当前是否有电机在运行
 */
uint8_t Motor_IsBusy(void);

#endif /* __MOTOR_H */
