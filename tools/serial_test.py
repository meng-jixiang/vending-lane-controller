#!/usr/bin/env python3
"""
串口协议测试工具 — M109E 兼容
用于测试货道控制板 RS-485 通讯

用法:
    python serial_test.py COM3           # 连接串口
    python serial_test.py COM3 --scan    # 扫描从机地址
    python serial_test.py COM3 --id 1    # 获取地址1的序列号
    python serial_test.py COM3 --run 1 0 --type 0x02  # 启动通道0, 两线制电机

依赖:
    pip install pyserial
"""

import serial
import struct
import sys
import time
import argparse


def crc16_modbus(data: bytes) -> bytes:
    """计算 CRC16-MODBUS，返回低字节在前的2字节"""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return struct.pack('<H', crc)


def build_frame(addr: int, cmd: int, data: bytes = None) -> bytes:
    """构建20字节指令帧"""
    if data is None:
        data = b'\x00' * 16
    elif len(data) < 16:
        data = data + b'\x00' * (16 - len(data))
    frame = struct.pack('BB', addr, cmd) + data[:16]
    return frame + crc16_modbus(frame)


def parse_response(frame: bytes):
    """解析20字节响应帧"""
    if len(frame) != 20:
        return None
    addr, cmd = struct.unpack('BB', frame[:2])
    data = frame[2:18]
    crc = frame[18:20]
    # 校验CRC
    expected = crc16_modbus(frame[:18])
    if crc != expected:
        print(f"  ⚠️ CRC校验失败: 期望 {expected.hex()}, 收到 {crc.hex()}")
    return addr, cmd, data


def send_command(ser, addr, cmd, data=None, timeout=1.0):
    """发送指令并等待响应"""
    frame = build_frame(addr, cmd, data)
    ser.write(frame)
    time.sleep(0.05)  # 等待50ms
    response = ser.read(20)
    if len(response) == 0:
        print("  ❌ 超时无响应")
        return None
    return parse_response(response)


def cmd_get_id(ser, addr):
    """获取序列号 (0x01)"""
    print(f"\n📡 获取地址 {addr} 的序列号...")
    result = send_command(ser, addr, 0x01)
    if result:
        _, _, data = result
        serial_no = data[:12].hex()
        print(f"  序列号: {serial_no}")


def cmd_motor_status(ser, addr):
    """查询电机状态 (0x03)"""
    print(f"\n📡 查询电机状态...")
    result = send_command(ser, addr, 0x03)
    if result:
        _, _, data = result
        status = data[0]
        channel = data[1] + (data[2] << 8) if len(data) > 2 else data[1]
        err_code = data[3] if len(data) > 3 else 0
        status_names = {0: '空闲', 1: '执行中', 2: '执行完毕'}
        print(f"  状态: {status_names.get(status, '未知')} ({status})")
        print(f"  通道: {channel}")
        print(f"  结果: 0x{err_code:02X}")


def cmd_read_temp(ser, addr):
    """读温度 (0x07)"""
    print(f"\n📡 读取温度...")
    result = send_command(ser, addr, 0x07)
    if result:
        _, _, data = result
        temp_raw = struct.unpack('>h', data[:2])[0]
        temp = temp_raw * 0.1
        print(f"  温度: {temp:.1f}°C")


def main():
    parser = argparse.ArgumentParser(description='M109E 兼容串口测试工具')
    parser.add_argument('port', help='串口号 (如 COM3)')
    parser.add_argument('--baud', type=int, default=9600, help='波特率 (默认9600)')
    parser.add_argument('--addr', type=int, default=1, help='从机地址 (默认1)')
    parser.add_argument('--id', action='store_true', help='获取序列号')
    parser.add_argument('--status', action='store_true', help='查询电机状态')
    parser.add_argument('--temp', action='store_true', help='读温度')
    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        print(f"✅ 已连接 {args.port} @ {args.baud}bps")
    except Exception as e:
        print(f"❌ 无法打开串口: {e}")
        sys.exit(1)

    if args.id:
        cmd_get_id(ser, args.addr)
    elif args.status:
        cmd_motor_status(ser, args.addr)
    elif args.temp:
        cmd_read_temp(ser, args.addr)
    else:
        # 默认：获取序列号
        cmd_get_id(ser, args.addr)

    ser.close()


if __name__ == '__main__':
    main()
