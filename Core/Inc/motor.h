#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "main.h"

typedef enum {
    MOTOR_DIR_STOP = 0,
    MOTOR_DIR_FORWARD,
    MOTOR_DIR_BACKWARD,
} MotorDirection;

void Motor_Init(TIM_HandleTypeDef *pwm_timer);
void Motor_SetLeft(int16_t speed);
void Motor_SetRight(int16_t speed);
void Motor_SetBoth(int16_t left_speed, int16_t right_speed);
void Motor_Stop(void);

#endif

