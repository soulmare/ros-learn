#include <Arduino.h>
#include "heading.h"
#include "imu.h"
#include "encoders.h"
#include "pid.h"
#include "config/params.h"

static bool     s_active      = false;
static float    s_forward_vel = 0.0f;   // m/s forward component
static float    s_omega_dps   = 0.0f;   // deg/s angular rate (0 = straight)
static float    s_target_deg  = 0.0f;   // target heading (degrees), advanced each update when omega≠0
static PidState s_pid;
static uint32_t s_last_update_ms = 0;

void heading_init() {
    pid_init(&s_pid, HEADING_KP, HEADING_KI, HEADING_KD, HEADING_I_MAX);
    Serial.println(F("OK INIT HEADING"));
}

void heading_update() {
    if (!s_active) return;
    if (millis() - s_last_update_ms < PID_INTERVAL_MS) return;

    float dt_s = (millis() - s_last_update_ms) / 1000.0f;
    s_last_update_ms = millis();
    s_target_deg += s_omega_dps * dt_s;

    float heading_error = s_target_deg - imu_get_heading();
    float correction = pid_update(&s_pid, -heading_error, dt_s);

    // Feedforward: pre-spin wheels at the commanded turn rate so motors don't
    // stall waiting for error to build up when omega != 0.
    float omega_ff = s_omega_dps * (TRACK_WIDTH_M / 2.0f) / 57.2958f;

    // For pure rotation: clamp correction so it cannot reverse wheel direction.
    // A 0.01 m/s margin keeps setpoints from hitting exactly zero, which would
    // call motors_stop() inside encoders_set_velocity.
    // TODO: occasional forward runaway — robot overshoots and keeps spinning instead of
    // decelerating.  Root cause not yet identified.
    if (s_forward_vel == 0.0f && s_omega_dps != 0.0f) {
        const float kMargin = 0.01f;
        if (omega_ff > 0.0f)
            correction = constrain(correction, -(omega_ff - kMargin), HEADING_CORRECTION_MAX);
        else
            correction = constrain(correction, -HEADING_CORRECTION_MAX, -omega_ff - kMargin);
    } else {
        correction = constrain(correction, -HEADING_CORRECTION_MAX, HEADING_CORRECTION_MAX);
    }

    encoders_set_velocity(s_forward_vel - omega_ff - correction,
                          s_forward_vel + omega_ff + correction);

#if DEBUG_HEADING
    // tgt=target heading(deg) hdg=current(deg) err=error(deg)
    // KpE=Kp*err KiI=Ki*integral KdD=Kd*deriv c=velocity correction applied to each wheel(m/s)
    Serial.print(F("HDG tgt=")); Serial.print(s_target_deg, 1);
    Serial.print(F(" hdg="));   Serial.print(imu_get_heading(), 1);
    Serial.print(F(" err="));   Serial.print(heading_error, 2);
    Serial.print(F(" KpE="));   Serial.print(s_pid.kp * heading_error, 2);
    Serial.print(F(" KiI="));   Serial.print(s_pid.ki * s_pid.integral, 2);
    Serial.print(F(" KdD="));   Serial.print(s_pid.last_d, 2);
    Serial.print(F(" c="));     Serial.println(correction, 3);
#endif
}

void heading_set_velocity(float mps, float omega_dps) {
    s_forward_vel = mps;
    s_omega_dps   = omega_dps;
    if (mps == 0.0f && omega_dps == 0.0f) {
        s_active = false;
        encoders_set_velocity(0.0f, 0.0f);
        pid_reset(&s_pid);
        return;
    }
    s_target_deg = imu_get_heading();
    pid_reset(&s_pid);
    s_last_update_ms = millis();
    s_active = true;
}
