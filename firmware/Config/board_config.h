/**
 * @file    board_config.h
 * @brief   货道控制板 30×30 — 板级配置
 * @version V2.0
 * @date    2026-07-02
 * @note    基于实际 PCB 引脚分配（lane_board.ioc）
 */

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

/* ======================== 矩阵参数 ======================== */
#define BOARD_ROWS              30      /* 行数（P端595驱动） */
#define BOARD_COLS              30      /* 列数（N端595驱动） */
#define BOARD_CHANNELS          900     /* 总通道数 = ROWS × COLS */

/* ======================== N端595引脚（列驱动/负极） ======================== */
/* PA4=DS, PA5=SHCP, PA6=STCP, PA7=MR */
#define N595_DS_PORT            GPIOA
#define N595_DS_PIN             GPIO_PIN_4
#define N595_SHCP_PORT          GPIOA
#define N595_SHCP_PIN           GPIO_PIN_5
#define N595_STCP_PORT          GPIOA
#define N595_STCP_PIN           GPIO_PIN_6
#define N595_MR_PORT            GPIOA
#define N595_MR_PIN             GPIO_PIN_7

/* ======================== P端595引脚（行驱动/正极） ======================== */
/* PB12=DS, PB13=SHCP, PB14=STCP, PB15=MR */
#define P595_DS_PORT            GPIOB
#define P595_DS_PIN             GPIO_PIN_12
#define P595_SHCP_PORT          GPIOB
#define P595_SHCP_PIN           GPIO_PIN_13
#define P595_STCP_PORT          GPIOB
#define P595_STCP_PIN           GPIO_PIN_14
#define P595_MR_PORT            GPIOB
#define P595_MR_PIN             GPIO_PIN_15

/* ======================== ADC / 电流检测 ======================== */
#define ADC_CURRENT_CH          0       /* PA0 — ADC1_CH0 电流检测 */
#define ADC_TEMP_CH             1       /* PA1 — ADC2_CH1 NTC温度 */
#define ADC_SHUNT_RESISTOR_MOHM 20      /* 采样电阻 20mΩ */
#define ADC_GAIN                21      /* 运放增益 1+20k/1k */
#define ADC_REF_MV              3300    /* ADC参考电压 3.3V */
#define ADC_RESOLUTION          4096    /* 12bit */

/* 电流阈值 (mA) */
#define CURRENT_OVER_MA         3250    /* 过流阈值 */
#define CURRENT_OVERLIMIT_MA    4000    /* 硬件极限保护 */
#define CURRENT_UNDER_MA        50      /* 欠流阈值 */

/* ======================== 到位检测输入（10路） ======================== */
/* PC13, PC14, PC15, PB3~PB9 — 货道到位信号，低电平=到位 */
#define DI_ROW0_PORT            GPIOC   /* PC13 */
#define DI_ROW0_PIN             GPIO_PIN_13
#define DI_ROW1_PORT            GPIOC   /* PC14 */
#define DI_ROW1_PIN             GPIO_PIN_14
#define DI_ROW2_PORT            GPIOC   /* PC15 */
#define DI_ROW2_PIN             GPIO_PIN_15
#define DI_ROW3_PORT            GPIOB   /* PB3 */
#define DI_ROW3_PIN             GPIO_PIN_3
#define DI_ROW4_PORT            GPIOB   /* PB4 */
#define DI_ROW4_PIN             GPIO_PIN_4
#define DI_ROW5_PORT            GPIOB   /* PB5 */
#define DI_ROW5_PIN             GPIO_PIN_5
#define DI_ROW6_PORT            GPIOB   /* PB6 */
#define DI_ROW6_PIN             GPIO_PIN_6
#define DI_ROW7_PORT            GPIOB   /* PB7 */
#define DI_ROW7_PIN             GPIO_PIN_7
#define DI_ROW8_PORT            GPIOB   /* PB8 */
#define DI_ROW8_PIN             GPIO_PIN_8
#define DI_ROW9_PORT            GPIOB   /* PB9 */
#define DI_ROW9_PIN             GPIO_PIN_9

/* ======================== 光幕输入（2路） ======================== */
/* PA12=光幕1, PA15=光幕2 — NPN传感器，低电平=掉货 */
#define LASER1_PORT             GPIOA
#define LASER1_PIN              GPIO_PIN_12
#define LASER2_PORT             GPIOA
#define LASER2_PIN              GPIO_PIN_15

/* ======================== NMOS开关量输出（4路） ======================== */
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

/* ======================== 串口定义 ======================== */
/* USART1 = 调试日志 (PA9/PA10, 115200bps) */
/* USART3 = RS-485通讯 (PB10/PB11, 9600bps, DMA) */
#define DEBUG_USART             USART1
#define DEBUG_USART_BAUD        115200
#define RS485_USART             USART3
#define RS485_BAUD              9600

/* ======================== 通讯参数 ======================== */
#define SLAVE_ADDR_DEFAULT      1       /* 默认从机地址 */
#define SLAVE_ADDR_MIN          1
#define SLAVE_ADDR_MAX          8
#define FRAME_LENGTH            20      /* 固定帧长 */
#define RESPONSE_TIMEOUT_MS     1000    /* 主机等待响应超时 */

/* ======================== 电机参数 ======================== */
#define MOTOR_TIMEOUT_DEFAULT   70      /* 默认超时 7s (×0.1s) */
#define MOTOR_DETECT_DELAY_DEF  15      /* 默认微动延时 1.5s (×0.1s) */
#define MOTOR_LOCK_TIME_DEF     2       /* 默认时间锁 0.2s (×0.1s) */

/* ======================== RAM 分配 ======================== */
#define STACK_SIZE              1024    /* 0x400 */
#define HEAP_SIZE               512     /* 0x200 */
#define FAULT_LOG_ENTRIES       10

#endif /* __BOARD_CONFIG_H */
