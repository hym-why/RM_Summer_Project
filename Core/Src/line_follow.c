#include "line_follow.h"
#include "board_config.h"
#include "project_config.h"

static int16_t s_last_valid_error = 0;

void LineSensor_Reset(void)
{
    s_last_valid_error = 0;
}

LineSensorState LineSensor_Read(void)
{
    LineSensorState state;
    state.left_on_black = HAL_GPIO_ReadPin(LINE_LEFT_GPIO_Port, LINE_LEFT_Pin) == LINE_BLACK_LEVEL;
    state.right_on_black = HAL_GPIO_ReadPin(LINE_RIGHT_GPIO_Port, LINE_RIGHT_Pin) == LINE_BLACK_LEVEL;
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
    if (state.lost) {
        return 0;
    }
    return (int16_t)(state.error * LINE_OPEN_LOOP_TURN);
}
