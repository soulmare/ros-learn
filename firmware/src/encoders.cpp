#include <Arduino.h>
#include "encoders.h"
#include "motors.h"
#include "pid.h"
#include "imu.h"
#include "config/pins.h"
#include "config/params.h"

// Meters travelled per encoder tick
#define METERS_PER_TICK  (2.0f * 3.14159265f * WHEEL_RADIUS_M / ENCODER_TICKS_PER_REV)

// --- ISR state (volatile: shared between ISR and main loop) ---
static volatile int32_t  s_left_ticks  = 0;
static volatile int32_t  s_right_ticks = 0;
// Direction flags set by encoders_set_velocity() so ISRs know the sign
static volatile int8_t   s_left_dir  = 1;   // +1 forward, -1 reverse
static volatile int8_t   s_right_dir = 1;

// Tick-timing state for velocity estimation
static volatile uint32_t s_left_tick_us      = 0;  // micros() of last left tick
static volatile uint32_t s_left_interval_us  = 0;  // µs between last two left ticks
static volatile uint32_t s_right_tick_us     = 0;
static volatile uint32_t s_right_interval_us = 0;

// --- Velocity estimator / PID cadence state ---
static uint32_t s_last_update_ms = 0;

static float s_left_vel  = 0.0f;   // m/s, updated each PID interval
static float s_right_vel = 0.0f;

// --- PID instances ---
static PidState s_pid_left;
static PidState s_pid_right;

// --- ISRs ---

static void isr_left() {
    static uint32_t last_us = 0;
    uint32_t now = micros();
    if (now - last_us < ENC_DEBOUNCE_US) return;
    s_left_interval_us = now - last_us;
    s_left_tick_us     = now;
    last_us            = now;
    if (s_left_dir > 0) s_left_ticks++;
    else if (s_left_dir < 0) s_left_ticks--;
}

static void isr_right() {
    static uint32_t last_us = 0;
    uint32_t now = micros();
    if (now - last_us < ENC_DEBOUNCE_US) return;
    s_right_interval_us = now - last_us;
    s_right_tick_us     = now;
    last_us             = now;
    if (s_right_dir > 0) s_right_ticks++;
    else if (s_right_dir < 0) s_right_ticks--;
}

// --- Public API ---

void encoders_init() {
    pid_init(&s_pid_left,  PID_KP, PID_KI, PID_KD, PID_I_MAX);
    pid_init(&s_pid_right, PID_KP, PID_KI, PID_KD, PID_I_MAX);

    attachInterrupt(digitalPinToInterrupt(PIN_ENC_LEFT),  isr_left,  RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_RIGHT), isr_right, RISING);

    s_last_update_ms = millis();
    Serial.println(F("OK INIT ENCODERS"));
}

void encoders_set_velocity(float left_mps, float right_mps) {
    s_pid_left.setpoint  = left_mps;
    s_pid_right.setpoint = right_mps;

    s_left_dir = (left_mps > 0.0f) ? 1 : (left_mps < 0.0f) ? -1 : s_left_dir;
    s_right_dir = (right_mps > 0.0f) ? 1 : (right_mps < 0.0f) ? -1 : s_right_dir;

    if (left_mps == 0.0f && right_mps == 0.0f) {
        motors_stop();
        pid_reset(&s_pid_left);
        pid_reset(&s_pid_right);
    }
}

void encoders_update() {
    uint32_t now    = millis();
    uint32_t dt_ms  = now - s_last_update_ms;
    if (dt_ms < PID_INTERVAL_MS) return;

    float dt_s = dt_ms / 1000.0f;
    s_last_update_ms = now;

    // Snapshot tick-timing state atomically
    noInterrupts();
    uint32_t left_tick_us       = s_left_tick_us;
    uint32_t left_interval_us   = s_left_interval_us;
    uint32_t right_tick_us      = s_right_tick_us;
    uint32_t right_interval_us  = s_right_interval_us;
    interrupts();

    uint32_t now_us = micros();

    if (now_us - left_tick_us > VEL_TIMEOUT_US) {
        s_left_vel = 0.0f;  // bypass filter on stop — no slow fade-out
    } else {
        float raw = (float)s_left_dir * METERS_PER_TICK / (left_interval_us / 1000000.0f);
        s_left_vel = ENC_VEL_ALPHA * raw + (1.0f - ENC_VEL_ALPHA) * s_left_vel;
    }

    if (now_us - right_tick_us > VEL_TIMEOUT_US) {
        s_right_vel = 0.0f;
    } else {
        float raw = (float)s_right_dir * METERS_PER_TICK / (right_interval_us / 1000000.0f);
        s_right_vel = ENC_VEL_ALPHA * raw + (1.0f - ENC_VEL_ALPHA) * s_right_vel;
    }

#if DEBUG_ENCODERS
    {
        static int32_t last_printed_left  = 0;
        static int32_t last_printed_right = 0;
        if (left_ticks != last_printed_left || right_ticks != last_printed_right) {
            // v    = current wheel velocity (m/s)
            // dist = cumulative distance since power-on (m), signed (negative = reversed)
            float left_dist  = left_ticks  * METERS_PER_TICK;
            float right_dist = right_ticks * METERS_PER_TICK;
            last_printed_left  = left_ticks;
            last_printed_right = right_ticks;
            Serial.print(F("ENC L v="));  Serial.print(s_left_vel, 3);
            Serial.print(F(" dist="));    Serial.print(left_dist, 3);
            Serial.print(F("  R v="));    Serial.print(s_right_vel, 3);
            Serial.print(F(" dist="));    Serial.println(right_dist, 3);
        }
    }
#endif

    // Skip PID when setpoint is zero — motors_stop() was already called in
    // encoders_set_velocity(); applying corrections here would fight that.
    if (s_pid_left.setpoint == 0.0f && s_pid_right.setpoint == 0.0f) return;

    // Average both wheels so encoder noise/asymmetry doesn't drive the PIDs apart.
    // IMU heading correction will handle yaw; this loop only controls forward speed.
    float vel_avg = (s_left_vel + s_right_vel) * 0.5f;

    // Use static deadband as minimum base when wheels are stopped, kinetic when already moving
    float min_pwm = (fabsf(vel_avg) < VEL_KINETIC_THRESHOLD) ? PWM_DEADBAND_STATIC : PWM_DEADBAND_KINETIC;

    float base_pwm_left = s_pid_left.setpoint * VEL_TO_PWM_SCALE;
    if (base_pwm_left  >  0 && base_pwm_left  <  min_pwm)  base_pwm_left  =  min_pwm;
    if (base_pwm_left  <  0 && base_pwm_left  > -min_pwm)  base_pwm_left  = -min_pwm;
    float pid_correction_left = pid_update(&s_pid_left, vel_avg, dt_s);
    int32_t left_pwm = constrain(base_pwm_left + pid_correction_left, -PWM_MAX, PWM_MAX);

    float base_pwm_right = s_pid_right.setpoint * VEL_TO_PWM_SCALE;
    if (base_pwm_right >  0 && base_pwm_right <  min_pwm)  base_pwm_right =  min_pwm;
    if (base_pwm_right <  0 && base_pwm_right > -min_pwm)  base_pwm_right = -min_pwm;
    float pid_correction_right = pid_update(&s_pid_right, vel_avg, dt_s);
    int32_t right_pwm = constrain(base_pwm_right + pid_correction_right, -PWM_MAX, PWM_MAX);

    motors_set(left_pwm, right_pwm);

#if VEL_REPORT
    Serial.print(F("VEL "));
    Serial.print(millis());
    Serial.print(F(" "));
    Serial.print(vel_avg, 3);
    Serial.print(F(" "));
    Serial.println(imu_get_heading(), 1);
#endif

#if DEBUG_PID
    // sp=target(m/s) vL/vR=per-wheel avg=shared PID input KpE/KiI/KdD=PID terms c=correction pwm=output
    Serial.print(F("SPD sp="));  Serial.print(s_pid_left.setpoint, 2);
    Serial.print(F(" vL=")); Serial.print(s_left_vel, 2);
    Serial.print(F(" vR=")); Serial.print(s_right_vel, 2);
    Serial.print(F(" vAvg=")); Serial.print(vel_avg, 2);
    Serial.print(F(" KpE=")); Serial.print(s_pid_left.kp * (s_pid_left.setpoint - vel_avg), 1);
    Serial.print(F(" KiI=")); Serial.print(s_pid_left.ki * s_pid_left.integral, 1);
    Serial.print(F(" KdD=")); Serial.print(s_pid_left.last_d, 1);
    Serial.print(F(" c="));   Serial.print(pid_correction_left, 1);
    Serial.print(F(" pwmL=")); Serial.print(left_pwm);
    Serial.print(F(" pwmR=")); Serial.println(right_pwm);
#endif
}

float encoders_get_left_velocity()  { return s_left_vel; }
float encoders_get_right_velocity() { return s_right_vel; }
