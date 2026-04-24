# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A step-by-step learning project building a **semi-autonomous 4WD rover** with two distinct software layers. Developer skill levels are documented in `SKILLS.md` — use it to calibrate explanation depth and code complexity.

1. **Arduino firmware** (low-level, real-time) — closed-loop motor control, gyro-based straight-line stabilization and precise turning, reactive obstacle/bumper safety.
2. **ROS2 on Raspberry Pi 4** (high-level brain) — perception, waypoint path following, and behavioral decision-making.

The Arduino handles all time-critical physical control; the Raspberry Pi runs ROS2 and sends high-level commands over serial. Hardware is fully documented in `HARDWARE.md` — consult it for all pin assignments, I2C addresses, sensor parameters, and library versions before writing any Arduino code. The full 10-phase learning and implementation roadmap is in `LEARNING-PLAN.md`.

## Build System

The firmware uses **PlatformIO**. Common commands (once a `platformio.ini` is present):

```bash
pio run                  # compile firmware
pio run --target upload  # compile and flash to Arduino Uno
pio run --target clean   # clean build artifacts
pio test                 # run unit tests (PlatformIO test framework)
pio device monitor --baud 115200  # open serial monitor
```

## Hardware Summary

- **Arduino Uno** (ATmega328P) — real-time firmware, serial at 115200 baud
- **Raspberry Pi 4** — ROS2 host, connects to Arduino over serial
- **Drive:** 4WD, four DC motors wired in left/right pairs sharing H-bridge channels (Arduino sees two logical motors), single-channel encoders (40 ticks/rev, 34 mm wheel radius)
- **Scanning:** HC-SR04 ultrasonic sensor on a servo (pin 8, forward = 81°, range 5°–165°)
- **IMU:** MPU-6050 at I2C 0x68 (yaw integration, 21 Hz filter, 1 s startup calibration)
- **Bumpers:** Two collision switches via PCF8574 I/O expander at I2C 0x24, active LOW
- **I2C bus:** A4 (SDA) / A5 (SCL), shared by MPU-6050 and PCF8574

## Libraries

| Library | Purpose |
|---------|---------|
| Adafruit MPU6050 (≥ 2.2.6) | IMU driver |
| Adafruit Unified Sensor | Sensor abstraction (Adafruit dependency) |
| PCF8574 (RobTillaart) | Bumper switch I/O expander |
| Servo (built-in) | Servo PWM control |
| Wire (built-in) | I2C communication |

## Arduino Firmware Structure

Each hardware subsystem gets its own `.cpp`/`.h` pair. `main.cpp` contains only `setup()` and `loop()` — no logic, no magic numbers. Pin assignments and all tunable constants (PID gains, thresholds, timing) live in dedicated config headers; no raw numbers anywhere else in the codebase.

Prefer C-style modules over OOP: each subsystem exposes a set of functions and keeps its state in `static` variables. Use a plain `struct` to group related state when it improves readability. Avoid virtual methods and dynamic allocation (`new`/`delete`) — both are hazardous on a 2 KB RAM chip.

String literals must use the `F()` macro (`Serial.print(F("text"))`) to store them in flash instead of RAM.

## Conventions

- If you need to ask a list of questions - ask them one by one instead of giving a full list at once. Always mention total count of questions and current question number.
- Before starting a new phase, ask review questions on the previous phase's material to reinforce learning.
- When introducing a topic known for gotchas (ISRs, I2C, PID windup, floating point on Arduino, etc.), proactively flag the most common beginner mistake before starting implementation.
- Any architectural or design decision must be explicitly presented, explained, and confirmed by the user before proceeding. Never assume a decision is approved because the user moved to the next step.