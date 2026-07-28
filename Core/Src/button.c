#include "button.h"
#include "board_config.h"

#define BUTTON_DEBOUNCE_MS 25u

static bool ReadRawPressed(void)
{
    return HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin) == USER_KEY_ACTIVE;
}

void Button_Init(Button *button)
{
    button->stable_pressed = ReadRawPressed();
    button->last_sample = button->stable_pressed;
    button->last_change_ms = HAL_GetTick();
}

bool Button_UpdatePressedEvent(Button *button, uint32_t now_ms)
{
    bool sample = ReadRawPressed();
    if (sample != button->last_sample) {
        button->last_sample = sample;
        button->last_change_ms = now_ms;
    }

    if ((now_ms - button->last_change_ms) < BUTTON_DEBOUNCE_MS) {
        return false;
    }

    if (button->stable_pressed != sample) {
        button->stable_pressed = sample;
        return button->stable_pressed;
    }

    return false;
}

