/**
 * @file    protocol.c
 * @brief   M109E 协议解析 — 帧收发, CRC16-MODBUS
 * @version V1.0
 * @date    2026-07-02
 *
 * 当前实现:
 *   - CRC16-MODBUS 校验
 *   - 20 字节定长帧接收 (USART3 中断驱动)
 *   - 0xFF 指令: 设置从机地址 (广播, 掉电记忆)
 *
 * 待实现:
 *   - 0x01~0x2B 其他指令的响应
 *   - USART3 DMA 发送
 */

#include "protocol.h"
#include "param_store.h"
#include "board_config.h"
#include <string.h>

/* ======================== 外部句柄 ======================== */
extern UART_HandleTypeDef huart3;

/* ======================== 内部变量 ======================== */

static uint8_t  s_rx_buf[PROTO_FRAME_LEN];     /* 接收缓冲 */
static uint8_t  s_rx_idx = 0;                   /* 当前接收位置 */
static uint8_t  s_frame_ready = 0;              /* 帧就绪标志 */
static uint8_t  s_tx_buf[PROTO_FRAME_LEN];     /* 发送缓冲 */

/* ======================== CRC16-MODBUS ======================== */

uint16_t Proto_Crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;  /* 低字节在前 */
}

/* ======================== LED 闪烁 ======================== */

void Proto_LedBlink(uint8_t times, uint16_t interval_ms)
{
    for (uint8_t i = 0; i < times; i++) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        HAL_Delay(interval_ms);
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        if (i < times - 1) {
            HAL_Delay(interval_ms);
        }
    }
}

/* ======================== 帧收发 ======================== */

/**
 * @brief 发送 20 字节响应帧
 */
static void proto_send_response(uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
    memset(s_tx_buf, 0, PROTO_FRAME_LEN);
    s_tx_buf[0] = 0x00;     /* 主机地址固定 0 */
    s_tx_buf[1] = cmd;

    if (data != NULL && data_len > 0) {
        uint8_t copy_len = (data_len > PROTO_DATA_LEN) ? PROTO_DATA_LEN : data_len;
        memcpy(&s_tx_buf[2], data, copy_len);
    }

    uint16_t crc = Proto_Crc16(s_tx_buf, PROTO_FRAME_LEN - 2);
    s_tx_buf[18] = crc & 0xFF;         /* CRC 低字节 */
    s_tx_buf[19] = (crc >> 8) & 0xFF;  /* CRC 高字节 */

    HAL_UART_Transmit(&huart3, s_tx_buf, PROTO_FRAME_LEN, 100);
}

/**
 * @brief 检查地址是否匹配（本机地址或广播地址）
 */
static uint8_t proto_addr_match(uint8_t frame_addr)
{
    if (frame_addr == PROTO_BROADCAST_ADDR) return 1;
    if (frame_addr == Param_GetAddr()) return 1;
    return 0;
}

/* ======================== 指令处理 ======================== */

/**
 * @brief 处理 0xFF — 设置从机地址
 *
 * 协议: [FF][目标地址 0x01-0x08][00×16][CRC]
 * 广播指令，无响应帧。
 * 设置成功后 LED 闪烁 N 次确认。
 */
static void handle_set_addr(const uint8_t *data)
{
    uint8_t new_addr = data[0];

    if (new_addr < SLAVE_ADDR_MIN || new_addr > SLAVE_ADDR_MAX) {
        return;  /* 地址无效，忽略 */
    }

    uint8_t result = Param_SaveAddr(new_addr);
    if (result == 0) {
        /* 设置成功，LED 闪烁新地址次数确认 */
        Proto_LedBlink(new_addr, 200);
    }
}

/**
 * @brief 处理 0x01 — 获取序列号
 * 响应: [00][01][Z1-Z12=序列号][00×4][CRC]
 */
static void handle_get_id(void)
{
    uint8_t resp[PROTO_DATA_LEN];
    memset(resp, 0, PROTO_DATA_LEN);

    /* TODO: 从 Flash 或 UID 读取真实序列号，暂用默认 */
    resp[0] = 0x00;
    resp[1] = 0x64;
    /* resp[2..11] = 序列号 */

    proto_send_response(CMD_GET_ID, resp, PROTO_DATA_LEN);
}

/**
 * @brief 处理 0x03 — 查询电机状态
 * 暂返回空闲状态
 */
static void handle_motor_poll(void)
{
    uint8_t resp[PROTO_DATA_LEN];
    memset(resp, 0, PROTO_DATA_LEN);
    resp[0] = 0x00;  /* Z1: 空闲 */
    proto_send_response(CMD_MOTOR_POLL, resp, PROTO_DATA_LEN);
}

/**
 * @brief 处理 0x07 — 读温度
 * 暂返回 -50.0℃（未接传感器）
 */
static void handle_read_temp(void)
{
    uint8_t resp[PROTO_DATA_LEN];
    memset(resp, 0, PROTO_DATA_LEN);
    resp[0] = 0xFE;  /* Z1: -50.0℃ 高字节 */
    resp[1] = 0x0C;  /* Z2: -50.0℃ 低字节 */
    proto_send_response(CMD_READ_TEMP, resp, PROTO_DATA_LEN);
}

/**
 * @brief 通用未实现指令 — 返回全零
 */
static void handle_unsupported(uint8_t cmd)
{
    uint8_t resp[PROTO_DATA_LEN];
    memset(resp, 0, PROTO_DATA_LEN);
    proto_send_response(cmd, resp, PROTO_DATA_LEN);
}

/* ======================== 帧解析与分发 ======================== */

/**
 * @brief 解析并处理一帧完整数据
 */
static void proto_parse_frame(void)
{
    uint8_t addr = s_rx_buf[0];
    uint8_t cmd  = s_rx_buf[1];
    uint8_t *data = &s_rx_buf[2];

    /* CRC 校验 */
    uint16_t crc_calc = Proto_Crc16(s_rx_buf, PROTO_FRAME_LEN - 2);
    uint16_t crc_recv = s_rx_buf[18] | (s_rx_buf[19] << 8);
    if (crc_calc != crc_recv) {
        return;  /* CRC 错误，丢弃 */
    }

    /* 广播指令（不需要地址匹配） */
    if (cmd == CMD_SET_ADDR && addr == PROTO_BROADCAST_ADDR) {
        handle_set_addr(data);
        return;
    }

    /* 非广播指令：检查地址匹配 */
    if (!proto_addr_match(addr)) {
        return;  /* 非本机地址，忽略 */
    }

    /* 指令分发 */
    switch (cmd) {
    case CMD_GET_ID:
        handle_get_id();
        break;
    case CMD_MOTOR_POLL:
        handle_motor_poll();
        break;
    case CMD_READ_TEMP:
        handle_read_temp();
        break;
    case CMD_SET_ADDR:
        /* 非广播的地址设置也处理 */
        handle_set_addr(data);
        break;
    default:
        handle_unsupported(cmd);
        break;
    }
}

/* ======================== 公开 API ======================== */

void Proto_Init(void)
{
    s_rx_idx = 0;
    s_frame_ready = 0;

    /* 启用 USART3 接收中断（1 字节中断模式） */
    HAL_UART_Receive_IT(&huart3, &s_rx_buf[0], 1);
}

void Proto_RxByte(uint8_t byte)
{
    /* 简单的状态机：连续接收 20 字节后标记帧就绪 */
    if (s_frame_ready) return;  /* 上一帧还没处理完 */

    s_rx_buf[s_rx_idx] = byte;
    s_rx_idx++;

    if (s_rx_idx >= PROTO_FRAME_LEN) {
        s_frame_ready = 1;
    }
}

void Proto_Process(void)
{
    if (!s_frame_ready) return;

    proto_parse_frame();

    /* 重置接收 */
    s_rx_idx = 0;
    s_frame_ready = 0;

    /* 重新启动接收 */
    HAL_UART_Receive_IT(&huart3, &s_rx_buf[0], 1);
}

uint8_t Proto_IsFrameReady(void)
{
    return s_frame_ready;
}
