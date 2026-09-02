#include "pid.h"

static float Clamp(float value, float limit)
{
    if (value > limit) {
        return limit;
    }
    if (value < -limit) {
        return -limit;
    }
    return value;
}

void PID_Init(PidController *pid, float kp, float ki, float kd, float output_limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->output_limit = output_limit;
}

float PID_Update(PidController *pid, float error, float dt_s)
{
    if (dt_s <= 0.0f) {
        dt_s = 0.001f;
    }

    pid->integral = Clamp(pid->integral + error * dt_s, pid->output_limit);
    float derivative = (error - pid->last_error) / dt_s;
    pid->last_error = error;

    return Clamp(pid->kp * error + pid->ki * pid->integral + pid->kd * derivative,
                 pid->output_limit);
}

void PID_Reset(PidController *pid)
{
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
}
