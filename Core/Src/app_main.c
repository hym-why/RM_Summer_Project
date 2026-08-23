#include <stdbool.h>
#include "main.h"
#include "board_config.h"
#include "button.h"
#include "display.h"
#include "line_follow.h"
#include "motor.h"
#include "pid.h"

extern TIM_HandleTypeDef MOTOR_PWM_TIMER;

typedef enum {
    APP_MODE_DIGIT_COUNTER = 0,
    APP_MODE_MOTOR_DIRECTION_TEST,
    APP_MODE_START_STOP,
    APP_MODE_LINE_FOLLOW,
    APP_MODE_PID_LINE_FOLLOW,
} AppMode;

static void Beep(uint32_t ms)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
    HAL_Delay(ms);
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

static void RunDigitCounter(void)
{
    uint8_t digit = 0;
    while (1) {
        Display_ShowDigit(digit);
        digit = (uint8_t)((digit + 1) % 10);
        HAL_Delay(1000);
    }
}

static void RunMotorDirectionTest(void)
{
    Button key;
    Button_Init(&key);
    bool forward = true;
    Motor_SetBoth(550, 550);

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            forward = !forward;
            Motor_SetBoth(forward ? 550 : -550, forward ? 550 : -550);
            Beep(40);
        }
    }
}

static void RunStartStop(void)
{
    Button key;
    Button_Init(&key);
    bool running = false;
    Motor_Stop();

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            running = !running;
            if (running) {
                Motor_SetBoth(500, 500);
                Display_ShowDigit(1);
            } else {
                Motor_Stop();
                Display_ShowDigit(0);
            }
        }
    }
}

static void RunLineFollow(void)
{
    Button key;
    Button_Init(&key);
    bool running = false;
    const int16_t base_speed = 430;

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            running = !running;
            if (!running) {
                Motor_Stop();
                Display_ShowDigit(0);
            }
        }

        if (!running) {
            continue;
        }

        LineSensorState line = LineSensor_Read();
        if (line.lost) {
            Motor_SetBoth(220, 220);
            Beep(15);
            Display_ShowDigit(8);
            HAL_Delay(40);
            continue;
        }

        int16_t turn = LineFollow_ComputeTurn(line);
        Motor_SetBoth((int16_t)(base_speed + turn), (int16_t)(base_speed - turn));
        Display_ShowDigit((uint8_t)(line.error + 1));
        HAL_Delay(10);
    }
}

static void RunPidLineFollow(void)
{
    Button key;
    Button_Init(&key);
    PidController pid;
    PID_Init(&pid, 260.0f, 0.0f, 45.0f, 380.0f);

    bool running = false;
    uint32_t last_tick = HAL_GetTick();
    const int16_t base_speed = 560;

    while (1) {
        uint32_t now = HAL_GetTick();
        if (Button_UpdatePressedEvent(&key, now)) {
            running = !running;
            PID_Reset(&pid);
            last_tick = now;
            if (!running) {
                Motor_Stop();
            }
        }

        if (!running) {
            continue;
        }

        LineSensorState line = LineSensor_Read();
        float dt = (float)(now - last_tick) / 1000.0f;
        last_tick = now;

        if (line.lost) {
            Motor_SetBoth(260, 260);
            HAL_Delay(20);
            continue;
        }

        float turn = PID_Update(&pid, (float)line.error, dt);
        Motor_SetBoth((int16_t)(base_speed + turn), (int16_t)(base_speed - turn));
        HAL_Delay(5);
    }
}

void App_Main(void)
{
    /* Change this mode as your project progresses, then commit each step. */
    AppMode mode = APP_MODE_DIGIT_COUNTER;

    if (mode != APP_MODE_DIGIT_COUNTER) {
        Motor_Init(&MOTOR_PWM_TIMER);
    }

    switch (mode) {
    case APP_MODE_DIGIT_COUNTER:
        RunDigitCounter();
        break;
    case APP_MODE_MOTOR_DIRECTION_TEST:
        RunMotorDirectionTest();
        break;
    case APP_MODE_START_STOP:
        RunStartStop();
        break;
    case APP_MODE_LINE_FOLLOW:
        RunLineFollow();
        break;
    case APP_MODE_PID_LINE_FOLLOW:
    default:
        RunPidLineFollow();
        break;
    }
}
