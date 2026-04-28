#pragma once

// Initialise heading PID. Call from setup() after imu_init().
void heading_init();

// Run heading correction each loop iteration.
// Holds heading when omega=0 (straight drive); tracks a continuously advancing
// target when omega≠0 (curved drive). Both use IMU feedback.
void heading_update();

// Set forward speed and angular rate. Routes all motion through IMU-based heading control.
// omega_dps: desired turning rate in degrees/s (0 = straight, positive = counter-clockwise).
// Pass mps=0.0f and omega_dps=0.0f to stop.
void heading_set_velocity(float mps, float omega_dps);
