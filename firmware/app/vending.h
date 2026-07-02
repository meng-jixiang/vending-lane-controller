/**
 * @file    vending.h
 * @brief   App 层 — 出货主流程
 *
 * 串联 protocol → motor → laser → fault_log，实现完整的出货业务逻辑。
 */
#ifndef __VENDING_H
#define __VENDING_H

#include <stdint.h>

/**
 * @brief 出货模块初始化
 */
void Vending_Init(void);

/**
 * @brief 主循环调用
 * @note  内部调用 Motor_Process() + 状态上报
 */
void Vending_Process(void);

/**
 * @brief 获取系统运行时间 (秒)
 */
uint32_t Vending_GetUptime_s(void);

#endif
