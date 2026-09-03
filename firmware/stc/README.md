# STC 固件

## 编译与烧录

- 编译器：SDCC，MCS-51 后端。
- MCU：IAP15F2K61S2 / STC15F2K60S2 兼容寄存器集。
- 系统时钟：11.0592 MHz。
- UART1 与 UART2：9600、8N1，共用 Timer2 波特率发生器。
- 输出：`build/petcargo.hex`。
- SDCC 使用 large memory model，把状态与协议缓冲区放入片上扩展 RAM，避免挤占 8051 的 128 字节直接寻址 RAM。

根目录运行 `build.bat all`，分别编译每个C后链接主板、诊断板、遥控板；`build.bat main` / `diagnostic` 可单独构建。使用 STC-ISP 烧录 `build/petcargo.hex`，IRC/系统频率设为11.0592 MHz，**P5.4设置为普通I/O，而非复位**。

## 定时器分配

- Timer0：1 ms自动重装载，固定扫描8位数码管+LED，双缓冲，优先于UART/PCA。
- Timer1：板载蜂鸣器硬件方波。
- Timer2：UART1 和 UART2 的 9600 波特率。

## 当前交互

- K1短按切页；K2短按睡眠/唤醒、长按1.2秒切换音乐；K3锁定急停，松开再长按解除。
- K3只与导航ADC共享P1.7，0–28为K3；RTC的IO=P5.4、CE=P1.6、CLK=P1.5。
- 光照直接使用原始ADC：L≥20持续200ms触发，L≤15持续2秒重新武装，不再使用旧校准流程。
- 音乐按`music/1.png`到`3.png`播放：降B调，136 BPM，每轮约17.87秒，持续循环。语音“停下来”同时停止运动与音乐，不锁定；急停期间拒绝重新启动音乐。
- UART2只接受带反码校验的4字节命令，残帧100ms超时；ROS二进制协议仍为v2。
- 新增拒绝事件为`EVENT_FAULT, value=100+命令编号`，例如110表示锁定状态拒绝唱歌。仍会上报原`EVENT_VOICE`。

## 验证

根目录运行 `python tools/test_firmware.py` 执行生产C状态机测试（需要GCC/Clang）；构建后运行 `python tools/verify_firmware.py` 审计HEX/向量/内存/扫描汇编。两者都不能代替实物验收。

独立诊断镜像：`build/petcargo_diagnostic.hex`，说明见 `docs/bench-test.md`。正式固件发送二进制，诊断固件发送ASCII；不要把诊断固件接入ROS串口桥。
