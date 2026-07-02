/**
 * @file    protocol.c
 * @brief   Service 层 — M109E 协议解析
 *
 * 依赖: bsp_uart (收发), bsp_led (指示), param_store (地址存储)
 * 不直接操作任何硬件
 */
#include "protocol.h"
#include "param_store.h"
#include "bsp_uart.h"
#include "bsp_led.h"
#include "board_config.h"
#include <string.h>

/* ======================== 内部变量 ======================== */

static uint8_t  s_rx_buf[PROTO_FRAME_LEN];
static uint8_t  s_rx_idx = 0;
static uint8_t  s_frame_ready = 0;
static uint8_t  s_tx_buf[PROTO_FRAME_LEN];

/* ======================== CRC16-MODBUS ======================== */

uint16_t Proto_Crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
        }
    }
    return crc;
}

/* ======================== 帧收发 ======================== */

static void proto_send(uint8_t cmd, const uint8_t *data, uint8_t len)
{
    memset(s_tx_buf, 0, PROTO_FRAME_LEN);
    s_tx_buf[0] = 0x00;
    s_tx_buf[1] = cmd;
    if (data && len > 0) {
        memcpy(&s_tx_buf[2], data, (len > PROTO_DATA_LEN) ? PROTO_DATA_LEN : len);
    }
    uint16_t crc = Proto_Crc16(s_tx_buf, PROTO_FRAME_LEN - 2);
    s_tx_buf[18] = crc & 0xFF;
    s_tx_buf[19] = (crc >> 8) & 0xFF;
    BSP_RS485_Send(s_tx_buf, PROTO_FRAME_LEN);
}

static uint8_t addr_match(uint8_t frame_addr)
{
    return (frame_addr == PROTO_BROADCAST_ADDR || frame_addr == Param_GetAddr());
}

/* ======================== 指令处理 ======================== */

static void handle_set_addr(const uint8_t *data)
{
    uint8_t new_addr = data[0];
    if (new_addr >= SLAVE_ADDR_MIN && new_addr <= SLAVE_ADDR_MAX) {
        if (Param_SaveAddr(new_addr) == 0) {
            BSP_LED_Blink(new_addr, 200);
        }
    }
}

static void handle_get_id(void)
{
    uint8_t resp[PROTO_DATA_LEN] = {0};
    resp[1] = 0x64;
    proto_send(CMD_GET_ID, resp, PROTO_DATA_LEN);
}

static void handle_motor_poll(void)
{
    uint8_t resp[PROTO_DATA_LEN] = {0};
    proto_send(CMD_MOTOR_POLL, resp, PROTO_DATA_LEN);
}

static void handle_read_temp(void)
{
    uint8_t resp[PROTO_DATA_LEN] = {0};
    resp[0] = 0xFE; resp[1] = 0x0C;  /* -50.0℃ */
    proto_send(CMD_READ_TEMP, resp, PROTO_DATA_LEN);
}

static void dispatch(uint8_t cmd, const uint8_t *data)
{
    switch (cmd) {
    case CMD_GET_ID:      handle_get_id();      break;
    case CMD_MOTOR_POLL:  handle_motor_poll();  break;
    case CMD_READ_TEMP:   handle_read_temp();   break;
    default:              proto_send(cmd, NULL, 0); break;
    }
}

static void parse_frame(void)
{
    uint8_t addr = s_rx_buf[0];
    uint8_t cmd  = s_rx_buf[1];

    if (Proto_Crc16(s_rx_buf, PROTO_FRAME_LEN - 2) != (s_rx_buf[18] | (s_rx_buf[19] << 8)))
        return;

    if (cmd == CMD_SET_ADDR && addr == PROTO_BROADCAST_ADDR) {
        handle_set_addr(&s_rx_buf[2]);
        return;
    }
    if (!addr_match(addr)) return;
    dispatch(cmd, &s_rx_buf[2]);
}

/* ======================== 公开 API ======================== */

void Proto_Init(void)
{
    s_rx_idx = 0;
    s_frame_ready = 0;
    BSP_RS485_RxStart();
}

void Proto_RxByte(uint8_t byte)
{
    if (s_frame_ready) return;
    s_rx_buf[s_rx_idx++] = byte;
    if (s_rx_idx >= PROTO_FRAME_LEN) s_frame_ready = 1;
}

void Proto_Process(void)
{
    if (!s_frame_ready) return;
    parse_frame();
    s_rx_idx = 0;
    s_frame_ready = 0;
    BSP_RS485_RxStart();
}

uint8_t Proto_IsFrameReady(void)
{
    return s_frame_ready;
}
