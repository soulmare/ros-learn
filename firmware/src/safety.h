#pragma once

// Call once in setup().
void safety_init();

// Check bumpers and forward distance. Must be called first in loop().
// Stops the robot and sends ESTOP on the first triggering event.
// Re-arms automatically once the hazard clears.
void safety_update();

// True while a hazard is present (bumper pressed or obstacle too close).
bool safety_is_triggered();

// Set a named runtime parameter. Returns true if name was recognised, false otherwise.
bool safety_set_param(const char *name, float val);
