/**
 * @file    param_store.h
 * @brief   参数存储 — STM32 Flash 模拟 EEPROM
 * @version V1.0
 * @date    2026-07-02
 *
 * 使用 Flash 最后 2 页做双页轮转存储，支持掉电保存。
 * 每页 1KB (512 个半字)，每个参数占 1 个半字。
 */

#ifndef __PARAM_STORE_H
#define __PARAM_STORE_H

#include "main.h"
#include <stdint.h>

/* ======================== Flash 地址定义 ======================== */
/* STM32F103C8T6: 64KB Flash, 每页 1KB */
/* FLASH_PAGE_SIZE 已在 HAL 中定义 (0x400U) */
#define FLASH_END_ADDR          0x08010000      /* Flash 末尾 (64KB) */
#define PARAM_PAGE0_ADDR        (FLASH_END_ADDR - 2 * FLASH_PAGE_SIZE)  /* 0x0800F800 */
#define PARAM_PAGE1_ADDR        (FLASH_END_ADDR - 1 * FLASH_PAGE_SIZE)  /* 0x0800FC00 */

/* ======================== 参数存储格式 ======================== */
/*
 * 每个参数存为 16-bit 半字: [有效标记 0xA5xx | 数据]
 * 高字节 = 0xA5 表示有效, 低字节 = 参数值
 * Flash 擦除后 = 0xFFFF (无效)
 */
#define PARAM_VALID_MARKER      0xA500
#define PARAM_INVALID           0xFFFF

/* 参数 ID (高 4 位索引, 最多 16 种参数) */
#define PARAM_ID_ADDR           0x00    /* 从机地址 */
/* 未来可扩展: */
/* #define PARAM_ID_MOTOR_TYPE  0x01 */

/* ======================== API ======================== */

/**
 * @brief 参数存储模块初始化（启动时调用）
 * @note  自动从 Flash 加载有效参数，无有效数据则使用默认值
 */
void Param_Init(void);

/**
 * @brief 保存从机地址到 Flash
 * @param addr 从机地址 (1-8)
 * @return 0=成功, 1=参数非法, 2=Flash写入失败
 */
uint8_t Param_SaveAddr(uint8_t addr);

/**
 * @brief 读取当前从机地址
 * @return 从机地址 (1-8)
 */
uint8_t Param_GetAddr(void);

/**
 * @brief 恢复默认参数（擦除所有参数页）
 */
void Param_ResetDefaults(void);

#endif /* __PARAM_STORE_H */
