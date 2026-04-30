#include <Arduino.h>
#include "safety.h"
#include "bumper.h"
#include "scanner.h"
#include "heading.h"
#include "config/params.h"

static bool s_triggered = false;

void safety_init() {
    Serial.println(F("OK INIT SAFETY"));
}

void safety_update() {
    bumper_update();

    bool bumper_hit = bumper_is_triggered();

    // Forward distance: use the scanner's most recent forward reading.
    // -1.0f means no valid reading yet — treat as safe (don't false-trigger on startup).
    float fwd = scanner_forward_distance_m();
    bool obstacle = (fwd >= 0.0f && fwd < OBSTACLE_THRESHOLD_M);

    bool hazard = bumper_hit || obstacle;

    if (hazard && !s_triggered) {
        // Transition into ESTOP: stop motors and notify Pi.
        heading_set_velocity(0.0f, 0.0f);

        if (bumper_hit) Serial.println(F("ESTOP BUMPER"));
        else            Serial.println(F("ESTOP OBSTACLE"));

        s_triggered = true;
    } else if (!hazard && s_triggered) {
        // Hazard cleared — re-arm so the safety layer can trigger again if needed.
        // The robot stays stopped; the Pi must send a new SET_VEL to resume motion.
        s_triggered = false;
    }
}

bool safety_is_triggered() { return s_triggered; }
