/**
 * @file    protocol.h
 * @brief   M109E 协议解析 — 帧收发, CRC16-MODBUS
 * @version V1.0
 * @date    2026-07-02
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "main.h"
#include <stdint.h>

/* ======================== 帧格式 ======================== */
#define PROTO_FRAME_LEN         20      /* 固定帧长 */
#define PROTO_DATA_LEN          16      /* 数据段长度 */
#define PROTO_BROADCAST_ADDR    0xFF    /* 广播地址 */

/* ======================== 指令码 ======================== */
#define CMD_GET_ID              0x01    /* 获取序列号 */
#define CMD_MOTOR_POLL          0x03    /* 查询电机状态 */
#define CMD_MOTOR_RUN           0x05    /* 启动电机 */
#define CMD_READ_TEMP           0x07    /* 读温度 */
#define CMD_WRITE_DO            0x08    /* 开关量输出 */
#define CMD_READ_DI             0x09    /* 读开关量输入 */
#define CMD_LASER_ONOFF         0x0B    /* 开停光幕 */
#define CMD_READ_LASER          0x0C    /* 读光幕状态 */
#define CMD_READ_LASER_CNT      0x0D    /* 读光幕遮挡计时 */
#define CMD_READ_RH_TEMP        0x10    /* 读温湿度 */
#define CMD_READ_SWITCH         0x2A    /* 读单通道到位 */
#define CMD_READ_SWITCH_LAYER   0x2B    /* 读一行到位 */
#define CMD_SET_ADDR            0xFF    /* 设置从机地址 */

/* ======================== 接收状态 ======================== */
typedef enum {
    PROTO_IDLE,         /* 等待接收 */
    PROTO_RECEIVING,    /* 正在接收 */
    PROTO_FRAME_READY   /* 一帧接收完成 */
} Proto_RxState_t;

/* ======================== API ======================== */

/**
 * @brief 协议模块初始化
 * @note  启用 USART3 接收中断
 */
void Proto_Init(void);

/**
 * @brief 从 USART3 DMA/中断中喂入一个字节
 * @param byte 接收到的字节
 * @note  在 USART3 接收中断回调中调用
 */
void Proto_RxByte(uint8_t byte);

/**
 * @brief 主循环调用，处理收到的完整帧
 * @note  检测到 FrameReady 后自动解析+分发+响应
 */
void Proto_Process(void);

/**
 * @brief 获取当前帧是否就绪（调试用）
 */
uint8_t Proto_IsFrameReady(void);

/**
 * @brief CRC16-MODBUS 计算
 * @param data   数据指针
 * @param length 数据长度
 * @return CRC16 值（低字节在前的 2 字节）
 */
uint16_t Proto_Crc16(const uint8_t *data, uint16_t length);

/**
 * @brief LED 闪烁 N 次（用于地址设置确认）
 * @param times 闪烁次数
 * @param interval_ms 每次间隔(ms)
 */
void Proto_LedBlink(uint8_t times, uint16_t interval_ms);

#endif /* __PROTOCOL_H */
