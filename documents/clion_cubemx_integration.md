# CLion / CubeMX 集成步骤

## 1. CubeMX 配置

芯片选择 `STM32F103C8T6`。

时钟可以先用默认 HSI，也可以配置外部 8 MHz 晶振。新人调车阶段，先让 GPIO/PWM 跑起来更重要。

GPIO：

| 引脚 | 模式 | 说明 |
| --- | --- | --- |
| PA0-PA7 | GPIO_Output | 数码管 a,b,c,d,e,f,g,dp，默认高电平熄灭 |
| PA15 | GPIO_Input + Pull-up | 用户按键，按下为低 |
| PB0 | GPIO_Output | 蜂鸣器 |
| PB12-PB15 | GPIO_Output | L298N 四个方向输入 |
| PA11/PA12 | GPIO_Input | 左/右循迹模块输出 |

PWM：

| 引脚 | 定时器 | 说明 |
| --- | --- | --- |
| PB6 | TIM4_CH1 PWM | 左电机 ENA |
| PB7 | TIM4_CH2 PWM | 右电机 ENB |

TIM4 推荐：

- Prescaler：`71`
- Counter Period：`999`
- PWM 频率约 1 kHz
- Pulse 初始值：`0`

注意：`PA15/PB3/PB4` 默认可能被 JTAG 占用。CubeMX 中把 Debug 设为 `Serial Wire`，释放 PA15 作为普通 IO。

## 2. main.c 调用

在 CubeMX 生成的 `Core/Src/main.c` 里添加：

```c
#include "app_main.h"
```

然后在初始化完成后调用：

```c
MX_GPIO_Init();
MX_TIM4_Init();
App_Main();
```

`App_Main()` 内部是无限循环，因此可以替代 CubeMX 默认的 `while (1)` 内容。

## 3. 模式切换

打开 `Core/Src/app_main.c`，修改：

```c
AppMode mode = APP_MODE_PID_LINE_FOLLOW;
```

可选值：

```c
APP_MODE_DIGIT_COUNTER
APP_MODE_MOTOR_DIRECTION_TEST
APP_MODE_START_STOP
APP_MODE_LINE_FOLLOW
APP_MODE_PID_LINE_FOLLOW
```

建议每完成一个模式就提交一次 git，对应作业步骤。

## 4. 常见问题

- 数码管显示反了：检查是否共阳。如果是共阴，把 `display.c` 里的高低电平逻辑反过来。
- 按键无反应：确认 PA15 已从 JTAG 释放，并启用上拉。
- 电机只响不转：先架空轮子，提高 PWM 到 700；再检查 L298N ENA/ENB 跳帽或 PWM 线。
- 车往后走：交换电机线，或在 `Motor_SetBoth()` 调用处把速度正负反过来。
- 循迹方向反：交换左右传感器线，或把 `line_follow.c` 中 error 的正负调换。
- 黑白识别反：修改 `board_config.h` 的 `LINE_BLACK_LEVEL`。

