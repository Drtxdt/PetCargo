# PetCargo 红外遥控板

使用第二块同型号 STC-B 学习板，无需杜邦线和外接模块。导航键控制方向，中心按下或 K3 停止；松开后发送两次空闲帧。板载 `P3.7/IR_TXD` 经 ULN2003 驱动红外发射管。

运行 `build.bat`，使用 STC-ISP 以 11.0592 MHz 烧录 `build/petcargo_remote.hex`。
