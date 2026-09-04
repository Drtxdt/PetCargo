# PetCargo 通信接口 v2

## STC ↔ ROS 帧

`AA 55 | VERSION | TYPE | SEQ | LENGTH | PAYLOAD | CRC16_LO CRC16_HI`

- 版本固定为 2，最大负载32字节，多字节字段使用小端。
- CRC16-CCITT 初值 `FFFF`、多项式 `1021`，覆盖 VERSION 至 PAYLOAD。
- v1 与 v2 不兼容；任一端收到错误版本只回复错误，不执行动作。

| 类型 | 值 | 方向 | 负载 |
|---|---:|---|---|
| HELLO | `01` | 双向 | `capabilities:u16` |
| HEARTBEAT | `02` | 双向 | `uptime_ms:u32` |
| TELEMETRY | `10` | STC→ROS | 20字节，见下表 |
| EVENT | `11` | STC→ROS | `event:u8, value:i16` |
| MOTION_REQUEST | `20` | STC→ROS | `id:u16, kind:u8, distance_mm:i16, angle_cdeg:i32, max_speed:u16` |
| MOTION_RESULT | `21` | ROS→STC | `id:u16, code:u8, actual_mm:i16, actual_cdeg:i32` |
| JOG_REQUEST | `22` | STC→ROS | `direction:u8, speed_mm_s:u16, lease_ms:u16` |
| STOP | `31` | 双向 | `code:u8`：0解除锁定、1锁定、2只取消当前动作 |
| ACK/NACK | `7E/7F` | 双向 | `ref_type:u8, ref_seq:u8, code:u8` |

### TELEMETRY 20字节

| 偏移 | 字段 | 类型 |
|---:|---|---|
| 0 | uptime_ms | u32 |
| 4 | light_raw | u8 |
| 5 | temp_x10 | i16 |
| 7/9/11 | accel_x/y/z_mg | i16×3 |
| 13 | vibration | u8 |
| 14 | hall | u8 |
| 15/16 | happiness/fear | u8×2 |
| 17 | flags | u8 |
| 18 | feed_count | u16 |

Flags：bit0睡眠、bit1急停、bit4传感器故障、bit5主板红外输入（兼容保留；Windows遥控状态来自运动节点）。

### 动作语义

- `kind=1`：沿车头方向移动，正值前进、负值后退。
- `kind=2`：原地旋转，角度单位为厘度。
- `kind=3`：横向移动，正值向左、负值向右。
- `direction=0/1/2/3/4`：停止/前/后/左/右点动。
- 固定动作目标为500毫米；STC遥控点动速度120毫米每秒，租约300毫秒。
- 动作结果新增 `6=LIDAR_UNAVAILABLE`、`7=OBSTACLE_BLOCKED`；负载格式和协议版本不变。

## CSK ↔ STC

CSK命令帧为 `A5 5A COMMAND (COMMAND XOR FF)`；STC播报触发帧为 `5A A5 COMMAND (COMMAND XOR FF)`。完整录入表见 `voice/csk5062/commands.md`。

## ROS话题与网页API

| 话题 | 类型 | 说明 |
|---|---|---|
| `/petcargo/telemetry` | std_msgs/String | JSON遥测 |
| `/petcargo/events` | std_msgs/String | JSON事件 |
| `/petcargo/motion_request` | std_msgs/String | 固定距离动作 |
| `/petcargo/jog_request` | std_msgs/String | STC点动与租约，source区分主板和Windows桥 |
| `/petcargo/motion_result` | std_msgs/String | 动作结果 |
| `/petcargo/lidar_status` | std_msgs/String | 雷达在线、净空、避障阶段与选边状态 |
| `/petcargo/cmd_vel_request` | geometry_msgs/Twist | 安全网内部速度 |
| `/petcargo/safety_set` | std_msgs/Bool | 锁定或解除急停 |
| `/petcargo/cancel_motion` | std_msgs/Empty | 只取消当前动作 |
| `/petcargo/serial_connected` | std_msgs/Bool | STC链路状态 |
| `/cmd_vel` | geometry_msgs/Twist | 仅safety_gateway发布 |

Windows遥控通过车载网页服务`POST /api/jog`进入上述话题。JSON字段为`direction`（0停止、1前、2后、3左、4右）、`speed_mm_s`（60–180）和`lease_ms`（100–500）；服务端固定标记`source=windows_stc`。接口只用于可信局域网。

- `GET /api/state`
- `GET /api/events`
- `POST /api/stop`，负载为 `{"engaged":true}` 或 `false`
- `POST /api/jog`，负载为方向、速度和租约
