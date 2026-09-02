#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "main.h"

/* STM32_10B base board seven-segment display.
 * The schematic maps a,b,c,d,e,f,g,dp to PA0-PA7.
 * The display is common-anode, so low level turns a segment on.
 */
#define SEG_A_GPIO_Port  GPIOA
#define SEG_A_Pin        GPIO_PIN_0
#define SEG_B_GPIO_Port  GPIOA
#define SEG_B_Pin        GPIO_PIN_1
#define SEG_C_GPIO_Port  GPIOA
#define SEG_C_Pin        GPIO_PIN_2
#define SEG_D_GPIO_Port  GPIOA
#define SEG_D_Pin        GPIO_PIN_3
#define SEG_E_GPIO_Port  GPIOA
#define SEG_E_Pin        GPIO_PIN_4
#define SEG_F_GPIO_Port  GPIOA
#define SEG_F_Pin        GPIO_PIN_5
#define SEG_G_GPIO_Port  GPIOA
#define SEG_G_Pin        GPIO_PIN_6
#define SEG_DP_GPIO_Port GPIOA
#define SEG_DP_Pin       GPIO_PIN_7

#define USER_KEY_GPIO_Port GPIOA
#define USER_KEY_Pin       GPIO_PIN_15
#define USER_KEY_ACTIVE    GPIO_PIN_RESET

#define BUZZER_GPIO_Port GPIOB
#define BUZZER_Pin       GPIO_PIN_0

#define MOTOR_L_IN1_GPIO_Port GPIOB
#define MOTOR_L_IN1_Pin       GPIO_PIN_12
#define MOTOR_L_IN2_GPIO_Port GPIOB
#define MOTOR_L_IN2_Pin       GPIO_PIN_13
#define MOTOR_R_IN1_GPIO_Port GPIOB
#define MOTOR_R_IN1_Pin       GPIO_PIN_14
#define MOTOR_R_IN2_GPIO_Port GPIOB
#define MOTOR_R_IN2_Pin       GPIO_PIN_15

#define MOTOR_PWM_TIMER       htim4
#define MOTOR_L_PWM_CHANNEL   TIM_CHANNEL_1
#define MOTOR_R_PWM_CHANNEL   TIM_CHANNEL_2

#define LINE_LEFT_GPIO_Port  GPIOA
#define LINE_LEFT_Pin        GPIO_PIN_11
#define LINE_RIGHT_GPIO_Port GPIOA
#define LINE_RIGHT_Pin       GPIO_PIN_12

/* The supplied LM358 module drives D0 high when black is detected. */
#define LINE_BLACK_LEVEL GPIO_PIN_SET

#endif
