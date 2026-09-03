# PetCargo 无焊接接线

接线依据 STC-B 学习板原理图和 EVM-KWS-P-0101-V2.0 模组丝印。所有连接均使用杜邦线。

## 上电前原则

1. CSK5062 使用独立 USB 供电。
2. **禁止连接 CSK VCC 与 STC VCC。**
3. STC 与 CSK 必须共地，否则 UART 没有共同电平参考。
4. 断电完成全部杜邦线连接，核对后再分别上电。
5. 小车底盘原串口 `/dev/ttyS0` 不动；STC 使用另一个 USB 口。

## CSK5062 ↔ STC EXT

| CSK5062 | 方向 | STC EXT | 用途 |
|---|---:|---|---|
| A11 / TX | → | P1.0 / RXD2 | CSK 命令输出 |
| A12 / RX | ← | P1.1 / TXD2 | STC 强光播报触发 |
| GND | ↔ | GND | 共地 |
| VCC | 不接 | — | CSK 独立 USB 供电 |

EXT 丝印从功能上为 `VCC、P1.1、P1.0、GND`。以板上丝印为准，不以照片方向猜测上下。

## OLED ↔ STC

| OLED | STC 接点 | 说明 |
|---|---|---|
| VCC | EXT VCC | OLED 由 STC 板供电 |
| GND | EXT GND | 共地 |
| SCL/SCK | SM S1 | P4.4 → ULN2003 反相开集电极 |
| SDA | SM S2 | P4.3 → ULN2003 反相开集电极 |

SM 插座丝印顺序为 `VCC、S1、S2、S3、S4`。OLED 不接 SM VCC，便于把数据线和供电线分开核对。

固件已经补偿 ULN2003 反相：MCU 输出 0 表示释放总线，输出 1 表示拉低。OLED ACK 被有意忽略。

板载 RTC：SCLK=P1.5、CE=P1.6、IO=P5.4；**STC-ISP 的 P5.4 选普通 I/O，不选复位脚**。K3/导航 ADC 独占 P1.7，不能被 RTC 驱动改写。原文档把 S1/S2 写成 P4.1/P4.2 属于错误映射，现已按原理图修正。

杜邦线自身不分方向，端点必须交叉：CSK 的 **A11→STC P1.0**、**A12←STC P1.1**。不要用模块另一排标着 TX/RX/VCC 的日志串口替代 A11/A12。

## USB

- STC USB → 小车主机：供电、烧录和 UART1/CH340 数据。
- CSK USB → 小车 USB 供电口、可靠集线器或充电宝：模组供电和扬声器负载。
- 首次联调可把 STC USB 接 PC，使用 `tools/serial_diagnostic.py` 查看帧。

## 首次通电检查

1. 不接 CSK，只接 STC 与 OLED，确认开发板没有异常发热。
2. 烧录固件，OLED 应出现脸部图形，数码管应显示 `L`、光照 ADC 和害怕值。
3. OLED 冷启动失败时重复 3 次；确认线序仍失败则将 `firmware/stc/include/config.h` 的 `PETCARGO_OLED_ENABLED` 改为 0 后重新编译。
4. 再接 CSK 的 TX、RX、GND和独立 USB。
5. 最后把 STC USB 接小车，并验证 `/dev/petcargo_stc`。
