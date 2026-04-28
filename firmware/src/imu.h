#pragma once
#include <stdint.h>

// Initialise MPU-6050 and run 1 s bias calibration (blocking).
// Must be called from setup() before imu_update().
void imu_init();

// Read gyro Z, subtract calibrated bias, integrate into heading.
// Call from loop() — internally rate-limits to IMU_SAMPLE_INTERVAL_MS.
void imu_update();

// Return current heading in degrees. Positive = counter-clockwise (right-hand rule).
float imu_get_heading();

// Zero the heading — call at the start of each new move so errors don't carry over.
void imu_reset_heading();
