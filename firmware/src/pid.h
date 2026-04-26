#pragma once
#include <stdint.h>

struct PidState {
    float kp;
    float ki;
    float kd;
    float i_max;        // integral clamp magnitude

    float setpoint;
    float integral;
    float prev_error;
    float last_d;       // last derivative term (Kd * derivative), for debug
    uint32_t last_ms;   // timestamp of last update
};

// Initialise a PidState with gains; setpoint and accumulators start at zero.
void pid_init(PidState *pid, float kp, float ki, float kd, float i_max);

// Reset the integrator, derivative state, and timestamp without changing gains.
void pid_reset(PidState *pid);

// Run one PID update. measured is the current value; dt_s is elapsed time in
// seconds since the last call. Returns the correction to add to the base output.
float pid_update(PidState *pid, float measured, float dt_s);
