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

static void KickStartLineFollow(void)
{
    Motor_SetBoth((int16_t)(LINE_START_BOOST_SPEED + LINE_LEFT_TRIM),
                  (int16_t)(LINE_START_BOOST_SPEED + LINE_RIGHT_TRIM));
    HAL_Delay(LINE_START_BOOST_MS);
}

static void DriveOneWheelTurn(int16_t direction)
{
    /* Differential turn: inner wheel keeps rolling slowly instead of
       stopping completely, so the nose swings gently instead of
       over-shooting (reduces straight-line wobble). */
    if (direction < 0) {
        Motor_SetBoth(LINE_TURN_INNER_SPEED, LINE_TURN_OUTER_SPEED);
    } else {
        Motor_SetBoth(LINE_TURN_OUTER_SPEED, LINE_TURN_INNER_SPEED);
    }
}

static void DrivePivotTurn(int16_t direction)
{
    /* Hard pivot (one wheel stopped): tightest turning radius, used
       only when the line is LOST and the car must circle back to it.
       Differential turning cannot recover from a lost line. */
    if (direction < 0) {
        Motor_SetBoth(0, LINE_LOST_TURN_SPEED);
    } else {
        Motor_SetBoth(LINE_LOST_TURN_SPEED, 0);
    }
}

static uint8_t GetLinePattern(LineSensorState line)
{
    return (line.left_on_black ? 1u : 0u)
         | (line.right_on_black ? 2u : 0u);
}

static void StartOrUpdateTurn(int16_t requested_direction,
                              int16_t *turn_direction,
                              uint32_t *turn_started_ms,
                              uint32_t now)
{
    if (*turn_direction == 0 ||
        (*turn_direction != requested_direction &&
         (now - *turn_started_ms) >= LINE_TURN_MIN_HOLD_MS)) {
        *turn_direction = requested_direction;
        *turn_started_ms = now;
    }

    DriveOneWheelTurn(*turn_direction);
}

static void UpdateDirectionHint(uint8_t pattern,
                                uint8_t *candidate_pattern,
                                uint32_t *candidate_started_ms,
                                int16_t *last_hint,
                                uint32_t *last_hint_ms,
                                uint32_t now)
{
    if (pattern != 1u && pattern != 2u) {
        *candidate_pattern = pattern;
        *candidate_started_ms = now;
        return;
    }

    if (pattern != *candidate_pattern) {
        *candidate_pattern = pattern;
        *candidate_started_ms = now;
        return;
    }

    if ((now - *candidate_started_ms) >= LINE_HINT_CONFIRM_MS) {
        *last_hint = pattern == 1u ? -1 : 1;
        *last_hint_ms = now;
    }
}

static void RunLineFollow(void)
{
    Button key;
    bool running = LINE_FOLLOW_AUTO_START != 0;
    int16_t turn_direction = 0;
    int16_t last_hint = 0;
    uint32_t turn_started_ms = 0u;
    uint32_t last_hint_ms = 0u;
    uint32_t lost_started_ms = 0u;
    uint32_t center_started_ms = 0u;
    uint32_t candidate_started_ms = 0u;
    uint32_t hint_candidate_started_ms = 0u;
    uint8_t stable_pattern;
    uint8_t candidate_pattern;
    uint8_t hint_candidate_pattern;
    bool lost_active = false;

    Button_Init(&key);
    LineSensor_Reset();
    Motor_Stop();
    stable_pattern = GetLinePattern(LineSensor_Read());
    candidate_pattern = stable_pattern;
    hint_candidate_pattern = stable_pattern;
    candidate_started_ms = HAL_GetTick();
    hint_candidate_started_ms = candidate_started_ms;
    Display_ShowDigit(running ? stable_pattern : 0u);
    if (running) {
        KickStartLineFollow();
    }

    while (1) {
        if (Button_UpdatePressedEvent(&key, HAL_GetTick())) {
            running = !running;
            LineSensor_Reset();
            turn_direction = 0;
            last_hint = 0;
            turn_started_ms = 0u;
            last_hint_ms = 0u;
            lost_started_ms = 0u;
            center_started_ms = 0u;
            lost_active = false;
            if (!running) {
                Motor_Stop();
                Display_ShowDigit(0);
            } else {
                stable_pattern = GetLinePattern(LineSensor_Read());
                candidate_pattern = stable_pattern;
                hint_candidate_pattern = stable_pattern;
                candidate_started_ms = HAL_GetTick();
                hint_candidate_started_ms = candidate_started_ms;
                Display_ShowDigit(stable_pattern);
                KickStartLineFollow();
            }
        }

        if (running) {
            uint32_t now = HAL_GetTick();
            LineSensorState line = LineSensor_Read();
            uint8_t pattern = GetLinePattern(line);

            UpdateDirectionHint(pattern,
                                &hint_candidate_pattern,
                                &hint_candidate_started_ms,
                                &last_hint,
                                &last_hint_ms,
                                now);

            if (pattern != candidate_pattern) {
                candidate_pattern = pattern;
                candidate_started_ms = now;
            } else if (pattern != stable_pattern &&
                       (now - candidate_started_ms) >= LINE_STATE_CONFIRM_MS) {
                stable_pattern = pattern;
            }

            Display_ShowDigit(stable_pattern);

            if (stable_pattern == 1u) {
                center_started_ms = 0u;
                lost_active = false;
                last_hint = -1;
                last_hint_ms = now;
                StartOrUpdateTurn(-1, &turn_direction, &turn_started_ms, now);
            } else if (stable_pattern == 2u) {
                center_started_ms = 0u;
                lost_active = false;
                last_hint = 1;
                last_hint_ms = now;
                StartOrUpdateTurn(1, &turn_direction, &turn_started_ms, now);
            } else if (stable_pattern == 3u) {
                lost_active = false;
                if (turn_direction != 0 &&
                    (now - turn_started_ms) < LINE_TURN_MIN_HOLD_MS) {
                    center_started_ms = 0u;
                    DriveOneWheelTurn(turn_direction);
                } else {
                    if (center_started_ms == 0u) {
                        center_started_ms = now;
                    }
                    if (turn_direction != 0 &&
                        (now - center_started_ms) < LINE_CENTER_CONFIRM_MS) {
                        DriveOneWheelTurn(turn_direction);
                    } else {
                        turn_direction = 0;
                        Motor_SetBoth((int16_t)(LINE_BASE_SPEED + LINE_LEFT_TRIM),
                                      (int16_t)(LINE_BASE_SPEED + LINE_RIGHT_TRIM));
                    }
                }
            } else {
                center_started_ms = 0u;

                if (!lost_active) {
                    lost_active = true;
                    lost_started_ms = now;
                    if (last_hint != 0 &&
                        (now - last_hint_ms) <= LINE_HINT_VALID_MS) {
                        turn_direction = last_hint;
                    } else {
                        turn_direction = LINE_BLIND_DEFAULT_DIRECTION;
                    }
                    turn_started_ms = now;
                }

                if (turn_direction != 0 &&
                    (now - lost_started_ms) < LINE_TURN_TIMEOUT_MS) {
                    DrivePivotTurn(turn_direction);
                } else {
                    turn_direction = 0;
                    Motor_Stop();
                }
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
            /* Pivot back toward the line instead of driving straight:
               LINE_OPEN_LOOP_TURN was 0, so the car just ran off curves. */
            if (line.last_valid_error < 0) {
                Motor_SetBoth(0, LINE_LOST_TURN_SPEED);
            } else {
                Motor_SetBoth(LINE_LOST_TURN_SPEED, 0);
            }
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
