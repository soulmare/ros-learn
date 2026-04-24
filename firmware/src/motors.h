#pragma once

void motors_init();
void motors_set(int16_t left, int16_t right);  // -255 to 255
void motors_stop();
