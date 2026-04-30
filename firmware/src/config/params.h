#pragma once

// Geometry — measure on the physical robot
#define TRACK_WIDTH_M       0.127f   // distance between left and right wheel centres (metres)

// Motor output limits
#define PWM_MAX                 255
#define PWM_MIN                 0
#define PWM_DEADBAND_KINETIC    55   // minimum PWM to keep a moving wheel rolling
#define PWM_DEADBAND_STATIC     80  // minimum PWM to break static friction from rest (tune on surface)
// Stay on static deadband until wheel speed exceeds this — prevents one stray tick from
// switching to kinetic prematurely and dropping back into the oscillation cycle
#define VEL_KINETIC_THRESHOLD   0.05f

// Velocity scaling: maps 1 m/s to this PWM value (open-loop approximation, tune by observation)
#define VEL_TO_PWM_SCALE    300.0f

// Encoder geometry
#define ENCODER_TICKS_PER_REV   40
#define WHEEL_RADIUS_M          0.034f
#define ENC_DEBOUNCE_US         200     // reject pulses arriving faster than this (bounce filter)
// #define ENC_DEBOUNCE_US         5000     // reject pulses arriving faster than this (bounce filter)
#define VEL_TIMEOUT_US          200000  // declare wheel stopped if no tick in this long (~0.027 m/s minimum)
// Velocity low-pass filter strength: 1.0 = no filter, lower = smoother but slower to react
// A sustained real speed change propagates through in roughly 1/alpha update cycles
//  — at alpha=0.4 and 100ms interval, that's about 2-3 updates (200-300ms) to track a step change
#define ENC_VEL_ALPHA           0.4f

// PID / velocity control
#define PID_INTERVAL_MS         100      // velocity estimator and PID update period
#define PID_KP                  150.0f
#define PID_KI                  350.0f     // updated in pair with PID_I_MAX
#define PID_KD                  0.0f

// PID_I_MAX clamps the integral accumulator. Max PWM contribution from integral = Ki * PID_I_MAX.
// Rule of thumb: PID_I_MAX = PWM_MAX / Ki  (keeps max integral contribution ≤ 255 PWM)
// Example: Ki=50 → I_MAX=5,  Ki=20 → I_MAX=12,  Ki=2 → I_MAX=127.5
#define PID_I_MAX               0.73f  // integral clamp, prevents windup

// IMU & heading control
#define IMU_CALIBRATION_MS          1000    // duration of gyro bias calibration on startup (blocking)
#define IMU_SAMPLE_INTERVAL_MS      47      // ~21 Hz gyro integration rate (1000 / 21 ≈ 47)

#define HEADING_KP                  0.02f    // tune on robot: start low, increase until drift corrects quickly
#define HEADING_KI                  0.0f
#define HEADING_KD                  0.002f
#define HEADING_I_MAX               10.0f
// Max velocity correction the heading PID can apply to each wheel (m/s) — keeps turns from overpowering forward vel
#define HEADING_CORRECTION_MAX      0.15f
// Heading error below which a TURN is considered complete (degrees)
#define TURN_THRESHOLD_DEG          2.0f

// Servo & scanner
#define SCANNER_SERVO_FORWARD_DEG   81.0f   // angle that points the sensor straight ahead
#define SCANNER_SERVO_MIN_DEG       5.0f
#define SCANNER_SERVO_MAX_DEG       165.0f
#define SCANNER_STEP_DEG            5.0f    // degrees between sweep samples
#define SCANNER_STEP_MS             100     // ms between steps (servo settle time + measurement)

// Ultrasonic (HC-SR04)
// Timeout formula: us = (max_dist_m * 2) / 0.000343
//   0.5 m →  2915 us |  1.0 m →  5831 us
//   1.5 m →  8746 us |  2.6 m → 15000 us (HC-SR04 datasheet max)
// Readings beyond this range return -1.0f (no echo).
#define ULTRASONIC_TIMEOUT_US   15000

// Safety
#define OBSTACLE_THRESHOLD_M    0.20f   // stop if forward obstacle closer than this (metres)

// Telemetry
#define VEL_REPORT          0       // send "VEL <left_mps> <right_mps>" each PID interval (0 = off, 1 = on)

// Debug output flags
#define DEBUG_MOTORS        0       // print PWM values when motors change (0 = off, 1 = on)
#define DEBUG_PID           0       // print PID state each update (0 = off, 1 = on)
#define DEBUG_ENCODERS      0       // print wheel velocity and cumulative distance each PID interval
#define DEBUG_HEADING       0       // print heading PID state each update (0 = off, 1 = on)
#define DEBUG_SCANNER       1       // print every servo write (0 = off, 1 = on)
