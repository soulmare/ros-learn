# ROS2 Rover

A step-by-step learning project building a semi-autonomous 4WD rover.

## Architecture

Two software layers:

- **Arduino Uno** — real-time firmware: closed-loop motor control, gyro-based stabilization, reactive obstacle/bumper safety
- **Raspberry Pi 5** — ROS2 Jazzy brain: odometry, waypoint path following, obstacle avoidance

The Arduino handles all time-critical physical control. The Pi sends high-level commands over serial (115200 baud, text-based protocol).

## Repository Layout

| Path | Contents |
|------|----------|
| `firmware/` | Arduino firmware (PlatformIO project) |
| `firmware/src/` | C++ source — one `.cpp`/`.h` pair per subsystem |
| `firmware/src/config/` | Pin assignments and tunable parameters |
| `ros2_ws/src/` | ROS2 packages (Python) |
| `docs/` | Hardware reference and serial protocol spec |
| `tools/` | Desktop test scripts (Python, talk to Arduino over serial) |

## Key Documents

| Document | Purpose |
|----------|---------|
| `docs/hardware.md` | Pin assignments, I2C addresses, sensor parameters, libraries |
| `docs/serial-protocol.md` | Arduino ↔ ROS2 serial message format |
| `LEARNING-PLAN.md` | 10-phase implementation roadmap with time estimates |
| `SKILLS.md` | Developer skill levels (used to calibrate AI assistance) |

## Stack

- **Firmware:** C++, PlatformIO
- **ROS2:** Python, ROS2 Jazzy on Ubuntu 24.04
