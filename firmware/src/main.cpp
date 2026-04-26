#include <Arduino.h>
#include "motors.h"
#include "encoders.h"
#include "serial_comms.h"

void setup() {
    motors_init();
    encoders_init();
    serial_init();
}

void loop() {
    serial_update();
    encoders_update();
}
