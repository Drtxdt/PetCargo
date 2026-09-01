# STC 固件

## 编译与烧录

- 编译器：SDCC，MCS-51 后端。
- MCU：IAP15F2K61S2 / STC15F2K60S2 兼容寄存器集。
- 系统时钟：11.0592 MHz。
- UART1 与 UART2：9600、8N1，共用 Timer2 波特率发生器。
- 输出：`build/petcargo.hex`。
- SDCC 使用 large memory model，把状态与协议缓冲区放入片上扩展 RAM，避免挤占 8051 的 128 字节直接寻址 RAM。

运行 `build.bat` 后，使用 STC-ISP 烧录 `build/petcargo.hex`，并在 STC-ISP 中把 IRC/系统频率设为 11.0592 MHz。

## 定时器分配

- Timer0：1 ms 调度时基。
- Timer1：板载蜂鸣器硬件方波。
- Timer2：UART1 和 UART2 的 9600 波特率。

## 首次校准

1. 正常教室光线下长按 K1，数码管显示 `CAL1`。
2. 用手电筒照射光敏电阻。
3. 保持照射并短按 K2。
4. 成功时蜂鸣器发出较高提示音；若明暗差不足会发低音并保持校准状态。

K3 与导航 ADC、DS1302 的复用来自原理图。固件将导航 ADC 小于 12 识别为 K3 按下。
