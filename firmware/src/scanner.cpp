#include <Arduino.h>
#include <Servo.h>
#include "scanner.h"
#include "ultrasonic.h"
#include "config/pins.h"
#include "config/params.h"

static Servo    s_servo;
static bool     s_busy         = false;  // true while a sweep is in progress
static float    s_from_deg     = 0.0f;   // sweep start angle in servo space (degrees)
static float    s_to_deg       = 0.0f;   // sweep end angle in servo space (degrees)
static float    s_current_deg  = 0.0f;   // angle being measured this step, servo space
static float    s_step_deg     = SCANNER_STEP_DEG;  // +deg for forward sweep, -deg for reverse
static float    s_last_dist_m  = -1.0f;  // most recent distance reading (any angle); -1 = none
static float    s_fwd_dist_m   = -1.0f;  // most recent reading near SCANNER_SERVO_FORWARD_DEG; -1 = none
static uint32_t s_last_step_ms = 0;      // millis() timestamp of the last step
static float    s_servo_pos_deg = -1.0f; // last angle written to servo; -1 = unknown

// Move servo to deg (servo space) only if it is not already there.
// Attaches on demand so the servo is powered only while moving — prevents idle buzzing.
static void servo_move_to(float deg) {
    if (fabsf(deg - s_servo_pos_deg) < 0.5f) return;
    if (!s_servo.attached()) s_servo.attach(PIN_SERVO);
    s_servo.write((int)deg);
    s_servo_pos_deg = deg;
    if (DEBUG_SCANNER) {
        Serial.print(F("DBG SERVO "));
        Serial.println(deg, 1);
    }
}

void scanner_init() {
    servo_move_to(SCANNER_SERVO_FORWARD_DEG);
    delay(300);   // worst-case travel time 0°→165° at ~60°/100ms; acceptable in setup()
    s_servo.detach();
    Serial.println(F("OK INIT SCANNER"));
}

void scanner_update() {
    if (!s_busy) return;
    if (millis() - s_last_step_ms < SCANNER_STEP_MS) return;
    s_last_step_ms = millis();

    servo_move_to(s_current_deg);

    s_last_dist_m = ultrasonic_measure_m();

    // Report in robot-frame degrees (0 = forward) — per serial-protocol.md
    Serial.print(F("SCAN "));
    Serial.print(millis());
    Serial.print(F(" "));
    Serial.print(s_current_deg - SCANNER_SERVO_FORWARD_DEG, 1);
    Serial.print(F(" "));
    Serial.println(s_last_dist_m, 2);

    if (fabsf(s_current_deg - SCANNER_SERVO_FORWARD_DEG) <= SCANNER_STEP_DEG) {
        s_fwd_dist_m = s_last_dist_m;
    }

    s_current_deg += s_step_deg;

    if ((s_step_deg > 0 && s_current_deg > s_to_deg) || (s_step_deg < 0 && s_current_deg < s_to_deg)) {
        s_busy = false;
        if (s_from_deg != s_to_deg) {
            servo_move_to(SCANNER_SERVO_FORWARD_DEG);
        }
        if (s_servo.attached()) s_servo.detach();
        Serial.println(F("DONE SCAN"));
    }
}

// from_deg and to_deg are robot-frame angles: 0 = forward, positive = left, negative = right (ROS convention).
void scanner_start(float from_deg, float to_deg) {
    float from_servo = SCANNER_SERVO_FORWARD_DEG + from_deg;
    float to_servo   = SCANNER_SERVO_FORWARD_DEG + to_deg;
    s_from_deg    = constrain(from_servo, SCANNER_SERVO_MIN_DEG, SCANNER_SERVO_MAX_DEG);
    s_to_deg      = constrain(to_servo,   SCANNER_SERVO_MIN_DEG, SCANNER_SERVO_MAX_DEG);
    s_current_deg = s_from_deg;
    s_step_deg    = (s_to_deg >= s_from_deg) ? SCANNER_STEP_DEG : -SCANNER_STEP_DEG;
    s_busy        = true;
    s_last_step_ms = millis();
    servo_move_to(s_from_deg);
    Serial.println(F("OK SCAN"));
}

void scanner_start_full() {
    scanner_start(SCANNER_SERVO_MIN_DEG - SCANNER_SERVO_FORWARD_DEG,
                  SCANNER_SERVO_MAX_DEG - SCANNER_SERVO_FORWARD_DEG);
}
void scanner_start_single(float angle_deg) { scanner_start(angle_deg, angle_deg); }

void scanner_stop() {
    if (!s_busy) return;
    s_busy = false;
    servo_move_to(SCANNER_SERVO_FORWARD_DEG);
    if (s_servo.attached()) s_servo.detach();
    Serial.println(F("ERR SCAN interrupted"));
}

bool  scanner_is_busy()            { return s_busy; }
float scanner_last_distance_m()    { return s_last_dist_m; }
float scanner_forward_distance_m() { return s_fwd_dist_m; }
