# PetCargo Keil/BSP 课程版

本目录是与实机 SDCC 固件并行保存的 Keil C51 课程版。它直接链接老师提供的
`vendor/STCBSP_V3.6.LIB`，所有库已支持的板载外设均通过 BSP API 使用。

## 版本边界

- 实机继续烧录 `firmware/stc/build/petcargo.hex`、诊断版 HEX 和
  `firmware/stc_remote/build/petcargo_remote.hex`。
- 本目录用于 Keil 编译检查和课程源码审阅，不会复制或覆盖上述文件。
- `main_board` 保留 PetCargo 状态、光照触发、语音指令、协议 v2 和本地交互结构。
- `remote_board` 使用 BSP 导航键和 UART1，向 Windows 遥控桥发送现有六字节帧。
- OLED、ADXL345 和已放弃的红外链路不属于课程版构建门槛。

## 构建

在仓库根目录运行：

```bat
build_keil.bat
build_keil.bat main
build_keil.bat remote
```

脚本优先使用环境变量 `KEIL_C51_BIN`，否则检测 `D:\keil\C51\BIN` 等标准位置。
输出只会写入：

- `firmware/keil/main_board/build_keil/PetCargo_Main_Keil.hex`
- `firmware/keil/remote_board/build_keil/PetCargo_Remote_Keil.hex`

两份 `clean.bat` 也只会删除各自的 `build_keil` 目录。

## BSP 约束

两个入口均先注册 `SetEventCallBack`，再调用 `MySTC_Init()`，永久循环中仅调用
`MySTC_OS()`。应用代码不声明中断，不直接接管 BSP 使用的定时器或串口。

`vendor/` 是从老师的 `STC_B_SDCC_Exam_Template_v2/keil-src` 复制的构建依赖，
便于提交后在另一台安装 Keil C51 的电脑上直接编译。
