# 固件构建记录（2026-09-03）

状态：本机构建与自动检查通过，**待实机验证**。没有连接开发板验证扫描、串口电气信号或停止响应时间。

## 源码身份

- 基础提交：`2612cca2e38c3127b709befd196117e4ee8e57cb`
- 本次交付包含未提交的工作区修改，并非上述提交的原始构建。
- 固件源码集合 SHA256：`71b303eb3922dc04d37c07af7c5393cb0e9b605b9c93f3189ad9481a5893f932`

源码集合包括 `firmware/stc/include/*.h`、`firmware/stc/src/*.c`、主板 `main.c`、`diagnostic.c`、`build.bat`、`Makefile` 和根目录 `build.bat`。按完整路径排序，将每个文件写成“工作区相对路径（正斜杠）:小写文件 SHA256”，以 LF 连接、不添加末尾换行，对 UTF-8 文本求 SHA256。

## 产物

| 文件 | 代码地址高水位（字节） | XRAM（字节） | 链接器预留栈（字节） |
|---|---:|---:|---:|
| firmware/stc/build/petcargo.hex | 22229 | 1429 | 168 |
| firmware/stc/build/petcargo_diagnostic.hex | 12096 | 1013 | 181 |
| firmware/stc_remote/build/petcargo_remote.hex | 2307 | 62 | 223 |

预留栈是链接布局检查，不是实机最坏情况下栈深度测量。

SHA256：

```text
petcargo.hex
96fb17470f6b7097352c8b3e39fd634cdcbbc98d2c7aa2def9d80eefe3fde272
petcargo_diagnostic.hex
2e6ac4510bdad077a08c30c067d63675ffa024ecaeedd396fa03e284d55f8a59
petcargo_remote.hex
9cbe75212e52fdf912b8e0901dec584b58530693bbcc1c4c89e095a9391aef4c
```

## 已完成检查

- `build.bat all`：主板、诊断、遥控板分别编译链接成功，每个 C 文件独立编译。
- `python tools/verify_firmware.py`：HEX 校验和、链接中断向量、内存余量、扫描汇编与引脚配置通过。
- `python tools/test_firmware.py`：实际固件 C 逻辑的原生测试通过，覆盖按键、语音解析、发送队列、音乐和主板事件处理；外设使用模拟实现，不替代硬件验证。
- `python -m unittest discover -s tests -v`：原有 14 项测试通过。
- 本轮没有修改 `mars-sdk` 或比赛仓库；遥控板源码未改，仅构建回归。

## 实机交付门槛

按 [独立诊断与板上验收](bench-test.md) 先烧录诊断固件。STC-ISP 设置 **11.0592 MHz**，**P5.4 为普通 I/O**。

显示连续运行、按键各 20 次、UART2 原始四字节、音乐打断、K3 停止时延及 OLED 拔除测试均待实机完成。只有收到测试结果后才能确认硬件故障已经排除。
