#ifndef LINE_FOLLOW_H
#define LINE_FOLLOW_H

#include <stdint.h>

typedef struct {
    uint8_t left_on_black;
    uint8_t right_on_black;
    int16_t error;
    int16_t last_valid_error;
    uint8_t lost;
} LineSensorState;

void LineSensor_Reset(void);
LineSensorState LineSensor_Read(void);
int16_t LineFollow_ComputeTurn(LineSensorState state);

#endif
