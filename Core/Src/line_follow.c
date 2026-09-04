#include "line_follow.h"
#include "board_config.h"
#include "project_config.h"

static int16_t s_last_valid_error;

void LineSensor_Reset(void)
{
    s_last_valid_error = 0;
}

LineSensorState LineSensor_Read(void)
{
    LineSensorState state;
    uint8_t raw_left = HAL_GPIO_ReadPin(LINE_LEFT_GPIO_Port, LINE_LEFT_Pin) == LINE_BLACK_LEVEL;
    uint8_t raw_right = HAL_GPIO_ReadPin(LINE_RIGHT_GPIO_Port, LINE_RIGHT_Pin) == LINE_BLACK_LEVEL;

#if LINE_SENSOR_SWAP
    state.left_on_black = raw_right;
    state.right_on_black = raw_left;
#else
    state.left_on_black = raw_left;
    state.right_on_black = raw_right;
#endif
    state.last_valid_error = s_last_valid_error;

    if (state.left_on_black && !state.right_on_black) {
        state.error = -1;
        state.lost = 0;
    } else if (!state.left_on_black && state.right_on_black) {
        state.error = 1;
        state.lost = 0;
    } else if (state.left_on_black && state.right_on_black) {
        state.error = 0;
        state.lost = 0;
    } else {
        state.error = s_last_valid_error;
        state.lost = 1;
    }

    if (!state.lost) {
        s_last_valid_error = state.error;
        state.last_valid_error = s_last_valid_error;
    }

    return state;
}

int16_t LineFollow_ComputeTurn(LineSensorState state)
{
    return state.lost ? 0 : (int16_t)(state.error * LINE_OPEN_LOOP_TURN);
}
