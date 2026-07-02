/**
 * @file    board_config.h
 * @brief   货道控制板 30×30 — 板级配置
 * @version V1.0
 * @date    2026-07-02
 */

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

/* ======================== 矩阵参数 ======================== */
#define BOARD_ROWS              30      /* 行数（P端595驱动） */
#define BOARD_COLS              30      /* 列数（N端595驱动） */
#define BOARD_CHANNELS          900     /* 总通道数 = ROWS × COLS */

/* ======================== 595 引脚定义 ======================== */
/* P端（行驱动）控制引脚 */
#define P595_DS_PORT            GPIOB
#define P595_DS_PIN             GPIO_PIN_15
#define P595_SHCP_PORT          GPIOB
#define P595_SHCP_PIN           GPIO_PIN_14
#define P595_STCP_PORT          GPIOB
#define P595_STCP_PIN           GPIO_PIN_11
#define P595_MR_PORT            GPIOB
#define P595_MR_PIN             GPIO_PIN_10

/* N端（列驱动）控制引脚 */
#define N595_DS_PORT            GPIOA
#define N595_DS_PIN             GPIO_PIN_4
#define N595_SHCP_PORT          GPIOA
#define N595_SHCP_PIN           GPIO_PIN_5
#define N595_STCP_PORT          GPIOA
#define N595_STCP_PIN           GPIO_PIN_6
#define N595_MR_PORT            GPIOA
#define N595_MR_PIN             GPIO_PIN_7

/* ======================== ADC / 电流检测 ======================== */
#define ADC_CURRENT_CH          0       /* PA0 — 电流检测 */
#define ADC_TEMP_CH             1       /* PA1 — NTC温度 */
#define ADC_SHUNT_RESISTOR_MOHM 20      /* 采样电阻 20mΩ */
#define ADC_GAIN                21      /* 运放增益 1+20k/1k */
#define ADC_REF_MV              3300    /* ADC参考电压 3.3V */
#define ADC_RESOLUTION          4096    /* 12bit */

/* 电流阈值 (mA) */
#define CURRENT_OVER_MA         3250    /* 过流阈值 */
#define CURRENT_OVERLIMIT_MA    4000    /* 硬件极限保护 */
#define CURRENT_UNDER_MA        50      /* 欠流阈值 */

/* ======================== 光耦输入 ======================== */
#define DI_ROW0_PORT            GPIOB   /* INPUT_1 = PB13 */
#define DI_ROW0_PIN             GPIO_PIN_13
/* ... 其他引脚见原理图分析文档 */

/* 光幕输入 */
#define LASER1_PORT             GPIOA   /* INPUT_11 = PA12 */
#define LASER1_PIN              GPIO_PIN_12
#define LASER2_PORT             GPIOA   /* INPUT_12 = PA11 */
#define LASER2_PIN              GPIO_PIN_11

/* ======================== 继电器输出 ======================== */
#define NMOS1_PORT              GPIOB   /* PB0 */
#define NMOS1_PIN               GPIO_PIN_0
#define NMOS2_PORT              GPIOB   /* PB1 */
#define NMOS2_PIN               GPIO_PIN_1
#define NMOS3_PORT              GPIOA   /* PA2 */
#define NMOS3_PIN               GPIO_PIN_2
#define NMOS4_PORT              GPIOA   /* PA3 */
#define NMOS4_PIN               GPIO_PIN_3

/* ======================== LED ======================== */
#define LED_PORT                GPIOA   /* PA8 */
#define LED_PIN                 GPIO_PIN_8

/* ======================== RS-485 ======================== */
#define RS485_USART             USART1
#define RS485_BAUD              9600
#define RS485_TX_PORT           GPIOA   /* PA9 */
#define RS485_TX_PIN            GPIO_PIN_9
#define RS485_RX_PORT           GPIOA   /* PA10 */
#define RS485_RX_PIN            GPIO_PIN_10

/* ======================== 通讯参数 ======================== */
#define SLAVE_ADDR_DEFAULT      1       /* 默认从机地址 */
#define SLAVE_ADDR_MIN          1
#define SLAVE_ADDR_MAX          8
#define FRAME_LENGTH             20     /* 固定帧长 */
#define RESPONSE_TIMEOUT_MS     1000    /* 主机等待响应超时 */

/* ======================== 电机参数 ======================== */
#define MOTOR_TIMEOUT_DEFAULT   70      /* 默认超时 7s (×0.1s) */
#define MOTOR_DETECT_DELAY_DEF  15      /* 默认微动延时 1.5s (×0.1s) */
#define MOTOR_LOCK_TIME_DEF     2       /* 默认时间锁 0.2s (×0.1s) */

/* ======================== RAM 分配 ======================== */
#define STACK_SIZE              2048
#define HEAP_SIZE               512
#define FAULT_LOG_ENTRIES       10

#endif /* __BOARD_CONFIG_H */
