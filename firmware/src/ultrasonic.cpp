#include <Arduino.h>
#include "ultrasonic.h"
#include "config/pins.h"
#include "config/params.h"

void ultrasonic_init() {
    pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ECHO, INPUT);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    Serial.println(F("OK INIT ULTRASONIC"));
}

float ultrasonic_measure_m() {
    // Clean low → 10 µs high → low trigger pulse
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

    // pulseIn waits for the echo pin to go HIGH, measures how long it stays HIGH,
    // and returns that duration in microseconds (0 on timeout).
    uint32_t duration_us = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);
    if (duration_us == 0) return -1.0f;

    // Speed of sound ≈ 343 m/s; pulse travels to obstacle and back, so divide by 2.
    return duration_us * 0.000343f / 2.0f;
}
