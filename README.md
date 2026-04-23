# ROS2 Rover

A step-by-step learning project building a semi-autonomous 4WD rover.

## Architecture

Two software layers:

- **Arduino Uno** — real-time firmware: closed-loop motor control, gyro-based stabilization, reactive obstacle/bumper safety
- **Raspberry Pi 4** — ROS2 Humble brain: odometry, waypoint path following, obstacle avoidance

The Arduino handles all time-critical physical control. The Pi sends high-level commands over serial (115200 baud, text-based protocol).

## Key Documents

| Document | Purpose |
|----------|---------|
| `HARDWARE.md` | Pin assignments, I2C addresses, sensor parameters, libraries |
| `LEARNING-PLAN.md` | 10-phase implementation roadmap with time estimates |
| `SKILLS.md` | Developer skill levels (used to calibrate AI assistance) |

## Stack

- **Firmware:** C++, PlatformIO
- **ROS2:** Python, ROS2 Humble on Ubuntu 22.04
