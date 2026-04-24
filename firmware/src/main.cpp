#include <Arduino.h>
#include "motors.h"
#include "serial_comms.h"

void setup() {
    motors_init();
    serial_init();
}

void loop() {
    serial_update();
}
