#include <stdbool.h>
#include "app_main.h"
#include "board_config.h"
#include "button.h"
#include "display.h"
#include "line_follow.h"
#include "motor.h"
#include "pid.h"
#include "project_config.h"
#include "tim.h"

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
        digit = (uint8_t)((digit + 1u) % 10u);
        HAL_Delay(1000);
    }
}

static void RunMotorDirectionTest(void)
{
    Button key;
    bool forward = true;

    Button_Init(&key);
    Display_ShowDigit(1);
    Motor_SetBoth(MOTOR_TEST_SPEED, MOTOR_TEST_SPEED);

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            Motor_Stop();
            HAL_Delay(MOTOR_REVERSE_DELAY_MS);
            forward = !forward;
            Motor_SetBoth(forward ? MOTOR_TEST_SPEED : -MOTOR_TEST_SPEED,
                          forward ? MOTOR_TEST_SPEED : -MOTOR_TEST_SPEED);
            Display_ShowDigit(forward ? 1 : 2);
            Beep(40);
        }
        HAL_Delay(1);
    }
}

static void RunStartStop(void)
{
    Button key;
    bool running = false;

    Button_Init(&key);
    Motor_Stop();
    Display_ShowDigit(0);

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            running = !running;
            if (running) {
                Motor_SetBoth(STRAIGHT_DRIVE_SPEED, STRAIGHT_DRIVE_SPEED);
                Display_ShowDigit(1);
            } else {
                Motor_Stop();
                Display_ShowDigit(0);
            }
        }
        HAL_Delay(1);
    }
}

static void RunLineFollow(void)
{
    Button key;
    bool running = false;

    Button_Init(&key);
    LineSensor_Reset();
    Motor_Stop();
    Display_ShowDigit(0);

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            running = !running;
            LineSensor_Reset();
            if (!running) {
                Motor_Stop();
                Display_ShowDigit(0);
            } else {
                Display_ShowDigit(1);
            }
        }

        if (running) {
            LineSensorState line = LineSensor_Read();
            if (line.lost) {
                int16_t turn = line.last_valid_error < 0 ? -LINE_OPEN_LOOP_TURN : LINE_OPEN_LOOP_TURN;
                Motor_SetBoth((int16_t)(LINE_LOST_SEARCH_SPEED + turn),
                              (int16_t)(LINE_LOST_SEARCH_SPEED - turn));
                Display_ShowDigit(8);
            } else {
                int16_t turn = LineFollow_ComputeTurn(line);
                Motor_SetBoth((int16_t)(LINE_BASE_SPEED + turn),
                              (int16_t)(LINE_BASE_SPEED - turn));
                Display_ShowDigit((uint8_t)(line.error + 1));
            }
            HAL_Delay(LINE_CONTROL_PERIOD_MS);
        } else {
            HAL_Delay(1);
        }
    }
}

static void RunPidLineFollow(void)
{
    Button key;
    PidController pid;
    bool running = false;
    uint32_t last_control_ms = HAL_GetTick();

    Button_Init(&key);
    PID_Init(&pid, PID_KP, PID_KI, PID_KD, PID_OUTPUT_LIMIT);
    LineSensor_Reset();
    Motor_Stop();
    Display_ShowDigit(0);

    while (1) {
        uint32_t now = HAL_GetTick();

        if (Button_UpdatePressedEvent(&key, now)) {
            running = !running;
            PID_Reset(&pid);
            LineSensor_Reset();
            last_control_ms = now;
            if (!running) {
                Motor_Stop();
                Display_ShowDigit(0);
            } else {
                Display_ShowDigit(1);
            }
        }

        if (!running || (now - last_control_ms) < LINE_CONTROL_PERIOD_MS) {
            HAL_Delay(1);
            continue;
        }

        float dt = (float)(now - last_control_ms) / 1000.0f;
        last_control_ms = now;
        LineSensorState line = LineSensor_Read();

        if (line.lost) {
            int16_t turn = line.last_valid_error < 0 ? -LINE_OPEN_LOOP_TURN : LINE_OPEN_LOOP_TURN;
            Motor_SetBoth((int16_t)(LINE_LOST_SEARCH_SPEED + turn),
                          (int16_t)(LINE_LOST_SEARCH_SPEED - turn));
            Display_ShowDigit(8);
            continue;
        }

        float turn = PID_Update(&pid, (float)line.error, dt);
        Motor_SetBoth((int16_t)((float)PID_LINE_BASE_SPEED + turn),
                      (int16_t)((float)PID_LINE_BASE_SPEED - turn));
        Display_ShowDigit((uint8_t)(line.error + 1));
    }
}

void App_Main(void)
{
    AppMode mode = PROJECT_APP_MODE;

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
