#pragma once

// Geometry — measure on the physical robot
#define TRACK_WIDTH_M       0.127f   // distance between left and right wheel centres (metres)

// Motor output limits
#define PWM_MAX             255
#define PWM_MIN             0
#define PWM_DEADBAND        110     // minimum PWM that overcomes static friction

// Velocity scaling: maps 1 m/s to this PWM value (open-loop approximation, tune by observation)
#define VEL_TO_PWM_SCALE    180.0f

// Debug output flags
#define DEBUG_MOTORS        1       // print PWM values when motors change (0 = off, 1 = on)
