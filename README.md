# PetCargo

PetCargo 是一个基于 STC-B 学习板、CSK5062 离线语音模组和 uCar ROS 底盘的傲娇型电子宠物。项目重点展示 STC 的光敏、温度、震动、霍尔、加速度、按键、OLED、数码管、LED、蜂鸣器、EEPROM、RTC 和双串口能力。

核心演示链路：

1. 手电筒照射光敏传感器。
2. STC 更新快乐/恐惧情绪并输出 OLED、LED、数码管和蜂鸣器反馈。
3. STC 通过 USB 串口请求小车低速后退 0.5 m。
4. ROS 使用里程计闭环执行动作，网页同步显示状态并提供急停。

## 仓库结构

- `firmware/stc`：车载主板 SDCC 固件。
- `firmware/stc_remote`：第二块 STC 红外遥控板固件。
- `voice/csk5062`：离线命令词、回复和串口映射。
- `ros/petcargo_ros`：ROS Noetic 包。
- `dashboard`：无第三方前端依赖的网页看板。
- `tools`：部署、运行、诊断和模拟工具。
- `docs`：接线、协议、测试和演示文档。

## 快速开始

### 1. 编译 STC 固件

在 Windows 上运行：

```powershell
cd firmware\stc
.\build.bat
```

将 `firmware/stc/build/petcargo.hex` 烧录到 STC，时钟选择 11.0592 MHz。

第二块板运行 `firmware/stc_remote/build.bat`，烧录生成的 `petcargo_remote.hex`。

### 2. 部署到小车

```powershell
.\tools\deploy_to_robot.ps1 -Robot ucar@192.168.1.10
```

比赛仓库不会被修改。PetCargo 使用独立的 `/home/ucar/petcargo_ws`，并将比赛工作空间作为只读 underlay。

### 3. 启动

```bash
/home/ucar/PetCargo/tools/run_robot.sh
```

浏览器打开 `http://小车IP:8080`。正式演示前必须阅读 `docs/wiring.md` 和 `docs/demo_script.md`。

## 安全约束

- CSK5062 独立 USB 供电，禁止把 CSK 的 VCC 接到 STC EXT VCC。
- CSK 与 STC 只连接交叉 TX/RX 和 GND。
- 四向运动演示前清空周围至少 1 m 通道。
- K3和网页急停会锁定底盘；语音“停下来”、红外松开和串口断开会立即归零。
- 不启动比赛任务节点；PetCargo 的 `safety_gateway` 是唯一 `/cmd_vel` 发布者。
