/**
 * @file    param_store.c
 * @brief   参数存储 — STM32 Flash 模拟 EEPROM
 * @version V1.0
 * @date    2026-07-02
 *
 * 双页轮转方案：
 *   - 两页交替使用，每页存 512 个半字参数
 *   - 页满时切换到另一页，擦除旧页
 *   - 读取时从当前页尾部向前扫描有效数据
 */

#include "param_store.h"
#include "board_config.h"

/* ======================== 内部变量 ======================== */

static uint8_t  s_slave_addr = SLAVE_ADDR_DEFAULT;  /* 当前从机地址 */
static uint32_t s_current_page = PARAM_PAGE0_ADDR;  /* 当前写入页 */
static uint32_t s_write_offset = 0;                  /* 当前页内写偏移 (半字) */

/* ======================== Flash 底层操作 ======================== */

/**
 * @brief 擦除一页 Flash (1KB)
 */
static uint8_t flash_erase_page(uint32_t page_addr)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase;
    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = page_addr;
    erase.NbPages     = 1;

    uint32_t error = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error);

    HAL_FLASH_Lock();

    return (status == HAL_OK && error == 0xFFFFFFFF) ? 0 : 1;
}

/**
 * @brief 写入一个半字 (16-bit) 到 Flash
 * @param addr  目标地址（必须是 2 字节对齐）
 * @param data  16-bit 数据
 */
static uint8_t flash_write_halfword(uint32_t addr, uint16_t data)
{
    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data);
    HAL_FLASH_Lock();
    return (status == HAL_OK) ? 0 : 1;
}

/**
 * @brief 从 Flash 读取一个半字
 */
static uint16_t flash_read_halfword(uint32_t addr)
{
    return *(volatile uint16_t *)addr;
}

/* ======================== 参数存取逻辑 ======================== */

/**
 * @brief 编码参数为半字
 */
static uint16_t param_encode(uint8_t param_id, uint8_t value)
{
    return (uint16_t)(PARAM_VALID_MARKER | (param_id << 8) | value);
}

/**
 * @brief 解码半字，提取参数 ID 和值
 * @return 0=有效, 1=无效
 */
static uint8_t param_decode(uint16_t raw, uint8_t *param_id, uint8_t *value)
{
    if ((raw & 0xFF00) == PARAM_VALID_MARKER) {
        *param_id = (raw >> 8) & 0x0F;  /* 低4位作为ID */
        *value    = raw & 0xFF;
        return 0;
    }
    return 1;
}

/**
 * @brief 从指定页扫描，找到最新参数值
 * @param page_addr  页起始地址
 * @param param_id   要查找的参数 ID
 * @param value      输出: 找到的值
 * @return 0=找到, 1=未找到
 */
static uint8_t scan_page_for_param(uint32_t page_addr, uint8_t param_id, uint8_t *value)
{
    uint32_t end = page_addr + FLASH_PAGE_SIZE;
    /* 从后向前扫描，找到第一个有效匹配 */
    for (uint32_t addr = end - 2; addr >= page_addr; addr -= 2) {
        uint16_t raw = flash_read_halfword(addr);
        if (raw == PARAM_INVALID) continue;

        uint8_t id, val;
        if (param_decode(raw, &id, &val) == 0 && id == param_id) {
            *value = val;
            return 0;
        }
    }
    return 1;
}

/**
 * @brief 初始化写入指针（找到当前页的空闲位置）
 */
static void find_write_offset(void)
{
    s_write_offset = 0;
    uint32_t end = s_current_page + FLASH_PAGE_SIZE;
    for (uint32_t addr = s_current_page; addr < end; addr += 2) {
        if (flash_read_halfword(addr) == PARAM_INVALID) {
            s_write_offset = (addr - s_current_page) / 2;
            return;
        }
    }
    /* 页满 */
    s_write_offset = FLASH_PAGE_SIZE / 2;
}

/**
 * @brief 切换到备用页（页满时调用）
 */
static void switch_page(void)
{
    uint32_t old_page = s_current_page;
    uint32_t new_page = (s_current_page == PARAM_PAGE0_ADDR)
                        ? PARAM_PAGE1_ADDR : PARAM_PAGE0_ADDR;

    /* 擦除旧页，为将来轮转做准备 */
    flash_erase_page(old_page);

    s_current_page = new_page;
    s_write_offset = 0;
}

/**
 * @brief 写入一个参数
 */
static uint8_t write_param(uint8_t param_id, uint8_t value)
{
    /* 检查当前页是否已满 */
    if (s_write_offset >= FLASH_PAGE_SIZE / 2) {
        switch_page();
    }

    uint32_t addr = s_current_page + s_write_offset * 2;
    uint16_t encoded = param_encode(param_id, value);

    if (flash_write_halfword(addr, encoded) != 0) {
        return 2;  /* Flash 写入失败 */
    }

    s_write_offset++;
    return 0;
}

/* ======================== 公开 API ======================== */

void Param_Init(void)
{
    uint8_t addr_val = 0;

    /* 先在 Page0 找 */
    if (scan_page_for_param(PARAM_PAGE0_ADDR, PARAM_ID_ADDR, &addr_val) == 0) {
        s_current_page = PARAM_PAGE0_ADDR;
    }
    /* 再在 Page1 找 */
    else if (scan_page_for_param(PARAM_PAGE1_ADDR, PARAM_ID_ADDR, &addr_val) == 0) {
        s_current_page = PARAM_PAGE1_ADDR;
    }
    /* 都没找到 → 使用默认值，写入 Page0 */
    else {
        s_current_page = PARAM_PAGE0_ADDR;
        flash_erase_page(PARAM_PAGE0_ADDR);
        addr_val = SLAVE_ADDR_DEFAULT;
        s_write_offset = 0;
        write_param(PARAM_ID_ADDR, addr_val);
        return;
    }

    /* 初始化写入指针 */
    find_write_offset();

    /* 校验地址范围 */
    if (addr_val >= SLAVE_ADDR_MIN && addr_val <= SLAVE_ADDR_MAX) {
        s_slave_addr = addr_val;
    } else {
        s_slave_addr = SLAVE_ADDR_DEFAULT;
    }
}

uint8_t Param_SaveAddr(uint8_t addr)
{
    if (addr < SLAVE_ADDR_MIN || addr > SLAVE_ADDR_MAX) {
        return 1;  /* 参数非法 */
    }

    uint8_t result = write_param(PARAM_ID_ADDR, addr);
    if (result == 0) {
        s_slave_addr = addr;
    }
    return result;
}

uint8_t Param_GetAddr(void)
{
    return s_slave_addr;
}

void Param_ResetDefaults(void)
{
    flash_erase_page(PARAM_PAGE0_ADDR);
    flash_erase_page(PARAM_PAGE1_ADDR);

    s_current_page = PARAM_PAGE0_ADDR;
    s_write_offset = 0;
    s_slave_addr   = SLAVE_ADDR_DEFAULT;

    write_param(PARAM_ID_ADDR, s_slave_addr);
}
