# 货道控制板固件 — 系统说明文档

> **版本**: V1.0 | **更新**: 2026-07-02 | **MCU**: STM32F103C8T6

---

## 一、项目概述

30×30 货道控制板，驱动 900 通道出货机构，通过 RS-485 通讯与上位机对接，兼容 M109E 售货机控制卡协议。

| 参数 | 值 |
|------|-----|
| MCU | STM32F103C8T6 (72MHz, 64KB Flash, 20KB RAM) |
| 矩阵 | 30行×30列，74HC595×8片级联 |
| 通讯 | USART3 RS-485, 9600bps, DMA |
| 调试 | USART1, 115200bps |
| 看门狗 | IWDG 已启用 |
| 时基 | TIM3 (替代 SysTick) |

---

## 二、分层架构

```
┌─────────────────────────────────────────────┐
│              app/  业务逻辑                  │  出货主流程
├─────────────────────────────────────────────┤
│           service/  通用服务                 │  协议解析、参数存储
├─────────────────────────────────────────────┤
│           device/  设备驱动                  │  电机驱动、光幕（预留）
├─────────────────────────────────────────────┤
│            bsp/  板级支持                    │  595、UART、GPIO、ADC、LED
├─────────────────────────────────────────────┤
│         HAL  硬件抽象层                      │  STM32 HAL Driver
├─────────────────────────────────────────────┤
│          Hardware  硬件                      │  MCU 外设寄存器
└─────────────────────────────────────────────┘
```

**依赖规则**：上层可调用下层，下层不可调用上层，同层不互相调用。

---

## 三、目录结构

```
firmware/
│
├── Core/                       CubeMX 自动生成（不要手动修改 USER CODE 区域以外的代码）
│   ├── Inc/
│   │   ├── main.h              引脚宏定义（CubeMX 生成）
│   │   ├── gpio.h              GPIO 初始化声明
│   │   ├── adc.h               ADC 初始化声明
│   │   ├── dma.h               DMA 初始化声明
│   │   ├── usart.h             UART 初始化声明
│   │   └── iwdg.h              看门狗声明
│   └── Src/
│       ├── main.c              入口：初始化 + 主循环
│       ├── gpio.c              CubeMX 生成的 GPIO 配置
│       ├── adc.c               CubeMX 生成的 ADC 配置
│       ├── usart.c             CubeMX 生成的 UART 配置
│       ├── dma.c               DMA 初始化
│       ├── iwdg.c              看门狗初始化
│       ├── stm32f1xx_it.c      中断服务函数
│       ├── stm32f1xx_hal_msp.c HAL MSP 初始化
│       └── system_stm32f1xx.c  系统初始化
│
├── Drivers/                    STM32 HAL 库（ST 官方，不要修改）
│   ├── CMSIS/                  ARM CMSIS 核心头文件
│   └── STM32F1xx_HAL_Driver/   HAL 驱动源码
│
├── Config/
│   └── board_config.h          ★ 板级配置：引脚定义、参数、阈值
│
├── bsp/                        ★ BSP 层 — 板级硬件封装
│   ├── bsp_led.c/h             LED 指示灯（开/关/闪）
│   ├── bsp_uart.c/h            USART1 调试 + USART3 RS-485 通讯
│   ├── bsp_595.c/h             74HC595 矩阵驱动（P端+N端级联）
│   ├── bsp_gpio.c/h            到位检测 + 光幕 + NMOS 输出
│   └── bsp_adc.c/h             电流检测 + 温度检测
│
├── service/                    ★ Service 层 — 通用服务
│   ├── protocol.c/h            M109E 协议解析（帧收发、CRC、指令分发）
│   └── param_store.c/h         Flash 参数存储（地址掉电保存）
│
├── device/                     Device 层（预留，电机驱动等）
│
├── app/                        App 层（预留，出货主流程）
│
├── cmake/                      CMake 工具链文件
│   ├── gcc-arm-none-eabi.cmake ARM GCC 工具链配置
│   └── stm32cubemx/            CubeMX CMake 子目录
│
├── .vscode/                    VSCode 配置
│   ├── settings.json           clangd + CMake + Cortex-Debug
│   ├── launch.json             DAP-Link 调试配置
│   └── tasks.json              构建/烧录任务
│
├── CMakeLists.txt              CMake 主构建文件
├── custom_sources.cmake        自定义源码注册（bsp/service/device/app）
├── STM32F103XX_FLASH.ld        链接脚本
├── startup_stm32f103xb.s       启动文件
└── lane_board.ioc              CubeMX 工程文件
```

---

## 四、硬件引脚分配

### 4.1 595 矩阵驱动

| 信号 | 引脚 | 功能 |
|------|------|------|
| N端 DS | PA4 | N端数据输入 |
| N端 SHCP | PA5 | N端移位时钟 |
| N端 STCP | PA6 | N端锁存时钟 |
| N端 MR | PA7 | N端复位（低有效） |
| P端 DS | PB12 | P端数据输入 |
| P端 SHCP | PB13 | P端移位时钟 |
| P端 STCP | PB14 | P端锁存时钟 |
| P端 MR | PB15 | P端复位（低有效） |

### 4.2 通讯接口

| 接口 | 引脚 | 波特率 | 用途 |
|------|------|--------|------|
| USART1 TX/RX | PA9/PA10 | 115200 | 调试日志 |
| USART3 TX/RX | PB10/PB11 | 9600 | RS-485 通讯（DMA） |

### 4.3 ADC 采样

| 通道 | 引脚 | 功能 |
|------|------|------|
| ADC1 CH0 | PA0 | 电流检测（OPA2365, 增益21倍） |
| ADC2 CH1 | PA1 | NTC 温度检测 |

### 4.4 GPIO 输入（12路）

| 引脚 | 功能 | 有效电平 |
|------|------|---------|
| PC13 | 到位检测 Row0 | 低=到位 |
| PC14 | 到位检测 Row1 | 低=到位 |
| PC15 | 到位检测 Row2 | 低=到位 |
| PB3 | 到位检测 Row3 | 低=到位 |
| PB4 | 到位检测 Row4 | 低=到位 |
| PB5 | 到位检测 Row5 | 低=到位 |
| PB6 | 到位检测 Row6 | 低=到位 |
| PB7 | 到位检测 Row7 | 低=到位 |
| PB8 | 到位检测 Row8 | 低=到位 |
| PB9 | 到位检测 Row9 | 低=到位 |
| PA12 | 光幕1 | 低=掉货 |
| PA15 | 光幕2 | 低=掉货 |

### 4.5 GPIO 输出

| 引脚 | 功能 |
|------|------|
| PB0 | NMOS1 开关量 |
| PB1 | NMOS2 开关量 |
| PA2 | NMOS3 开关量 |
| PA3 | NMOS4 开关量 |
| PA8 | LED 指示灯 |

---

## 五、模块说明

### 5.1 BSP 层

#### bsp_led — LED 指示灯

```c
BSP_LED_On();                       // 常亮
BSP_LED_Off();                      // 灭
BSP_LED_Toggle();                   // 翻转
BSP_LED_Blink(times, interval_ms);  // 闪烁 N 次
```

#### bsp_uart — 串口通讯

```c
BSP_RS485_Send(data, len);          // RS-485 发送
BSP_RS485_RxCallback(byte);         // 中断回调喂入
BSP_RS485_GetByte(&byte);           // 从环形缓冲取字节
```

#### bsp_595 — 矩阵驱动

```c
BSP_595_SelectRow(row);             // 选通行 (0-29)
BSP_595_SetColumns(col_mask);       // 设置列掩码 (30bit)
BSP_595_AllOff();                   // 全部关闭
```

#### bsp_gpio — 输入检测 + 输出控制

```c
BSP_DI_GetRow(row);                 // 到位检测 (0-9), 返回 1=到位
BSP_Laser1_IsBlocked();             // 光幕1, 返回 1=有遮挡
BSP_Laser2_IsBlocked();             // 光幕2
BSP_NMOS_On(ch);                    // NMOS 输出 (0-3)
BSP_NMOS_Off(ch);
```

#### bsp_adc — 电流/温度

```c
BSP_ADC_ReadCurrent();              // ADC 原始值 (0~4095)
BSP_ADC_ReadCurrent_mA();           // 电流 (mA)
BSP_ADC_ReadTemp();                 // 温度 ADC 值
```

### 5.2 Service 层

#### protocol — M109E 协议

- **帧格式**: 20 字节定长, CRC16-MODBUS
- **接收**: USART3 中断 → Proto_RxByte() → 20 字节帧缓冲
- **处理**: Proto_Process() 在主循环调用，解析+分发+响应

已实现指令:
| 指令 | 功能 | 状态 |
|------|------|------|
| 0x01 | 获取序列号 | ✅ 占位响应 |
| 0x03 | 查询电机状态 | ✅ 返回空闲 |
| 0x07 | 读温度 | ✅ 返回 -50℃ |
| 0xFF | 设置地址 | ✅ 完整实现 |

#### param_store — 参数存储

- **方案**: STM32 Flash 双页轮转（模拟 EEPROM）
- **地址**: Flash 最后 2KB (0x0800F800 ~ 0x0800FFFF)
- **特性**: 掉电保存，自动加载，磨损均衡

```c
Param_Init();                       // 启动时加载
Param_SaveAddr(addr);               // 保存地址 (1-8)
Param_GetAddr();                    // 读取地址
```

---

## 六、数据流

```
上位机 (RS-485)
    │
    ▼
USART3 中断 → bsp_uart 环形缓冲
    │
    ▼
Proto_RxByte() 逐字节喂入 → 20字节帧缓冲
    │
    ▼
Proto_Process() 主循环处理
    ├── CRC 校验
    ├── 地址匹配
    ├── 指令分发
    │   ├── 0xFF → Param_SaveAddr() → Flash 写入
    │   ├── 0x01 → 返回序列号
    │   ├── 0x03 → 返回电机状态
    │   └── ...
    └── BSP_RS485_Send() 发送响应帧
```

---

## 七、编译与烧录

### 7.1 命令行

```bash
cd firmware

# 配置 (首次)
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build

# 烧录 (DAP-Link)
openocd -f interface/cmsis-dap.cfg -f target/stm32f1x.cfg -c "program build/lane_board.elf verify reset exit"
```

### 7.2 VSCode

| 操作 | 快捷键 |
|------|--------|
| 编译 | Ctrl+Shift+B |
| 调试 | F5 |
| 搜索文件 | Ctrl+P |
| 全局搜索 | Ctrl+Shift+F |

---

## 八、RAM / Flash 使用

| 区域 | 已用 | 总量 | 占比 |
|------|------|------|------|
| RAM | 2192 B | 20 KB | 10.7% |
| Flash | 17256 B | 64 KB | 26.3% |

---

## 九、待开发模块

| 模块 | 目录 | 功能 | 状态 |
|------|------|------|------|
| motor_ctrl | device/ | 电机控制状态机 (14种类型) | 待开发 |
| laser_ctrl | device/ | 光幕自检 + 掉货检测 | 待开发 |
| vending | app/ | 出货主流程 | 待开发 |
| fault_log | service/ | 故障日志 | 待开发 |

---

## 十、开发规范

1. **CubeMX 重新生成代码时**: 只修改 `USER CODE BEGIN/END` 区域内的代码
2. **新增模块**: 按分层放到对应目录，在 `custom_sources.cmake` 注册
3. **BSP 层**: 只有 bsp/ 可以调用 HAL 库函数
4. **Service 层**: 通过 bsp/ 接口操作硬件，不直接调用 HAL
5. **命名规范**: BSP 层函数 `BSP_XXX_YYY()`，Service 层函数 `模块名_动作()`
