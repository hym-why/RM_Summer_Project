# Project 完成记录与解决方案

## 已由代码覆盖的要求

| 要求 | 状态 | 对应文件 |
| --- | --- | --- |
| 2. 数码管 0-9 循环 | 已完成代码 | `display.c`, `app_main.c` |
| 3. L298N 电机正反转 | 已完成代码 | `motor.c`, `app_main.c` |
| 4. RPR220 循迹思考 | 已完成文档和基础代码 | `line_follow_design.md`, `line_follow.c` |
| 6. 按键启停直行 | 已完成代码 | `button.c`, `motor.c`, `app_main.c` |
| 7. 按键后循迹 | 已完成代码 | `line_follow.c`, `app_main.c` |
| 8. S 形等更多路线 | 已完成自适应降速和丢线搜索代码，待实车跑圈 | `project_config.h`, `app_main.c`, `s_curve_pid_tuning.md` |
| 9. PID 降低抖动 | 已完成PID、微分滤波和抗积分饱和代码，待实车调参 | `pid.c`, `app_main.c` |

## 需要你完成的实物操作

### 1. 组装照片

已使用当前实车照片生成并提交`documents/Car_Is_Ready.png`。照片能够确认两个电机、L298N、电池、电源模块和STM32底板；安装RPR220后应再拍一张能清楚看到循迹探头与无动力前轮的最终验收照片。

### 2. 推送到 GitHub

当前远程仓库：

```text
https://github.com/hym-why/RM_Summer_Project.git
```

推送命令：

```bash
git push origin main
```

如果提示登录，请按 GitHub 弹窗完成授权。

### 3. 实车调试顺序

不要直接高速跑 PID，按下面顺序验收：

1. `APP_MODE_DIGIT_COUNTER`：数码管 0-9 每秒循环。
2. `APP_MODE_MOTOR_DIRECTION_TEST`：两个电机同向转，按键后反向。
3. `APP_MODE_START_STOP`：按键启动直行，再按停止。
4. `APP_MODE_LINE_FOLLOW`：低速基础循迹。
5. `APP_MODE_PID_LINE_FOLLOW`：最终 PID 循迹。

模式切换位置：

```c
#define PROJECT_APP_MODE APP_MODE_PID_LINE_FOLLOW
```

文件：

```text
Core/Inc/project_config.h
```

## PID 调参建议

先保持：

```c
#define PID_KI 0.0f
```

调参顺序：

1. 只调 `PID_KP`，从 `180` 到 `350` 试。
2. 如果左右摆动明显，小幅增加 `PID_KD`，从当前 `0.8` 开始每次增加 `0.1-0.2`。
3. 如果直线偏一侧，优先检查机械和电机方向，不急着加 `KI`。
4. 如果速度太快容易飞线，先降低 `PID_LINE_BASE_SPEED`。

## 常见现象处理

| 现象 | 处理 |
| --- | --- |
| 电机方向反 | 交换该电机两根线，或把对应速度取负 |
| 按键没反应 | CubeMX 中 SYS Debug 设为 Serial Wire，释放 PA15 |
| 黑白判断反 | 修改 `LINE_BLACK_LEVEL` |
| 小车左右疯狂摆 | 降低 `PID_KP` 或增加少量 `PID_KD` |
| 弯道冲出去 | 降低基础速度，增大转向修正 |
| 两个轮子速度差很多 | 调整机械安装，必要时给某一边速度乘修正系数 |

## 可选任务方案

### RTOS

可选引入 FreeRTOS，把程序分为：

- `ButtonTask`：按键扫描和状态切换。
- `LineSensorTask`：循迹传感器读取。
- `MotorTask`：根据目标速度刷新 PWM。
- `DebugTask`：蜂鸣器和数码管提示。

当前 project 主线不依赖 RTOS，先完成循迹更稳。

### 红外遥控

扩展板上有红外接收头，底板原理图中连接到 `PA8`。可选方案：

- 配置 `PA8` 输入捕获或外部中断。
- 解码 NEC 红外协议。
- 手机红外遥控器发送前进、停止、左转、右转命令。

这是附加功能，不影响主线验收。
