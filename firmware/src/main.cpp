#include <Arduino.h>
#include "motors.h"
#include "encoders.h"
#include "imu.h"
#include "heading.h"
#include "ultrasonic.h"
#include "scanner.h"
#include "bumper.h"
#include "safety.h"
#include "serial_comms.h"

void setup() {
    serial_init();      // first — Serial must be ready before other inits print messages
    motors_init();
    encoders_init();
    imu_init();         // blocks for IMU_CALIBRATION_MS; calls Wire.begin()
    heading_init();
    ultrasonic_init();
    scanner_init();     // moves servo to forward position; blocks ~300 ms
    bumper_init();      // must come after imu_init() — Wire already running
    safety_init();
}

void loop() {
    safety_update();    // first — preempts all motion if hazard detected
    serial_update();
    imu_update();
    heading_update();
    encoders_update();
    scanner_update();   // runs even during ESTOP (scan continues per protocol)
}
