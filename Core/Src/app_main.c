#include <stdbool.h>
#include "main.h"
#include "board_config.h"
#include "button.h"
#include "display.h"
#include "line_follow.h"
#include "motor.h"
#include "pid.h"
#include "project_config.h"

extern TIM_HandleTypeDef MOTOR_PWM_TIMER;

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
    Motor_SetBoth(MOTOR_TEST_SPEED, MOTOR_TEST_SPEED);

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            forward = !forward;
            Motor_SetBoth(forward ? MOTOR_TEST_SPEED : -MOTOR_TEST_SPEED,
                          forward ? MOTOR_TEST_SPEED : -MOTOR_TEST_SPEED);
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
                Motor_SetBoth(STRAIGHT_DRIVE_SPEED, STRAIGHT_DRIVE_SPEED);
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
    LineSensor_Reset();

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            running = !running;
            LineSensor_Reset();
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
            int16_t search_turn = (line.last_valid_error < 0) ? -LINE_OPEN_LOOP_TURN : LINE_OPEN_LOOP_TURN;
            Motor_SetBoth((int16_t)(LINE_LOST_SEARCH_SPEED + search_turn),
                          (int16_t)(LINE_LOST_SEARCH_SPEED - search_turn));
            Beep(15);
            Display_ShowDigit(8);
            HAL_Delay(40);
            continue;
        }

        int16_t turn = LineFollow_ComputeTurn(line);
        Motor_SetBoth((int16_t)(LINE_BASE_SPEED + turn), (int16_t)(LINE_BASE_SPEED - turn));
        Display_ShowDigit((uint8_t)(line.error + 1));
        HAL_Delay(10);
    }
}

static void RunPidLineFollow(void)
{
    Button key;
    Button_Init(&key);
    PidController pid;
    PID_Init(&pid, PID_KP, PID_KI, PID_KD, PID_OUTPUT_LIMIT);
    LineSensor_Reset();

    bool running = false;
    uint32_t last_tick = HAL_GetTick();

    while (1) {
        uint32_t now = HAL_GetTick();
        if (Button_UpdatePressedEvent(&key, now)) {
            running = !running;
            PID_Reset(&pid);
            LineSensor_Reset();
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
            int16_t search_turn = (line.last_valid_error < 0) ? -LINE_OPEN_LOOP_TURN : LINE_OPEN_LOOP_TURN;
            Motor_SetBoth((int16_t)(LINE_LOST_SEARCH_SPEED + search_turn),
                          (int16_t)(LINE_LOST_SEARCH_SPEED - search_turn));
            HAL_Delay(20);
            continue;
        }

        float turn = PID_Update(&pid, (float)line.error, dt);
        Motor_SetBoth((int16_t)(PID_LINE_BASE_SPEED + turn), (int16_t)(PID_LINE_BASE_SPEED - turn));
        HAL_Delay(5);
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
