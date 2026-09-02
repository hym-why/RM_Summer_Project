#ifndef PID_H
#define PID_H

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float last_error;
    float output_limit;
} PidController;

void PID_Init(PidController *pid, float kp, float ki, float kd, float output_limit);
float PID_Update(PidController *pid, float error, float dt_s);
void PID_Reset(PidController *pid);

#endif
