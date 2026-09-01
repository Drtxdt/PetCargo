# 2026-09-01 聆思 SDK 同步记录

本次以工作台重新导出的 `mars-sdk` 为唯一真源，未修改其中的音频、命令协议或生成配置。配置包含 `TONE_ID_0–18`，十二条 PetCargo 命令和强光接收协议均通过编译。

输出文件：

- `mars-sdk/dist/PetCargo-CSK5062.bin`：原始应用固件。
- `mars-sdk/dist/PetCargo-CSK5062.img`：经过 CSK3021 镜像封装的 MARS 烧录文件。

SDK 的协议串口为 9600、8N1。为保持链路一致，STC 的 UART1/UART2、ROS 串口节点和诊断工具也已同步为9600；四字节语音命令内容未改变。

构建时直接使用工作台随 SDK 提供的 `tone.h` 和 `tone_buf.h`，没有再次运行音频头生成器，避免覆盖平台已经正确打包的完整语音资源。
