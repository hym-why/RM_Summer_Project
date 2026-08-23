# Step 2: 七段数码管 0-9 循环显示

## 作业目标

在外接电源的情况下，点亮扩展板的七段数码管：

- 初始显示 `0`。
- 每过 1 秒，显示数字加 1。
- 显示到 `9` 后溢出回 `0`。
- 该过程持续循环执行。

## 当前实现

相关文件：

- `Core/Src/app_main.c`
- `Core/Src/display.c`
- `Core/Inc/display.h`
- `Core/Inc/board_config.h`

`App_Main()` 当前默认模式为：

```c
AppMode mode = APP_MODE_DIGIT_COUNTER;
```

对应执行函数：

```c
static void RunDigitCounter(void)
{
    uint8_t digit = 0;
    while (1) {
        Display_ShowDigit(digit);
        digit = (uint8_t)((digit + 1) % 10);
        HAL_Delay(1000);
    }
}
```

## CubeMX 配置

根据 `STM32_10B 小车底板原理图`，数码管段线接在 `PA0-PA7`：

| 引脚 | 数码管段 |
| --- | --- |
| PA0 | a |
| PA1 | b |
| PA2 | c |
| PA3 | d |
| PA4 | e |
| PA5 | f |
| PA6 | g |
| PA7 | dp |

在 CubeMX 中将 `PA0-PA7` 配置为：

- `GPIO_Output`
- `Output Push Pull`
- `No pull-up and no pull-down`
- 初始输出高电平

底板数码管为共阳数码管，因此程序中使用低电平点亮段码，高电平熄灭段码。

## main.c 接入方式

在 CubeMX 生成的 `Core/Src/main.c` 中加入：

```c
#include "app_main.h"
```

在初始化完成后调用：

```c
MX_GPIO_Init();
App_Main();
```

`App_Main()` 内部是无限循环，因此 CubeMX 默认的 `while (1)` 可以留空或不会再执行到。

## 验收现象

下载程序后，数码管应按如下顺序显示：

```text
0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 0 -> ...
```

每个数字保持约 1 秒。

## 常见问题

- 完全不亮：检查 `PA0-PA7` 是否配置为输出，开发板是否正确插在扩展板上。
- 数字残缺：检查某个 GPIO 是否配置错误，或数码管段线是否与 `board_config.h` 不一致。
- 亮灭反了：说明实际数码管可能不是共阳，把 `display.c` 中 `GPIO_PIN_RESET` 和 `GPIO_PIN_SET` 的逻辑对调。
- 下载后没变化：确认 `main.c` 已经调用 `App_Main()`。

