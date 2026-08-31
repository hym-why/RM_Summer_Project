# RM 电控组暑期 Project

本仓库面向 STM32F103C8T6 小车底板，按暑期 project 要求组织代码和文档。

## 当前默认程序

当前 `Core/Inc/project_config.h` 中配置为：

```c
#define PROJECT_APP_MODE APP_MODE_PID_LINE_FOLLOW
```

也就是最终版：按下扩展板用户按键后，小车开始 PID 循迹；再次按下按键后停止。

如果要验收中间步骤，可以把该宏改成：

```c
APP_MODE_DIGIT_COUNTER
APP_MODE_MOTOR_DIRECTION_TEST
APP_MODE_START_STOP
APP_MODE_LINE_FOLLOW
APP_MODE_PID_LINE_FOLLOW
```

## 进度对应

1. 创建 GitHub 仓库，并把本工程推送上去。
2. `display` 模块：7 位数码管每秒显示 0-9 循环。
3. `motor` + `button` 模块：L298N 驱动两个直流电机，按键切换正反转。
4. `line_follow` 文档和代码：读取 RPR220 循迹模块，设计沿黑线行走策略。
5. `documents/Car_Is_Ready.png`：你已经完成组装后，把实物照片放到这里。
6. `app_mode_start_stop`：按键启动直行，再按停止。
7. `app_mode_line_follow`：按键后开始循迹。
8. 可继续调 S 弯等线路。
9. `pid` 模块：用 PID 修正左右轮速度，降低抖动；参数集中在 `project_config.h`。

## 硬件要点

从底板原理图可读到：

- 七段数码管段线：`PA0-PA7`。
- 用户按键：`PA15`。
- 蜂鸣器：`PB0`。
- STM32：`STM32F103C8T6`。

L298N 与 RPR220 的实际 IO 请按你的接线修改 `Core/Inc/board_config.h`。建议先按 README 里的默认接线接，调通后再整理线束。

## 推荐接线

L298N：

| L298N | STM32 |
| --- | --- |
| IN1 | PB12 |
| IN2 | PB13 |
| IN3 | PB14 |
| IN4 | PB15 |
| ENA | PB6/TIM4_CH1 |
| ENB | PB7/TIM4_CH2 |
| GND | GND |
| +12V/VMS | 电池正极 |
| 5V | 按模块跳帽说明使用 |

RPR220 三/四路循迹模块：

| 模块输出 | STM32 |
| --- | --- |
| LEFT | PA11 |
| RIGHT | PA12 |
| VCC | 3.3V 或 5V，按模块说明 |
| GND | GND |

如果你的循迹模块有 4 路或 5 路，把多出来的输出接到空闲 IO，并在 `line_follow.c` 中扩展权重。

## CLion 使用方式

1. 用 STM32CubeMX 新建 `STM32F103C8T6` 工程，Toolchain/IDE 选择 `CMake` 或 `STM32CubeIDE` 后导出。
2. 把本仓库 `Core/Inc` 和 `Core/Src` 中同名用户代码复制/合并到 CubeMX 工程。
3. 在 CubeMX 中配置：
   - `PA0-PA7`：GPIO Output，推挽输出。
   - `PA15`：GPIO Input，Pull-up。
   - `PB0`：GPIO Output。
   - `PB12-PB15`：GPIO Output。
   - `PB6/PB7`：TIM4 PWM 输出。
   - `PA11/PA12`：GPIO Input。
4. 在 CLion 打开工程，编译下载。

## 调试顺序

1. 只烧录数码管模式，确认 `0-9` 每秒循环。
2. 架空车轮测试电机方向，若方向反了，交换电机两根线或改 `MOTOR_DIR_FORWARD`。
3. 用手按循迹模块下方黑/白区域，观察 `LineSensor_Read()` 的状态。
4. 先用低速循迹，确认不会冲出线，再逐步增大 `base_speed`。
5. 最后再调 PID，先调 `kp`，再少量加 `kd`，最后一般不急着加 `ki`。

## 不能由代码直接完成的内容

- GitHub 仓库创建与登录授权：需要你在浏览器中完成账号登录，并把本地提交 push 到远程。
- `documents/Car_Is_Ready.png`：需要你用手机拍摄已经组装好的小车，并放入该路径。
- 实车参数调试：电机方向、黑白电平、循迹速度和 PID 参数必须按你的真实硬件微调。

## Git 提交建议

按作业要求每一步都提交一次：

```bash
git add .
git commit -m "step2: display digit counter"
git commit -m "step3: drive motors with button direction toggle"
git commit -m "step4: add line sensor design"
git commit -m "step5: add assembled car photo"
git commit -m "step6: add button start stop driving"
git commit -m "step7: add line following mode"
git commit -m "step9: add pid correction"
git push
```
