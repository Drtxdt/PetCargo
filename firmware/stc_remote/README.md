# PetCargo Windows USB遥控板

使用第二块同型号 STC-B 学习板，无需杜邦线和外接模块。通过板载USB/CH340将导航键方向发送给Windows；电脑再经局域网转发到小车。中心或K3停止，松开发送两次空闲帧。不再使用红外。

运行 `build.bat`，使用 STC-ISP 以 11.0592 MHz 烧录 `build/petcargo_remote.hex`。

数码管显示`C1  A204`一类诊断信息，C后是方向编号，A后是原始ADC。串口为9600/8N1，固定帧为`A5 5A 02 方向 序号 异或校验`。完整操作见`docs/windows-stc-remote.md`。
