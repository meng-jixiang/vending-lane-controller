/**
 * @file    fault_log.h
 * @brief   Service 层 — 故障日志
 */
#ifndef __FAULT_LOG_H
#define __FAULT_LOG_H

#include <stdint.h>

#define FAULT_LOG_MAX  10

typedef struct {
    uint32_t timestamp;    /* HAL_GetTick() */
    uint8_t  fault_code;   /* 故障码 */
    uint16_t channel;      /* 通道号 */
    uint16_t current_mA;   /* 电流值 */
} Fault_Entry_t;

void    FaultLog_Init(void);
void    FaultLog_Add(uint8_t code, uint16_t channel, uint16_t current_mA);
uint8_t FaultLog_GetCount(void);
const Fault_Entry_t *FaultLog_GetEntry(uint8_t index);
void    FaultLog_Clear(void);

#endif
