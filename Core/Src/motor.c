#include "motor.h"
#include "board_config.h"
#include "project_config.h"

static TIM_HandleTypeDef *s_pwm;

static int16_t ClampSpeed(int32_t speed)
{
    if (speed > MOTOR_MAX_SPEED) {
        return MOTOR_MAX_SPEED;
    }
    if (speed < -MOTOR_MAX_SPEED) {
        return -MOTOR_MAX_SPEED;
    }
    return (int16_t)speed;
}

static void SetPwm(uint32_t channel, uint16_t duty)
{
    __HAL_TIM_SET_COMPARE(s_pwm, channel, duty);
}

static void SetBridgePwm(uint32_t forward_channel, uint32_t backward_channel,
                         int16_t speed)
{
    if (speed > 0) {
        SetPwm(forward_channel, (uint16_t)speed);
        SetPwm(backward_channel, 0u);
    } else if (speed < 0) {
        SetPwm(forward_channel, 0u);
        SetPwm(backward_channel, (uint16_t)-speed);
    } else {
        SetPwm(forward_channel, 0u);
        SetPwm(backward_channel, 0u);
    }
}

void Motor_Init(TIM_HandleTypeDef *pwm_timer)
{
    s_pwm = pwm_timer;
    HAL_TIM_PWM_Start(s_pwm, MOTOR_L_FORWARD_CHANNEL);
    HAL_TIM_PWM_Start(s_pwm, MOTOR_L_BACKWARD_CHANNEL);
    HAL_TIM_PWM_Start(s_pwm, MOTOR_R_FORWARD_CHANNEL);
    HAL_TIM_PWM_Start(s_pwm, MOTOR_R_BACKWARD_CHANNEL);
    Motor_Stop();
}

void Motor_SetLeft(int16_t speed)
{
    speed = ClampSpeed((int32_t)speed * MOTOR_LEFT_POLARITY);
    SetBridgePwm(MOTOR_L_FORWARD_CHANNEL, MOTOR_L_BACKWARD_CHANNEL, speed);
}

void Motor_SetRight(int16_t speed)
{
    speed = ClampSpeed((int32_t)speed * MOTOR_RIGHT_POLARITY);
    SetBridgePwm(MOTOR_R_FORWARD_CHANNEL, MOTOR_R_BACKWARD_CHANNEL, speed);
}

void Motor_SetBoth(int16_t left_speed, int16_t right_speed)
{
    Motor_SetLeft(left_speed);
    Motor_SetRight(right_speed);
}

void Motor_Stop(void)
{
    Motor_SetBoth(0, 0);
}
