# 故障排查

## `/dev/petcargo_stc` 不存在

1. `lsusb` 检查是否出现 `1a86:7523` CH340。
2. 执行一次 `sudo tools/install_udev_rule.sh /home/ucar/PetCargo`。
3. 拔插 STC USB，运行 `ls -l /dev/petcargo_stc`。
4. 确认当前用户属于 `dialout` 组；修改组后需要重新登录。

## 网页显示 STC 离线

1. `rosnode list` 应有 `serial_bridge`、`motion_executor`、`safety_gateway`、`dashboard_server`。
2. `python3 tools/serial_diagnostic.py /dev/petcargo_stc` 应持续看到 TELEMETRY。
3. CSK、STC 与 ROS 串口均必须为 9600、8N1。
4. 若 CRC 错误持续增长，先缩短 USB/杜邦线并断开 CSK，只保留 STC USB 排查。

## CSK 能说话但 STC 无动作

1. 共地是否连接。
2. A11/TX 是否接 P1.0/RXD2，A12/RX 是否接 P1.1/TXD2。
3. 平台串口是否为协议 UART，而不是 UART0 日志口。
4. 用 USB 串口工具确认命令“后退”输出 `A5 5A 02 FD`。

## OLED 不亮

1. 确认 OLED 已在原 P1.0/P1.1 测试程序中点亮过。
2. 检查 SCL→SM S1、SDA→SM S2，不要按普通 GPIO 的非反相逻辑理解。
3. 重插并冷启动三次。
4. 仍不稳定则在 `config.h` 关闭 OLED，使用数码管、LED 和蜂鸣器降级，不影响主链路。

## 小车不动

1. 网页安全锁必须为“未锁定”，STC 必须在线。
2. `rostopic echo /odom` 必须持续更新。
3. `rostopic echo /petcargo/motion_request` 检查 STC 是否发出请求。
4. `rostopic info /cmd_vel` 确认 safety_gateway 是唯一 PetCargo 发布者。
5. 不要同时运行比赛流程。

## 小车后退距离偏差大

- 检查轮胎打滑和地面；先保持最大速度 0.18 m/s。
- 查看动作结果中的 `actual_mm`，区分里程计误差和动作未完成。
- 若超时，先确认 `/odom` 时间戳和更新率，再调整机械问题，不提高速度。
