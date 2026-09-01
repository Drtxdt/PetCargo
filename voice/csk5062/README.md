# CSK5062 离线语音配置

适用模组：`EVM-KWS-P-0101-V2.0`（CSK5062）。唤醒词固定为“小爱同学”。

## 平台操作

1. 在聆思官方可视化平台新建离线语音项目，选择与模组一致的 EVM-KWS-P-0101-V2.0 固件目标。
2. 将 `commands.csv` 中的唤醒词、六条命令词和回复录入平台。
3. 将每条命令的串口输出设为 `serial_mapping.json` 中对应的 4 字节十六进制帧。
4. UART1/协议串口设置为 9600、8N1。模组日志 UART0 不连接 STC。
5. 如平台支持“外部串口触发播报”，建立命令 `LIGHT_ALERT`：接收到 `5A A5 80 7F` 时播放强光台词。
6. 编译并通过模组自身 USB 烧录；先在 PC 串口工具中确认输出，再接 STC。

平台工程包含账号侧素材，仓库无法替代平台导出的专有固件。这里的 CSV/JSON 是完整、固定的配置来源；导出后把平台生成的版本号和文件校验值记录到 `firmware-record.txt`。

## 串口帧

CSK → STC：

```text
A5 5A COMMAND (COMMAND XOR FF)
```

STC → CSK：

```text
5A A5 COMMAND (COMMAND XOR FF)
```

固件调试时也接受单独的 `01`～`06` 命令字节，但正式配置应使用四字节帧。

## 接线

- `A11/TX` → STC EXT `P1.0/RXD2`
- `A12/RX` ← STC EXT `P1.1/TXD2`
- `GND` ↔ STC `GND`
- CSK 使用独立 USB 供电；禁止连接 CSK VCC 与 STC VCC。
