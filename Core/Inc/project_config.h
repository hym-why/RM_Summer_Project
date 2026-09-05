
#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

typedef enum {
    APP_MODE_DIGIT_COUNTER = 0,
    APP_MODE_MOTOR_DIRECTION_TEST,
    APP_MODE_LINE_SENSOR_TEST,
    APP_MODE_START_STOP,
    APP_MODE_LINE_FOLLOW,
    APP_MODE_PID_LINE_FOLLOW,
} AppMode;

/* Change only this value when demonstrating an intermediate requirement. */
#define PROJECT_APP_MODE APP_MODE_PID_LINE_FOLLOW

#define MOTOR_MAX_SPEED          999
#define MOTOR_TEST_SPEED         600
#define MOTOR_RAMP_START_SPEED   100
#define MOTOR_RAMP_STEP           50
#define MOTOR_RAMP_STEP_MS        50u
#define STRAIGHT_DRIVE_SPEED     480
#define LINE_BASE_SPEED          450
#define LINE_LEFT_TRIM             0
#define LINE_RIGHT_TRIM            0
#define LINE_TURN_OUTER_SPEED     560
#define LINE_TURN_INNER_SPEED     280
#define LINE_LOST_TURN_SPEED      650
#define LINE_HINT_CONFIRM_MS        4u
#define LINE_HINT_VALID_MS       800u
#define LINE_BLIND_DEFAULT_DIRECTION (-1)
#define LINE_STATE_CONFIRM_MS      20u
#define LINE_TURN_MIN_HOLD_MS      60u
#define LINE_CENTER_CONFIRM_MS     40u
#define LINE_TURN_TIMEOUT_MS     1100u
#define PID_LINE_BASE_SPEED      440

/* Set either value to -1 if that wheel runs opposite to the expected direction. */
#define MOTOR_LEFT_POLARITY       1
#define MOTOR_RIGHT_POLARITY      1

#define MOTOR_REVERSE_DELAY_MS  300u
#define LINE_CONTROL_PERIOD_MS    2u
#define LINE_OPEN_LOOP_TURN        0
#define LINE_LOST_SEARCH_SPEED   320
#define LINE_FOLLOW_AUTO_START     0
#define LINE_START_BOOST_SPEED   850
#define LINE_START_BOOST_MS      150u

#define PID_KP                   140.0f
#define PID_KI                     0.0f
#define PID_KD                     1.2f
#define PID_OUTPUT_LIMIT         360.0f

#endif
