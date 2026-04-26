#include <Arduino.h>
#include "pid.h"

void pid_init(PidState *pid, float kp, float ki, float kd, float i_max) {
    pid->kp      = kp;
    pid->ki      = ki;
    pid->kd      = kd;
    pid->i_max   = i_max;
    pid_reset(pid);
}

void pid_reset(PidState *pid) {
    pid->setpoint   = 0.0f;
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
    pid->last_d     = 0.0f;
    pid->last_ms    = 0;
}

float pid_update(PidState *pid, float measured, float dt_s) {
    // Compute error
    float error = pid->setpoint - measured;

    // Accumulate integral with anti-windup clamping
    pid->integral += error * dt_s;
    pid->integral = constrain(pid->integral, -pid->i_max, pid->i_max);

    // Compute derivative
    float derivative = (error - pid->prev_error) / dt_s;
    pid->last_d      = pid->kd * derivative;

    pid->prev_error = error;

    return pid->kp * error + pid->ki * pid->integral + pid->last_d;
}
