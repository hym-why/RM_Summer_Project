#include "motor.h"
#include "board_config.h"

#define MOTOR_MAX_SPEED 1000

static TIM_HandleTypeDef *s_pwm;

static int16_t ClampSpeed(int16_t speed)
{
    if (speed > MOTOR_MAX_SPEED) {
        return MOTOR_MAX_SPEED;
    }
    if (speed < -MOTOR_MAX_SPEED) {
        return -MOTOR_MAX_SPEED;
    }
    return speed;
}

static void SetDirection(GPIO_TypeDef *in1_port, uint16_t in1_pin,
                         GPIO_TypeDef *in2_port, uint16_t in2_pin,
                         MotorDirection direction)
{
    switch (direction) {
    case MOTOR_DIR_FORWARD:
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
        break;
    case MOTOR_DIR_BACKWARD:
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_SET);
        break;
    default:
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
        break;
    }
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
    speed = ClampSpeed(speed);
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
    speed = ClampSpeed(speed);
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

