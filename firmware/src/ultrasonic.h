#pragma once

// Call once in setup().
void  ultrasonic_init();

// Trigger a single distance measurement.
// Returns distance in metres, or -1.0f if no echo within ULTRASONIC_TIMEOUT_US.
float ultrasonic_measure_m();
