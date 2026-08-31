#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

typedef enum {
    APP_MODE_DIGIT_COUNTER = 0,
    APP_MODE_MOTOR_DIRECTION_TEST,
    APP_MODE_START_STOP,
    APP_MODE_LINE_FOLLOW,
    APP_MODE_PID_LINE_FOLLOW,
} AppMode;

/* Select the function to demonstrate before compiling.
 * For the final project, use APP_MODE_PID_LINE_FOLLOW.
 */
#define PROJECT_APP_MODE APP_MODE_PID_LINE_FOLLOW

#define MOTOR_TEST_SPEED       550
#define STRAIGHT_DRIVE_SPEED   500
#define LINE_BASE_SPEED        430
#define PID_LINE_BASE_SPEED    560

#define LINE_OPEN_LOOP_TURN    220
#define LINE_LOST_SEARCH_SPEED 260

#define PID_KP                 260.0f
#define PID_KI                 0.0f
#define PID_KD                 45.0f
#define PID_OUTPUT_LIMIT       380.0f

#endif

