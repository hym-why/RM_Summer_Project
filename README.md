# RM 电控组暑期 Project

本仓库面向 STM32F103C8T6 小车底板，按暑期 project 要求组织代码和文档。仓库已经包含可直接由 CLion/CMake 构建的 CubeMX 工程、HAL/CMSIS 驱动、链接脚本和 `RM.ioc`。

## 当前默认程序

当前 `Core/Inc/project_config.h` 中配置为：

```c
#define PROJECT_APP_MODE APP_MODE_LINE_FOLLOW
```

当前用于循迹验收：上电后自动沿黑线行驶，按一次扩展板按键停止，再按一次重新启动。
基础循迹使用较低直线速度，并在丢失黑线后原地转向搜索，适合胶带弯道验收。
为保证低速下可靠起步，启动时会短暂提高电机输出，随后自动恢复慢速巡航。

如果要验收中间步骤，可以把该宏改成：

```c
APP_MODE_DIGIT_COUNTER
APP_MODE_MOTOR_DIRECTION_TEST
APP_MODE_LINE_SENSOR_TEST
APP_MODE_START_STOP
APP_MODE_LINE_FOLLOW
APP_MODE_PID_LINE_FOLLOW
```

## 进度对应

1. 创建 GitHub 仓库，并把本工程推送上去。
2. `display` 模块：7 位数码管每秒显示 0-9 循环。
3. `motor` + `button` 模块：L298N 驱动两个直流电机，按键切换正反转。
4. `line_follow` 文档和代码：读取 RPR220 循迹模块，设计沿黑线行走策略。
5. [`documents/Car_Is_Ready.png`](documents/Car_Is_Ready.png)：已加入组装完成的实车照片。
6. `app_mode_start_stop`：按键启动直行，再按停止。
7. `app_mode_line_follow`：按键后开始循迹。
8. 已加入弯道自动降速、丢线搜索和蜂鸣提示；S弯最终速度等待实车标定。
9. `pid` 模块：用带微分滤波和抗积分饱和的PID修正左右轮速度；参数集中在 `project_config.h`。

## 硬件要点

从底板原理图可读到：

- 七段数码管段线：`PA0-PA7`。
- 用户按键：`PA15`。
- 蜂鸣器：`PB0`。
- STM32：`STM32F103C8T6`。

L298N 与 RPR220 按下表接线；若实物接线不同，在 `Core/Inc/board_config.h` 中统一修改。

## 推荐接线

L298N：

| L298N | STM32 |
| --- | --- |
| IN1（左轮正转） | PB6/TIM4_CH1 |
| IN2（左轮反转） | PB7/TIM4_CH2 |
| IN3（右轮正转） | PB8/TIM4_CH3 |
| IN4（右轮反转） | PB9/TIM4_CH4 |
| ENA / ENB | 保留常高跳帽 |
| GND | GND |
| +12V/VMS | 电池正极 |
| 5V | 按模块跳帽说明使用 |

左右两个RPR220循迹模块：

| 模块输出 | STM32 |
| --- | --- |
| LEFT | PA11 |
| RIGHT | PA12 |
| VCC | 3.3V |
| GND | GND |

硬件资料中的 LM358 模块在检测到黑线时 `D0` 为高电平，代码已配置为 `GPIO_PIN_SET`。如果实测相反，只需修改 `LINE_BLACK_LEVEL`。

## CLion 使用方式

1. 在 CLion 中直接打开仓库根目录。
2. 在 CMake 配置中设置 `ARM_TOOLCHAIN_DIR`，指向 `arm-none-eabi-gcc.exe` 所在目录；当前开发电脑会自动使用已安装路径。
3. 重新加载 CMake，选择 `RM.elf`，执行 Build。
4. 方形 USB 下载接口使用板载 CH340；无需外接 TX/RX。串口自动下载需要反相后的 DTR/RTS 电平，详见 `documents/acceptance_steps.md`。

## 调试顺序

1. 只烧录数码管模式，确认 `0-9` 每秒循环。
2. 架空车轮测试电机方向，若方向反了，交换该电机两根线或修改对应极性宏。
3. 在黑线和白底上移动循迹模块，观察数码管状态和模块指示灯。
4. 先用低速循迹，确认不会冲出线，再逐步增大 `base_speed`。
5. 最后再调 PID，先调 `kp`，再少量加 `kd`，最后一般不急着加 `ki`；完整记录方法见 [`documents/s_curve_pid_tuning.md`](documents/s_curve_pid_tuning.md)。

## 实车验收边界

- 代码、CubeMX配置、照片和电机演示已经入库。
- RPR220高度、黑白阈值、电机极性和PID最终参数必须以实车场地测量为准，仓库不会把未进行的实车测试标记为通过。

完整验收顺序见 `documents/acceptance_steps.md`。
当前完成情况见 [`documents/test_record.md`](documents/test_record.md)。

## 实车 Demo

- [小车电机与按键演示视频](documents/demo/car_demo.mp4)

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
