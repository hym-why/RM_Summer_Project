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

static int16_t AbsSpeed(int16_t value)
{
    return value < 0 ? (int16_t)-value : value;
}

static void SetLostWarning(bool lost)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin,
                      lost ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void SearchForLine(LineSensorState line)
{
    int16_t direction = line.last_valid_error < 0 ? -1 : 1;
    int16_t turn = (int16_t)(direction * LINE_LOST_SEARCH_SPEED);

    Motor_SetBoth(turn, (int16_t)-turn);
    SetLostWarning(true);
    Display_ShowDigit(8);
}

static int16_t ComputeAdaptiveBaseSpeed(int16_t turn)
{
    int32_t reduction = (int32_t)AbsSpeed(turn) * PID_TURN_SLOWDOWN
                      / (int32_t)PID_OUTPUT_LIMIT;
    int16_t base = (int16_t)(PID_LINE_BASE_SPEED - reduction);

    return base < PID_LINE_MIN_SPEED ? PID_LINE_MIN_SPEED : base;
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

static void RampMotors(int16_t target_speed)
{
    int16_t direction = target_speed < 0 ? -1 : 1;
    uint16_t target = (uint16_t)(target_speed < 0 ? -target_speed : target_speed);

    for (uint16_t speed = MOTOR_RAMP_START_SPEED; speed < target;
         speed = (uint16_t)(speed + MOTOR_RAMP_STEP)) {
        int16_t signed_speed = (int16_t)(direction * (int16_t)speed);
        Motor_SetBoth(signed_speed, signed_speed);
        HAL_Delay(MOTOR_RAMP_STEP_MS);
    }

    Motor_SetBoth(target_speed, target_speed);
}

static void RunMotorDirectionTest(void)
{
    Button key;
    bool forward = true;

    Button_Init(&key);
    Display_ShowDigit(1);
    RampMotors(MOTOR_TEST_SPEED);

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            Motor_Stop();
            HAL_Delay(MOTOR_REVERSE_DELAY_MS);
            forward = !forward;
            Display_ShowDigit(forward ? 1 : 2);
            RampMotors(forward ? MOTOR_TEST_SPEED : -MOTOR_TEST_SPEED);
            Beep(40);
        }
        HAL_Delay(1);
    }
}

static void RunLineSensorTest(void)
{
    LineSensor_Reset();
    SetLostWarning(false);

    while (1) {
        LineSensorState line = LineSensor_Read();
        uint8_t state = (line.left_on_black ? 1u : 0u)
                      | (line.right_on_black ? 2u : 0u);

        Display_ShowDigit(state);
        HAL_Delay(20);
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
                RampMotors(STRAIGHT_DRIVE_SPEED);
                Display_ShowDigit(1);
            } else {
                Motor_Stop();
                Display_ShowDigit(0);
            }
        }
        HAL_Delay(1);
    }
}

static void KickStartLineFollow(void)
{
    Motor_SetBoth((int16_t)(LINE_START_BOOST_SPEED + LINE_LEFT_TRIM),
                  (int16_t)(LINE_START_BOOST_SPEED + LINE_RIGHT_TRIM));
    HAL_Delay(LINE_START_BOOST_MS);
}

static void RunLineFollow(void)
{
    Button key;
    bool running = LINE_FOLLOW_AUTO_START != 0;

    Button_Init(&key);
    LineSensor_Reset();
    Motor_Stop();
    SetLostWarning(false);
    Display_ShowDigit(running ? 1 : 0);
    if (running) {
        KickStartLineFollow();
    }

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            running = !running;
            LineSensor_Reset();
            if (!running) {
                Motor_Stop();
                SetLostWarning(false);
                Display_ShowDigit(0);
            } else {
                Display_ShowDigit(1);
                KickStartLineFollow();
            }
        }

        if (running) {
            LineSensorState line = LineSensor_Read();
            if (line.lost) {
#if LINE_STOP_WHEN_LOST
                Motor_Stop();
                SetLostWarning(true);
                Display_ShowDigit(8);
#else
                SearchForLine(line);
#endif
            } else {
                int16_t turn = LineFollow_ComputeTurn(line);
                Motor_SetBoth((int16_t)(LINE_BASE_SPEED + LINE_LEFT_TRIM + turn),
                              (int16_t)(LINE_BASE_SPEED + LINE_RIGHT_TRIM - turn));
                SetLostWarning(false);
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
    PID_Init(&pid, PID_KP, PID_KI, PID_KD,
             PID_OUTPUT_LIMIT, PID_DERIVATIVE_ALPHA);
    LineSensor_Reset();
    Motor_Stop();
    SetLostWarning(false);
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
                SetLostWarning(false);
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
            PID_Reset(&pid);
            SearchForLine(line);
            continue;
        }

        int16_t turn = (int16_t)PID_Update(&pid, (float)line.error, dt);
        int16_t base_speed = ComputeAdaptiveBaseSpeed(turn);
        Motor_SetBoth((int16_t)(base_speed + turn),
                      (int16_t)(base_speed - turn));
        SetLostWarning(false);
        Display_ShowDigit((uint8_t)(line.error + 1));
    }
}

void App_Main(void)
{
    AppMode mode = PROJECT_APP_MODE;

    if (mode != APP_MODE_DIGIT_COUNTER && mode != APP_MODE_LINE_SENSOR_TEST) {
        Motor_Init(&MOTOR_PWM_TIMER);
    }

    switch (mode) {
    case APP_MODE_DIGIT_COUNTER:
        RunDigitCounter();
        break;
    case APP_MODE_MOTOR_DIRECTION_TEST:
        RunMotorDirectionTest();
        break;
    case APP_MODE_LINE_SENSOR_TEST:
        RunLineSensorTest();
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
