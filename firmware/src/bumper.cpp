#include <Arduino.h>
#include <PCF8574.h>
#include "bumper.h"

// I2C address 0x24 — from hardware.md
static PCF8574 s_expander(0x24);

static bool s_left  = false;
static bool s_right = false;

void bumper_init() {
    // No pin-mode setup needed — PCF8574 pins default to INPUT with internal pull-ups.
    if (!s_expander.begin()) { Serial.println(F("ERR INIT BUMPER")); return; }

    Serial.println(F("OK INIT BUMPER"));
}

void bumper_update() {
    // read all 8 pins state
    uint8_t state = s_expander.read8();

    // extract bumper state bits
    s_left = !(state & 0x01);
    s_right = !(state & 0x02);
}

bool bumper_is_triggered() { return s_left || s_right; }
bool bumper_left()          { return s_left; }
bool bumper_right()         { return s_right; }
