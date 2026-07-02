/**
 * @file    fault_log.c
 * @brief   Service 层 — 故障日志 (环形缓冲)
 */
#include "fault_log.h"
#include "main.h"
#include <string.h>

static Fault_Entry_t s_log[FAULT_LOG_MAX];
static uint8_t s_count = 0;
static uint8_t s_write_idx = 0;

void FaultLog_Init(void)
{
    memset(s_log, 0, sizeof(s_log));
    s_count = 0;
    s_write_idx = 0;
}

void FaultLog_Add(uint8_t code, uint16_t channel, uint16_t current_mA)
{
    s_log[s_write_idx].timestamp   = HAL_GetTick();
    s_log[s_write_idx].fault_code  = code;
    s_log[s_write_idx].channel     = channel;
    s_log[s_write_idx].current_mA  = current_mA;
    s_write_idx = (s_write_idx + 1) % FAULT_LOG_MAX;
    if (s_count < FAULT_LOG_MAX) s_count++;
}

uint8_t FaultLog_GetCount(void)
{
    return s_count;
}

const Fault_Entry_t *FaultLog_GetEntry(uint8_t index)
{
    if (index >= s_count) return NULL;
    uint8_t real_idx = (s_write_idx + FAULT_LOG_MAX - s_count + index) % FAULT_LOG_MAX;
    return &s_log[real_idx];
}

void FaultLog_Clear(void)
{
    memset(s_log, 0, sizeof(s_log));
    s_count = 0;
    s_write_idx = 0;
}
