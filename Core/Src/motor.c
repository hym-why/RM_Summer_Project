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

static void SetDirection(GPIO_TypeDef *in1_port, uint16_t in1_pin,
                         GPIO_TypeDef *in2_port, uint16_t in2_pin,
                         MotorDirection direction)
{
    GPIO_PinState in1 = GPIO_PIN_RESET;
    GPIO_PinState in2 = GPIO_PIN_RESET;

    if (direction == MOTOR_DIR_FORWARD) {
        in1 = GPIO_PIN_SET;
    } else if (direction == MOTOR_DIR_BACKWARD) {
        in2 = GPIO_PIN_SET;
    }

    HAL_GPIO_WritePin(in1_port, in1_pin, in1);
    HAL_GPIO_WritePin(in2_port, in2_pin, in2);
}

static void SetPwm(uint32_t channel, uint16_t duty)
{
    __HAL_TIM_SET_COMPARE(s_pwm, channel, duty);
}

void Motor_Init(TIM_HandleTypeDef *pwm_timer)
{
    s_pwm = pwm_timer;
    HAL_TIM_PWM_Start(s_pwm, MOTOR_L_PWM_CHANNEL);
    HAL_TIM_PWM_Start(s_pwm, MOTOR_R_PWM_CHANNEL);
    Motor_Stop();
}

void Motor_SetLeft(int16_t speed)
{
    speed = ClampSpeed((int32_t)speed * MOTOR_LEFT_POLARITY);
    MotorDirection direction = MOTOR_DIR_STOP;

    if (speed > 0) {
        direction = MOTOR_DIR_FORWARD;
    } else if (speed < 0) {
        direction = MOTOR_DIR_BACKWARD;
        speed = (int16_t)-speed;
    }

    SetDirection(MOTOR_L_IN1_GPIO_Port, MOTOR_L_IN1_Pin,
                 MOTOR_L_IN2_GPIO_Port, MOTOR_L_IN2_Pin, direction);
    SetPwm(MOTOR_L_PWM_CHANNEL, (uint16_t)speed);
}

void Motor_SetRight(int16_t speed)
{
    speed = ClampSpeed((int32_t)speed * MOTOR_RIGHT_POLARITY);
    MotorDirection direction = MOTOR_DIR_STOP;

    if (speed > 0) {
        direction = MOTOR_DIR_FORWARD;
    } else if (speed < 0) {
        direction = MOTOR_DIR_BACKWARD;
        speed = (int16_t)-speed;
    }

    SetDirection(MOTOR_R_IN1_GPIO_Port, MOTOR_R_IN1_Pin,
                 MOTOR_R_IN2_GPIO_Port, MOTOR_R_IN2_Pin, direction);
    SetPwm(MOTOR_R_PWM_CHANNEL, (uint16_t)speed);
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
