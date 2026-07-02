# ============================================================
# 分层架构源码注册
# ============================================================
# bsp/      — 板级硬件封装 (唯一操作 HAL 的层)
# device/   — 设备驱动 (调用 bsp 接口)
# service/  — 通用服务 (协议/存储, 不碰硬件)
# app/      — 业务逻辑 (出货流程)
# ============================================================

# --- BSP 层 ---
set(BSP_SOURCES
    ${CMAKE_SOURCE_DIR}/bsp/bsp_led.c
    ${CMAKE_SOURCE_DIR}/bsp/bsp_uart.c
    ${CMAKE_SOURCE_DIR}/bsp/bsp_595.c
    ${CMAKE_SOURCE_DIR}/bsp/bsp_gpio.c
    ${CMAKE_SOURCE_DIR}/bsp/bsp_adc.c
)
set(BSP_INCLUDES ${CMAKE_SOURCE_DIR}/bsp)

# --- Device 层 (预留) ---
set(DEV_SOURCES "")
set(DEV_INCLUDES "")

# --- Service 层 ---
set(SVC_SOURCES
    ${CMAKE_SOURCE_DIR}/service/protocol.c
    ${CMAKE_SOURCE_DIR}/service/param_store.c
)
set(SVC_INCLUDES ${CMAKE_SOURCE_DIR}/service)

# --- App 层 (预留) ---
set(APP_SOURCES "")
set(APP_INCLUDES "")

# --- Config ---
set(CONFIG_INCLUDES ${CMAKE_SOURCE_DIR}/Config)

# --- 汇总 ---
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${BSP_SOURCES}
    ${DEV_SOURCES}
    ${SVC_SOURCES}
    ${APP_SOURCES}
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${BSP_INCLUDES}
    ${DEV_INCLUDES}
    ${SVC_INCLUDES}
    ${APP_INCLUDES}
    ${CONFIG_INCLUDES}
)
