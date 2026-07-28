#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool stable_pressed;
    bool last_sample;
    uint32_t last_change_ms;
} Button;

void Button_Init(Button *button);
bool Button_UpdatePressedEvent(Button *button, uint32_t now_ms);

#endif

