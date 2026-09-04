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
#define PROJECT_APP_MODE APP_MODE_LINE_FOLLOW

#define MOTOR_MAX_SPEED          999
#define MOTOR_TEST_SPEED         600
#define MOTOR_RAMP_START_SPEED   100
#define MOTOR_RAMP_STEP           50
#define MOTOR_RAMP_STEP_MS        50u
#define STRAIGHT_DRIVE_SPEED     480
#define LINE_BASE_SPEED          440
#define LINE_LEFT_TRIM             0
#define LINE_RIGHT_TRIM            0
#define PID_LINE_BASE_SPEED      440

/* Set either value to -1 if that wheel runs opposite to the expected direction. */
#define MOTOR_LEFT_POLARITY       1
#define MOTOR_RIGHT_POLARITY      1

#define MOTOR_REVERSE_DELAY_MS  300u
#define LINE_CONTROL_PERIOD_MS   10u
#define LINE_OPEN_LOOP_TURN        0
#define LINE_LOST_SEARCH_SPEED   240
#define LINE_FOLLOW_AUTO_START     1
#define LINE_STOP_WHEN_LOST        1
#define LINE_START_BOOST_SPEED   520
#define LINE_START_BOOST_MS      150u
#define PID_LINE_MIN_SPEED       280
#define PID_TURN_SLOWDOWN        160

#define PID_KP                   190.0f
#define PID_KI                     0.0f
#define PID_KD                     0.8f
#define PID_OUTPUT_LIMIT         350.0f
#define PID_DERIVATIVE_ALPHA       0.75f

#endif
