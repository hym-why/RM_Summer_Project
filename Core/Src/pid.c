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

void PID_Init(PidController *pid, float kp, float ki, float kd,
              float output_limit, float derivative_alpha)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->derivative = 0.0f;
    pid->output_limit = output_limit;
    pid->derivative_alpha = Clamp(derivative_alpha, 1.0f);
    if (pid->derivative_alpha < 0.0f) {
        pid->derivative_alpha = 0.0f;
    }
    pid->initialized = 0u;
}

float PID_Update(PidController *pid, float error, float dt_s)
{
    if (dt_s <= 0.0f) {
        dt_s = 0.001f;
    }

    float derivative = 0.0f;
    if (pid->initialized) {
        float raw_derivative = (error - pid->last_error) / dt_s;
        derivative = pid->derivative_alpha * pid->derivative
                   + (1.0f - pid->derivative_alpha) * raw_derivative;
    }

    float next_integral = Clamp(pid->integral + error * dt_s, pid->output_limit);
    float unclamped_output = pid->kp * error + pid->ki * next_integral
                           + pid->kd * derivative;
    float output = Clamp(unclamped_output, pid->output_limit);

    /* Do not accumulate integral while saturation pushes in the same direction. */
    if (output == unclamped_output || error * unclamped_output < 0.0f) {
        pid->integral = next_integral;
    }

    pid->last_error = error;
    pid->derivative = derivative;
    pid->initialized = 1u;

    return output;
}

void PID_Reset(PidController *pid)
{
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->derivative = 0.0f;
    pid->initialized = 0u;
}
