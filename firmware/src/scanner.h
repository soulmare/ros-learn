#pragma once

// Call once in setup().
void scanner_init();

// Advance the sweep by one step. Call every loop().
// Blocks for up to ULTRASONIC_TIMEOUT_US when a step fires.
void scanner_update();

// Begin a sweep from from_deg to to_deg in robot-frame degrees (0 = forward,
// positive = left, negative = right — ROS convention). Sends "OK SCAN" immediately.
void scanner_start(float from_deg, float to_deg);

// Shorthand: full sweep across the sensor's angular range.
void scanner_start_full();

// Single-point reading at robot-frame angle_deg. Still sends "OK SCAN" / "DONE SCAN".
void scanner_start_single(float angle_deg);

// Abort an active sweep. Sends "ERR SCAN interrupted". No-op if idle.
void scanner_stop();

bool  scanner_is_busy();

// Last distance reading from any angle (metres). -1.0f if none yet.
float scanner_last_distance_m();

// Most recent reading within ±SCANNER_STEP_DEG of forward (0°).
// Used by the safety layer for forward obstacle detection. -1.0f if none yet.
float scanner_forward_distance_m();
