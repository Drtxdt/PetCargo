# Windows + STC 局域网遥控

## 链路

```text
导航键 -> 遥控STC -> 板载CH340/USB -> Windows串口桥
       -> HTTP /api/jog -> 车载ROS motion_executor -> safety_gateway -> /cmd_vel
```

遥控板不再使用红外。USB同时供电和提供串口，无需杜邦线。电脑和小车必须在同一局域网，网页`http://小车IP:8080/`必须能从电脑打开。

## 串口协议

固定6字节：`A5 5A 02 DIRECTION SEQUENCE XOR`，最后一字节是前5字节逐字节异或。方向0松开、1前、2后、3左、4右、5停止。方向/停止按住时每100ms发送；松开发送两帧0。9600 baud、8N1。

数码管显示`C1  A204`：C后为方向编号，A后为导航键原始ADC。ADC读取失败显示`C0  E001`并发送松开。LED显示方向。

## 部署

1. 烧录`firmware/stc_remote/build/petcargo_remote.hex`，时钟11.0592MHz。
2. 在仓库根目录执行以下命令部署新ROS接口（地址换成小车实际IP），随后按脚本提示启动PetCargo：

```powershell
.\tools\deploy_to_robot.ps1 -Robot ucar@192.168.1.20
```
3. 在Windows安装依赖：`python -m pip install pyserial`。
4. 用USB连接遥控STC。若只有一个CH340，可自动识别：

```bat
tools\run_remote.bat http://192.168.1.20:8080
```

多个CH340时明确指定端口：

```bat
tools\run_remote.bat http://192.168.1.20:8080 COM7
```

IP和COM口换成实际值。控制台出现`seq=... direction=... ok`代表串口帧已解析且小车HTTP接口返回成功；这不等于底盘一定执行，仍需查看网页安全状态。

## 安全行为

- 小车端仅接受方向0-4、速度60-180mm/s、租约100-500ms；Windows桥固定使用120mm/s和300ms。
- 松开、中心/K3、Windows程序退出或串口300ms没有有效帧时，桥请求停止。
- 网络请求失败时不伪装成功；小车上的ROS租约在300ms后自行归零，不依赖Windows成功补发停止。
- HTTP短暂阻塞导致串口积压时，Windows只转发最新状态，禁止旧方向越过已经排队的松开帧。
- 锁定急停、里程计异常和ROS安全网关仍具有更高优先级。
- `/api/jog`与原网页一样没有身份认证，只应在可信教室局域网使用，不要暴露到公网。

## 排查顺序

1. 遥控板显示不随按键变化：看A后的ADC，属于板端采样问题。
2. 显示正常但Windows没有`seq`：检查设备管理器COM口、CH340驱动和端口占用。
3. 有`seq`但提示网络错误：确认小车IP、8080端口、新代码已部署且ROS已启动。
4. HTTP显示`ok`但车不动：检查网页是否急停、里程计是否有效及`motion_executor`日志。
5. 松开测试必须确认300ms内归零；通过后才进行连续方向测试。
