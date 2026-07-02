# 货道控制板固件

## 模块说明

### Core/ — 系统核心
- `main.c` — 主循环入口
- `system_stm32f1xx.c` — 时钟配置
- `stm32f1xx_it.c` — 中断服务函数
- `stm32f1xx_hal_conf.h` — HAL 配置

### Drivers/ — 硬件驱动
- `hc595.c/.h` — 74HC595 矩阵驱动（P端+N端）
- `adc_sense.c/.h` — ADC 电流检测
- `rs485.c/.h` — RS-485 通讯（USART1）
- `gpio_ext.c/.h` — 光耦输入 + 继电器输出 + LED

### App/ — 应用逻辑
- `motor_ctrl.c/.h` — 电机控制状态机（14种类型）
- `protocol.c/.h` — M109E 协议解析（帧收发+CRC）
- `laser_ctrl.c/.h` — 光幕控制
- `param_store.c/.h` — 参数存储（Flash EEPROM 模拟）

### Config/ — 配置
- `board_config.h` — 引脚定义、通道数、阈值参数
