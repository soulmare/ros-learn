#include <Arduino.h>
#include "motors.h"
#include "encoders.h"
#include "imu.h"
#include "heading.h"
#include "serial_comms.h"

void setup() {
    serial_init();     // first — Serial must be ready before other inits print messages
    motors_init();
    encoders_init();
    imu_init();        // blocks for IMU_CALIBRATION_MS while calibrating gyro bias
    heading_init();
}

void loop() {
    serial_update();
    imu_update();
    heading_update();
    encoders_update();
}
