# ============================================================
# vending-lane-controller 自定义源码注册
# ============================================================
# CubeMX 生成代码后，在 CMakeLists.txt 中添加:
#   include(custom_sources.cmake)
# ============================================================

# --- App 层：应用逻辑 ---
set(APP_SOURCES
    ${CMAKE_SOURCE_DIR}/App/motor_ctrl.c
    ${CMAKE_SOURCE_DIR}/App/protocol.c
    ${CMAKE_SOURCE_DIR}/App/laser_ctrl.c
    ${CMAKE_SOURCE_DIR}/App/param_store.c
)

set(APP_INCLUDES
    ${CMAKE_SOURCE_DIR}/App
)

# --- 自定义硬件驱动层 ---
set(DRV_SOURCES
    ${CMAKE_SOURCE_DIR}/Drivers_custom/hc595.c
    ${CMAKE_SOURCE_DIR}/Drivers_custom/adc_sense.c
    ${CMAKE_SOURCE_DIR}/Drivers_custom/rs485.c
    ${CMAKE_SOURCE_DIR}/Drivers_custom/gpio_ext.c
)

set(DRV_INCLUDES
    ${CMAKE_SOURCE_DIR}/Drivers_custom
)

# --- Config ---
set(CONFIG_INCLUDES
    ${CMAKE_SOURCE_DIR}/Config
)

# --- 汇总 ---
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${APP_SOURCES}
    ${DRV_SOURCES}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${APP_INCLUDES}
    ${DRV_INCLUDES}
    ${CONFIG_INCLUDES}
)
