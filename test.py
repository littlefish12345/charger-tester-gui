#!/usr/bin/env python3
"""充电头测试仪 下位机模拟器

通过 CH340 串口向上位机 GUI 发送协议数据，用于测试。
协议格式: <chip_id,command>{json}
"""

import serial
import json
import time
import random
import threading
import sys

CHIP_ID = "ABCDABCD"


def build_frame(command: int, payload: dict) -> bytes:
    json_str = json.dumps(payload, separators=(",", ":"))
    frame = f"<{CHIP_ID},{command:03d}>{json_str}"
    return frame.encode("utf-8")


def send_frame(ser: serial.Serial, command: int, payload: dict):
    data = build_frame(command, payload)
    ser.write(data)
    ser.flush()
    print(f"  -> {data.decode()}")


# ---- 串口接收线程 ----

def reader_thread(ser: serial.Serial, stop_event: threading.Event):
    """后台线程：持续读取串口数据并打印"""
    buf = b""
    while not stop_event.is_set():
        try:
            if ser.in_waiting > 0:
                buf += ser.read(ser.in_waiting)
                # 按行或按帧分隔打印
                while b"<" in buf and b">" in buf:
                    start = buf.index(b"<")
                    end = buf.index(b">", start)
                    # 找到对应的 JSON 结束 }
                    json_start = end + 1
                    depth = 0
                    json_end = json_start
                    for i in range(json_start, len(buf)):
                        ch = buf[i:i+1]
                        if ch == b"{" or ch == b"[":
                            depth += 1
                        elif ch == b"}" or ch == b"]":
                            depth -= 1
                            if depth == 0:
                                json_end = i + 1
                                break
                    if depth == 0 and json_end > json_start:
                        frame = buf[start:json_end]
                        print(f"  <- {frame.decode(errors='replace')}")
                        buf = buf[json_end:]
                    else:
                        break
            else:
                time.sleep(0.05)
        except Exception as e:
            print(f"  [接收错误] {e}")
            break


# ---- 内置测试函数 ----

def cmd_status(ser, voltage_mv=12000, current_ma=1300, info="PD@12V"):
    """发送 202 监测状态上报 (运行 V/I)"""
    send_frame(ser, 202, {"info": info, "I": str(current_ma), "V": str(voltage_mv)})


def cmd_protocol_info(ser, pdo_list=None, pps=None):
    """发送 201 可用协议上报 (PDO/PPS 格式)"""
    payload = {}
    if pdo_list is None:
        pdo_list = [
            {"index": 0, "V": "5000", "I": "3000"},
            {"index": 1, "V": "9000", "I": "3000"},
        ]
    payload["pdo"] = pdo_list
    if pps is not None:
        payload["pps"] = pps
    send_frame(ser, 201, payload)


def cmd_response(ser, command=101, result=0):
    """发送 200 指令回复"""
    send_frame(ser, 200, {"command": str(command), "result": str(result)})


def cmd_auto_poll(ser, interval=1.0, count=0):
    """自动定时上报监测数据"""
    print(f"自动上报: 间隔={interval}s, 次数={'无限' if count == 0 else count}")
    pdo_options = [
        [{"index": 0, "V": "5000", "I": "3000"}, {"index": 1, "V": "9000", "I": "3000"}],
        [{"index": 0, "V": "5000", "I": "3000"}, {"index": 1, "V": "9000", "I": "2000"}, {"index": 2, "V": "12000", "I": "2000"}],
        [{"index": 0, "V": "5000", "I": "3000"}, {"index": 1, "V": "9000", "I": "3000"}, {"index": 2, "V": "15000", "I": "3000"}, {"index": 3, "V": "20000", "I": "2000"}],
    ]
    i = 0
    try:
        while count == 0 or i < count:
            if i % 3 == 0:
                pdo = random.choice(pdo_options)
                pps = {"min_V": "3300", "max_V": "21000", "I": str(random.choice([3000, 5000]))}
                cmd_protocol_info(ser, pdo_list=pdo, pps=pps)

            v = 5000 + random.randint(0, 15000)
            c = random.randint(100, 3000)
            info = random.choice(["PD@5V", "PD@9V", "PD@12V", "PD@20V", "QC3.0@9V"])
            cmd_status(ser, voltage_mv=v, current_ma=c, info=info)
            i += 1
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\n自动上报已停止")


def cmd_raw(ser, text: str):
    data = text.encode("utf-8") if isinstance(text, str) else text
    ser.write(data)
    ser.flush()
    print(f"  -> {text}")


# ---- 交互式菜单 ----

DEFAULT_PDO = [
    {"index": 0, "V": "5000", "I": "3000"},
    {"index": 1, "V": "9000", "I": "3000"},
]
DEFAULT_PPS = {"min_V": "3300", "max_V": "21000", "I": "5000"}


def _menu_201_custom(ser):
    """发送 201 协议信息 (带 PPS 参数)"""
    print("  输入 PDO 格式: index,V,I (空行使用默认并结束)")
    print(f"  默认: {DEFAULT_PDO}")
    pdo_list = []
    while True:
        prompt = f"    PDO[{len(pdo_list)}]: "
        line = input(prompt).strip()
        if not line:
            if not pdo_list:
                pdo_list = DEFAULT_PDO
                print(f"    -> 使用默认 PDO")
            break
        parts = line.split(",")
        if len(parts) >= 3:
            pdo_list.append({"index": int(parts[0]), "V": parts[1], "I": parts[2]})

    has_pps = input(f"  包含 PPS? (y/n, 默认 y): ").strip().lower()
    pps = None
    if has_pps != "n":
        print(f"  默认 PPS: {DEFAULT_PPS}")
        min_v = input(f"    PPS min_V mV [{DEFAULT_PPS['min_V']}]: ").strip() or DEFAULT_PPS["min_V"]
        max_v = input(f"    PPS max_V mV [{DEFAULT_PPS['max_V']}]: ").strip() or DEFAULT_PPS["max_V"]
        i_ma  = input(f"    PPS I mA [{DEFAULT_PPS['I']}]: ").strip() or DEFAULT_PPS["I"]
        pps = {"min_V": min_v, "max_V": max_v, "I": i_ma}

    cmd_protocol_info(ser, pdo_list=pdo_list, pps=pps)


MENU = {
    "1":  ("发送运行状态 202 (V/I)", lambda ser: cmd_status(
        ser,
        voltage_mv=int(input("  电压 mV (默认12000): ") or 12000),
        current_ma=int(input("  电流 mA (默认1300): ") or 1300),
        info=input("  协议信息 (默认PD@12V): ") or "PD@12V",
    )),
    "2":  ("发送协议信息 201 (自定义 PDO/PPS)", _menu_201_custom),
    "3":  ("发送指令回复 (200)", lambda ser: cmd_response(
        ser,
        command=int(input("  回复的指令号 (默认101): ") or 101),
        result=int(input("  结果 (0=成功, 默认0): ") or 0),
    )),
    "4":  ("自动上报监测数据", lambda ser: cmd_auto_poll(
        ser,
        interval=float(input("  间隔秒数 (默认1.0): ") or 1.0),
        count=int(input("  次数 (0=无限, 默认0): ") or 0),
    )),
    "5":  ("发送 201 无 PPS (仅 PDO)", lambda ser: cmd_protocol_info(ser, pps=None)),
    "6":  ("发送 201 无 PDO (模拟未插充电头)", lambda ser: cmd_protocol_info(ser, pdo_list=[])),
    "7":  ("发送原始文本", lambda ser: cmd_raw(ser, input("  输入原始文本: "))),
    "h":  ("显示此菜单", lambda ser: print_menu()),
    "q":  ("退出", lambda ser: sys.exit(0)),
}


def print_menu():
    print("\n" + "=" * 50)
    print("  充电头测试仪 下位机模拟器")
    print("=" * 50)
    for key, (desc, _) in MENU.items():
        print(f"  [{key}] {desc}")
    print("=" * 50)


def list_ports():
    import serial.tools.list_ports
    all_ports = serial.tools.list_ports.comports()
    # Filter: only show USB serial adapters
    ports = [p for p in all_ports
             if "usb" in p.device.lower()
             or "usb" in (p.description or "").lower()
             or "usb" in (p.manufacturer or "").lower()]
    if not ports:
        print("没有找到 USB 串口")
        return []
    print("\n可用串口:")
    for i, p in enumerate(ports):
        print(f"  [{i}] {p.device} - {p.description}")
    return ports


def main():
    print_menu()
    ports = list_ports()
    if not ports:
        return

    sel = input(f"\n选择串口 [0-{len(ports)-1}]: ").strip()
    try:
        port = ports[int(sel)].device
    except (ValueError, IndexError):
        print("无效选择")
        return

    baud = input("波特率 (默认115200): ").strip()
    baud = int(baud) if baud else 115200

    try:
        ser = serial.Serial(port, baud, timeout=0.1)
        print(f"\n已打开 {port} @ {baud}")
    except Exception as e:
        print(f"打开串口失败: {e}")
        return

    # 启动接收线程
    stop_event = threading.Event()
    recv_thread = threading.Thread(target=reader_thread, args=(ser, stop_event), daemon=True)
    recv_thread.start()
    print("接收线程已启动，收到的数据将显示为 '<-'")

    # 交互循环
    while True:
        try:
            cmd = input("\n> ").strip().lower()
            if cmd in MENU:
                _, func = MENU[cmd]
                func(ser)
            elif cmd:
                print(f"未知命令: {cmd}, 输入 h 查看菜单")
        except (KeyboardInterrupt, EOFError):
            print("\n退出")
            break
        except Exception as e:
            print(f"错误: {e}")

    stop_event.set()
    recv_thread.join(timeout=1)
    ser.close()
    print("串口已关闭")


if __name__ == "__main__":
    main()
