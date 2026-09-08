# PetCargo STC 代码与通信说明

本文以仓库根目录 `finalhomework/` 为相对路径起点，说明 PetCargo 中全部 STC 代码，以及两条串口通信链路：

1. 遥控 STC 板通过 USB 串口连接 Windows PC，再经局域网控制小车。
2. 小车上的主 STC 板分别通过 UART1 与车载主机通信、通过 UART2 与 CSK5062 语音模组通信。

项目中有两块用途不同的 STC 板：

- **主 STC 板**：安装在小车上，负责传感器、情绪、显示、蜂鸣器、语音命令解析和向 ROS 请求运动。
- **遥控 STC 板**：连接 Windows 笔记本，读取导航键并发送方向帧；它不再使用红外。

## 1. STC 代码总览

### 1.1 主 STC 正式固件

工程目录：`firmware/stc/`

构建产物：`firmware/stc/build/petcargo.hex`

| 相对路径 | 作用 |
|---|---|
| `firmware/stc/main.c` | 正式固件入口；声明 SDCC 中断向量，依次调用 `hal_init()`、`petcargo_init()`，并循环执行 `petcargo_run_once()`。 |
| `firmware/stc/include/config.h` | 项目配置：OLED 开关、光照触发值 `20`、释放值 `15`、震动阈值和串口波特率 `9600`。 |
| `firmware/stc/include/stc15.h` | STC15F2K60S2 特殊功能寄存器、端口和板级引脚定义。 |
| `firmware/stc/include/hal.h` | HAL 公共接口：时基、ADC、按键、显示、LED、蜂鸣器、UART1/UART2 和红外兼容接收接口。 |
| `firmware/stc/src/hal.c` | 底层驱动实现。Timer0 负责 1 ms 时基和数码管/LED 扫描；Timer1 驱动蜂鸣器；Timer2 为两个 UART 提供 9600 波特率；UART 使用接收环形缓冲和发送队列。 |
| `firmware/stc/include/devices.h` | ADXL345、DS1302 和 EEPROM 持久化的数据结构及接口。 |
| `firmware/stc/src/devices.c` | ADXL345 软件 I²C、DS1302 RTC、24C01 EEPROM 轮换槽读写与校验。 |
| `firmware/stc/include/oled.h` | OLED 表情编号和驱动接口。 |
| `firmware/stc/src/oled.c` | SSD1306 128×64、地址 `0x3C` 的显示驱动；通过 SM/S1、SM/S2 的反相逻辑分批刷新预制像素表情。 |
| `firmware/stc/include/music.h` | 非阻塞音乐和短提示音接口。 |
| `firmware/stc/src/music.c` | 音乐状态机；调用 Timer1 蜂鸣器驱动，播放期间不阻塞传感、串口和按键任务。 |
| `firmware/stc/src/score.c` | 简谱转录后的音高、时值和连奏数据。 |
| `firmware/stc/include/runtime.h` | 按键消抖、CSK 帧解析器、UART 发送队列和音乐节拍数据结构。 |
| `firmware/stc/src/runtime.c` | 30 ms 按键消抖、1.2 s 长按、CSK 四字节帧重同步、异步发送队列和 136 BPM 时值计算。 |
| `firmware/stc/include/protocol.h` | STC—ROS 二进制协议 v2 的消息类型、动作类型、事件类型和编解码接口。 |
| `firmware/stc/src/protocol.c` | `AA 55` 帧编码、CRC16-CCITT、UART1 增量解析、错误计数和 ACK/NACK。 |
| `firmware/stc/include/petcargo.h` | 应用层初始化和单次调度接口。 |
| `firmware/stc/src/petcargo.c` | 主业务：传感采样、快乐/恐惧状态、按键、OLED/数码管/LED、语音命令、强光逃跑、运动请求、遥测、急停和通信结果处理。 |
| `firmware/stc/Makefile` | Linux/Make 构建入口，使用 SDCC、11.0592 MHz、large model、2 KB XRAM。 |
| `firmware/stc/build.bat` | Windows 一键构建入口，每个 `.c` 单独编译后链接并用 `packihx` 生成 HEX。 |

正式固件的主要调度周期位于 `firmware/stc/src/petcargo.c`：输入约 10 ms、光照 50 ms、加速度 40 ms、温度 500 ms、UI 100 ms、遥测 200 ms、心跳 500 ms、RTC 和情绪回落 1 s。

### 1.2 主 STC 诊断固件

入口：`firmware/stc/diagnostic.c`

构建产物：`firmware/stc/build/petcargo_diagnostic.hex`

诊断固件复用 `hal.c`、`oled.c`、`runtime.c`、`music.c` 和 `score.c`，但不链接正式业务状态机及 ROS 二进制协议。它通过 UART1 输出 ASCII 诊断信息，包括按键原始状态、K3 ADC、UART2 接收字节、有效语音帧、错误和溢出计数。该固件不会发送小车运动请求，不能与正式 ROS 串口桥混用。

### 1.3 Windows USB 遥控板固件

工程目录：`firmware/stc_remote/`

构建产物：`firmware/stc_remote/build/petcargo_remote.hex`

| 相对路径 | 作用 |
|---|---|
| `firmware/stc_remote/main.c` | 遥控板入口；初始化 ADC7、Timer0 显示扫描和 UART1，读取导航键并发送六字节方向帧。 |
| `firmware/stc_remote/remote.c` | ADC 区间映射、30 ms 消抖、100 ms 按住续发、松开双帧和 XOR 校验。 |
| `firmware/stc_remote/include/remote.h` | 方向编号、按键状态结构和遥控算法接口。 |
| `firmware/stc_remote/include/stc15.h` | 遥控工程使用的 STC 寄存器定义。 |
| `firmware/stc_remote/Makefile` | Make 构建入口。 |
| `firmware/stc_remote/build.bat` | Windows SDCC 构建入口。 |
| `firmware/stc_remote/README.md` | 烧录、数码管显示和串口帧摘要。 |

遥控板数码管显示 `Cn Axxx`：`n` 是方向编号，`xxx` 是导航键 ADC 原始值。方向编号为 `0=松开、1=前、2=后、3=左、4=右、5=停止`。

## 2. 遥控 STC 与 Windows PC 的串口通信

### 2.1 完整链路

```text
导航键
  -> firmware/stc_remote/main.c
  -> 遥控板 UART1 / 板载 CH340 / USB
  -> Windows tools/remote_bridge.py
  -> HTTP POST /api/jog
  -> 小车 ros/petcargo_ros/scripts/dashboard_server.py
  -> ROS /petcargo/jog_request
  -> motion_executor.py -> safety_gateway.py -> /cmd_vel
  -> ucar_controller 底盘
```

物理连接只需要遥控 STC 板的 USB 线。USB 同时供电并提供 CH340 串口，无需将遥控板用杜邦线连接到小车。

串口参数：`9600 baud、8 data bits、无校验、1 stop bit（8N1）`。

### 2.2 六字节遥控帧

```text
A5 5A | 02 | DIRECTION | SEQUENCE | XOR
```

| 字节偏移 | 含义 |
|---:|---|
| 0–1 | 帧头 `A5 5A`。 |
| 2 | 协议版本，固定 `02`。 |
| 3 | 方向：`00` 松开、`01` 前、`02` 后、`03` 左、`04` 右、`05` 停止。 |
| 4 | 8 位递增序号，溢出后回到 0。 |
| 5 | 前五字节逐字节 XOR 的结果。 |

编码端位于 `firmware/stc_remote/main.c` 的 `uart_send()`；校验函数位于 `firmware/stc_remote/remote.c`。Windows 解码器位于 `tools/remote_protocol.py`，能够从噪声、错位帧和校验错误后重新同步。

### 2.3 Windows 桥接代码

| 相对路径 | 作用 |
|---|---|
| `tools/run_remote.bat` | Windows 启动入口，接收机器人网页地址和可选 COM 口。 |
| `tools/remote_bridge.py` | 自动寻找 CH340、读取串口帧、维持 300 ms 串口失联保护，并异步调用小车 HTTP 接口。 |
| `tools/remote_protocol.py` | 与 I/O 无关的六字节帧解析器。 |
| `ros/petcargo_ros/src/petcargo_ros/remote_api.py` | 小车端校验 HTTP 点动参数。 |
| `ros/petcargo_ros/scripts/dashboard_server.py` | 提供 `POST /api/jog`，将请求发布到 ROS。 |

Windows 桥向小车发送：

```json
{"direction":1,"speed_mm_s":250,"lease_ms":500}
```

服务端接受方向 0–4、速度 60–300 mm/s、租约 100–500 ms，并添加 `source=windows_stc`。按键保持期间遥控板每 100 ms 发帧；Windows 桥只保留最新方向，避免网络延迟造成旧方向排队。松开、中心/K3、程序退出或 300 ms 收不到串口帧都会请求停车；机器人端租约到期也会自动归零。

启动示例：

```bat
tools\run_remote.bat http://192.168.1.6:8080 COM8
```

## 3. 主 STC 与车载主机的 UART1 通信

### 3.1 物理与软件路径

主 STC 板通过自身 USB 连接车载主机。USB 为开发板供电，同时由板载 CH340 将 UART1 转换为 USB 串口。

```text
主 STC UART1
  <-> 板载 CH340 / USB
  <-> /dev/petcargo_stc
  <-> ros/petcargo_ros/scripts/serial_bridge.py
  <-> PetCargo ROS 话题
```

相关文件：

| 相对路径 | 作用 |
|---|---|
| `firmware/stc/src/hal.c` | UART1 初始化、中断收发、64 字节 RX 环形缓冲和异步 TX 帧队列。 |
| `firmware/stc/src/protocol.c` | MCU 端协议 v2 编解码。 |
| `firmware/stc/src/petcargo.c` | 生成遥测/事件/动作，处理 ROS 返回结果、STOP 和 ACK。 |
| `ros/petcargo_ros/src/petcargo_ros/protocol.py` | PC/ROS 端同构协议、数据结构和增量解析器。 |
| `ros/petcargo_ros/scripts/serial_bridge.py` | pyserial 重连、心跳、协议与 ROS JSON 话题互转。 |
| `ros/petcargo_ros/config/petcargo.yaml` | 串口设备、9600 波特率和 1.5 s 链路超时。 |
| `ros/petcargo_ros/udev/99-petcargo-stc.rules` | 将 CH340 固定映射为 `/dev/petcargo_stc`。 |
| `tools/install_udev_rule.sh` | 安装 udev 规则。 |
| `tools/run_robot.sh` | 检查串口设备并启动唯一一套 ROS 节点。 |

串口参数同样为 `9600、8N1`。UART1 和 UART2 共用 Timer2 波特率发生器。

### 3.2 二进制协议 v2

```text
AA 55 | VERSION | TYPE | SEQ | LENGTH | PAYLOAD | CRC16_LO CRC16_HI
```

- `VERSION` 固定为 `02`。
- `LENGTH` 最大为 32。
- 多字节整数采用小端序。
- CRC16-CCITT 初值 `0xFFFF`、多项式 `0x1021`，覆盖 `VERSION` 到 `PAYLOAD`。
- CRC、长度或版本错误的帧不执行动作；解析器会重新搜索 `AA 55`。

| TYPE | 名称 | 方向 | PAYLOAD |
|---:|---|---|---|
| `01` | HELLO | 双向 | 能力/版本标识。 |
| `02` | HEARTBEAT | 双向 | `uptime_ms:u32`。 |
| `10` | TELEMETRY | STC→ROS | 20 字节遥测。 |
| `11` | EVENT | STC→ROS | `event:u8 + value:i16`。 |
| `20` | MOTION_REQUEST | STC→ROS | `id:u16 + kind:u8 + distance_mm:i16 + angle_cdeg:i32 + max_speed:u16`。 |
| `21` | MOTION_RESULT | ROS→STC | `id:u16 + result:u8 + actual_mm:i16 + actual_cdeg:i32`。 |
| `22` | JOG_REQUEST | STC→ROS | `direction:u8 + speed_mm_s:u16 + lease_ms:u16`，为旧主板红外入口兼容保留。 |
| `31` | STOP | 双向 | `0` 解除锁定、`1` 锁定急停、`2` 仅取消当前动作。 |
| `7E/7F` | ACK/NACK | 双向 | `ref_type:u8 + ref_seq:u8 + code:u8`。 |

20 字节遥测依次为：`uptime_ms:u32`、`light_raw:u8`、`temp_x10:i16`、三轴加速度 `i16×3`、`vibration:u8`、`hall:u8`、`happiness:u8`、`fear:u8`、`flags:u8`、`feed_count:u16`。Flags 的 bit0 表示睡眠、bit1 表示急停、bit4 表示传感器/ADC 故障、bit5 表示主板兼容遥控输入活跃。

动作类型为 `1=前后直线、2=原地旋转、3=左右横移`。动作结果为 `0=完成、1=停止、2=超时、3=里程计异常、4=忙、5=安全拒绝、6=雷达不可用、7=障碍阻挡`。

### 3.3 ROS 转换关系

`serial_bridge.py` 将 STC 帧转换为以下 ROS 话题：

| ROS 话题 | 来源/去向 | 用途 |
|---|---|---|
| `/petcargo/telemetry` | STC→ROS | JSON 传感器和情绪状态。 |
| `/petcargo/events` | STC→ROS | 强光、震动、投喂、语音、按键和故障事件。 |
| `/petcargo/motion_request` | STC→ROS | 固定距离或旋转动作。 |
| `/petcargo/jog_request` | STC/Windows→ROS | 带租约的方向点动。 |
| `/petcargo/motion_result` | ROS→STC | 动作完成或失败结果。 |
| `/petcargo/safety_set` | 双向协调 | 锁定/解除急停。 |
| `/petcargo/cancel_motion` | STC→ROS | 取消当前动作但不锁定后续命令。 |
| `/petcargo/serial_connected` | bridge→ROS | 1.5 s 内收到有效 STC 帧才视为连接。 |

`motion_executor.py` 将动作请求结合里程计和雷达生成内部速度，`safety_gateway.py` 是 `/cmd_vel` 的唯一发布者。串口断开、急停、雷达失联或速度租约超时都会输出零速度。

## 4. 主 STC 与 CSK5062 语音模组的 UART2 通信

### 4.1 接线与串口

- CSK `A11/TX` → STC `P1.0/RXD2`。
- CSK `A12/RX` ← STC `P1.1/TXD2`。
- CSK GND 与 STC GND 共地。
- CSK 使用独立 USB 供电，**禁止连接 CSK VCC 与 STC VCC**。
- 串口为 `9600、8N1`；只连接协议串口，不连接模组日志 UART。

配置文件和说明位于：

| 相对路径 | 作用 |
|---|---|
| `voice/csk5062/commands.md` | 工作台命令、泛化词、回复、发送/接收协议及 STC 行为总表。 |
| `voice/csk5062/commands.csv` | 命令表 CSV 版本。 |
| `voice/csk5062/serial_mapping.json` | 命令与串口字节映射。 |
| `voice/csk5062/README.md` | 模组配置、接线和烧录说明。 |
| `voice/csk5062/sdk_update_notes.md` | 当前 MARS SDK 产物和串口配置记录。 |
| `mars-sdk/` | 用户重新打包并确认音频、协议正确的语音 SDK；主 STC 代码不修改其协议。 |

### 4.2 CSK 发给 STC

```text
A5 5A | COMMAND | (COMMAND XOR FF)
```

命令范围是 `01–0C`：停止、前、后、左、右、转圈、睡觉、起床、卖萌、唱歌、询问心情、报告状态。完整语句和回复以 `voice/csk5062/commands.md` 为准。

接收路径：UART2 中断将字节放入 RX 环形缓冲；`petcargo.c` 的 `poll_csk()` 逐字节调用 `runtime.c` 的 `csk_feed()`；只有帧头、命令范围和反码校验全部正确才调用 `voice_command()`。残帧超过 100 ms 会丢弃并重同步；UART2 溢出时清空残帧，避免错误字节触发动作。

语音动作示例：

- `A5 5A 02 FD`：向前 500 mm。
- `A5 5A 03 FC`：向后 500 mm。
- `A5 5A 04 FB`：向左横移 500 mm。
- `A5 5A 05 FA`：向右横移 500 mm。
- `A5 5A 0A F5`：从第一音开始播放一次蜂鸣器音乐。

语音模组的回复音频由 CSK 自己播放；STC 不保存这些人声音频。收到有效命令后，STC 先执行本地显示/音乐/动作逻辑，再通过 UART1 向 ROS 上报，不要求小车先返回确认。

### 4.3 STC 发给 CSK

```text
5A A5 | COMMAND | (COMMAND XOR FF)
```

当前使用 `COMMAND=80` 触发强光警告播报，即 `5A A5 80 7F`。发送函数是 `firmware/stc/src/petcargo.c` 中的 `csk_send()`，底层调用 `hal_uart2_write()`。如果语音 SDK 不支持被动接收触发，强光时 STC 的 OLED、LED、蜂鸣器和运动请求仍独立工作。

## 5. 构建、烧录与进一步阅读

Windows 构建主板：

```bat
firmware\stc\build.bat
```

Windows 构建遥控板：

```bat
firmware\stc_remote\build.bat
```

使用 STC-ISP 烧录时，两块板均选择 11.0592 MHz。正式主板烧录 `petcargo.hex`，排查串口/按键时才烧录 `petcargo_diagnostic.hex`，Windows 遥控板烧录 `petcargo_remote.hex`。

相关专题文档：

- `docs/protocol.md`：通信协议速查。
- `docs/windows-stc-remote.md`：Windows 遥控部署和排查。
- `docs/wiring.md`：主板、OLED、CSK 接线。
- `docs/firmware-build-report.md`：固件构建与资源报告。
- `docs/troubleshooting.md`：常见故障排查。
- `voice/csk5062/commands.md`：语音工作台命令表。
