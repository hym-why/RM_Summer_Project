# 暑期 Project 验收步骤

## 安全准备

1. 首次测试电机时架空两个驱动轮，并断开或固定可能卷入轮子的导线。
2. L298N、电池、底板必须共地。不要同时用多个电源向同一条 5V/3.3V 电源轨供电。
3. 每次只验收一个模式，修改 `Core/Inc/project_config.h` 的 `PROJECT_APP_MODE` 后重新 Build 和烧录。

## 模式与现象

| 要求 | 模式 | 预期现象 |
| --- | --- | --- |
| 2 | `APP_MODE_DIGIT_COUNTER` | 数码管每秒循环 0-9 |
| 3 | `APP_MODE_MOTOR_DIRECTION_TEST` | 上电两轮转动；按扩展板用户键后停 150 ms 并反转；可重复 |
| 6 | `APP_MODE_START_STOP` | 初始停止；按键直行并显示 1；再按停止并显示 0 |
| 7 | `APP_MODE_LINE_FOLLOW` | 初始停止；按键后使用固定修正量循迹；再按停止 |
| 9 | `APP_MODE_PID_LINE_FOLLOW` | 初始停止；按键后使用 PID 循迹；再按停止 |

循迹模式下数码管状态：`0` 表示左传感器检测黑线，`1` 表示两侧同时检测黑线，`2` 表示右传感器检测黑线，`8` 表示两侧都未检测到黑线并进入寻线。

## L298N 接线

| L298N | STM32 |
| --- | --- |
| IN1 / IN2 | PB12 / PB13 |
| IN3 / IN4 | PB14 / PB15 |
| ENA / ENB | PB6 / PB7 |
| GND | GND |

使用 PB6/PB7 PWM 调速时，取下 L298N 的 ENA、ENB 常高跳帽。电机转向相反时，优先交换该电机的两根输出线；也可把 `MOTOR_LEFT_POLARITY` 或 `MOTOR_RIGHT_POLARITY` 设为 `-1`。

## RPR220 接线与标定

| 左模块 | STM32 |
| --- | --- |
| D0 | PA11 |
| VCC | 3.3V |
| GND | GND |

右模块的 D0 接 PA12，VCC/GND 相同。把探头放在白底和黑线之间反复移动，旋转电位器，直到指示灯能稳定切换。代码按“黑线 D0 高电平”配置。

## CH340 自动烧录

底板方形 USB 下载接口已经内部连接 USART1，不需要外接 TX/RX。Windows 识别为 `USB-SERIAL CH340` 后，可使用：

```powershell
stm32loader -p COM13 -b 115200 -f F1 -R -B -e -w -v -g 0x08000000 cmake-build-debug/RM.bin
```

COM 号以设备管理器实际显示为准。`-R -B` 用于匹配底板上反相的自动复位和 BOOT0 电路。

## PID 调参顺序

1. 先把车速保持较低，`PID_KI` 保持 0。
2. 增加 `PID_KP`，直到能明显纠偏但尚未连续左右摆动。
3. 小幅增加 `PID_KD` 抑制摆动。
4. S 弯冲线时先降低 `PID_LINE_BASE_SPEED`，再微调 `PID_KP/PID_KD`。
5. 每组参数至少完整跑一圈，并记录在 `documents/test_record_template.md`。

## 必须由本人完成

拍摄能看清整车、STM32底板、L298N、两个电机、电池、前轮、循迹模块和线束的照片，命名为 `Car_Is_Ready.png`，放入 `documents/` 后提交到 GitHub。实车方向、电机差异、传感器高度和 PID 参数必须现场调试。
