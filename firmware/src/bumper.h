#pragma once

// Call once in setup() — after imu_init() so Wire is already running.
void bumper_init();

// Read current bumper state from the PCF8574. Call each loop().
void bumper_update();

// True if either bumper switch is pressed.
bool bumper_is_triggered();

bool bumper_left();
bool bumper_right();
