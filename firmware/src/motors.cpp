#include <Arduino.h>
#include "motors.h"
#include "config/pins.h"
#include "config/params.h"

void motors_init() {
    pinMode(PIN_LEFT_DIR1,  OUTPUT);
    pinMode(PIN_LEFT_DIR2,  OUTPUT);
    pinMode(PIN_LEFT_PWM,   OUTPUT);
    pinMode(PIN_RIGHT_DIR1, OUTPUT);
    pinMode(PIN_RIGHT_DIR2, OUTPUT);
    pinMode(PIN_RIGHT_PWM,  OUTPUT);
    motors_stop();
}

static int16_t apply_deadband(int16_t speed) {
    if (speed > 0 && speed < PWM_DEADBAND_KINETIC) return PWM_DEADBAND_KINETIC;
    if (speed < 0 && speed > -PWM_DEADBAND_KINETIC) return -PWM_DEADBAND_KINETIC;
    return speed;
}

static void set_one(uint8_t dir1, uint8_t dir2, uint8_t pwm_pin, int16_t speed) {
    if (speed > 0) {
        digitalWrite(dir1, HIGH);
        digitalWrite(dir2, LOW);
    } else if (speed < 0) {
        digitalWrite(dir1, LOW);
        digitalWrite(dir2, HIGH);
        speed = -speed;
    } else {
        digitalWrite(dir1, LOW);
        digitalWrite(dir2, LOW);
    }
    analogWrite(pwm_pin, (uint8_t)speed);
}

void motors_set(int16_t left, int16_t right) {
    left  = apply_deadband(left);
    right = apply_deadband(right);
#if DEBUG_MOTORS
    Serial.print(F("DBG MOTORS L="));
    Serial.print(left);
    Serial.print(F(" R="));
    Serial.println(right);
#endif
    set_one(PIN_LEFT_DIR1,  PIN_LEFT_DIR2,  PIN_LEFT_PWM,  left);
    set_one(PIN_RIGHT_DIR1, PIN_RIGHT_DIR2, PIN_RIGHT_PWM, right);
}

void motors_stop() {
    motors_set(0, 0);
}
