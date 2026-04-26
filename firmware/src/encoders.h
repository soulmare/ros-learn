#pragma once
#include "pid.h"

// Call once in setup().
void encoders_init();

// Call every loop(). Runs velocity estimation and PID on a 50 ms cadence;
// writes new PWM values to the motors when the interval elapses.
void encoders_update();

// Set the target velocity for both wheels (m/s). Negative = reverse.
// Called by serial_comms when a SET_VEL command arrives.
void encoders_set_velocity(float left_mps, float right_mps);

// Returns the most recently estimated speed for each wheel (m/s).
float encoders_get_left_velocity();
float encoders_get_right_velocity();
