#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float last_error;
    float derivative;
    float output_limit;
    float derivative_alpha;
    uint8_t initialized;
} PidController;

void PID_Init(PidController *pid, float kp, float ki, float kd,
              float output_limit, float derivative_alpha);
float PID_Update(PidController *pid, float error, float dt_s);
void PID_Reset(PidController *pid);

#endif
