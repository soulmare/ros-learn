# Learning & Implementation Plan

Step-by-step roadmap for building the semi-autonomous 4WD rover. Each phase ends with a working, testable milestone before moving on.

---

## Phase 1 — Arduino: Open-Loop Drive & Serial Protocol

**Goal:** Get motors moving and establish the serial command interface that all future layers build on.

**Estimated time:** 10–15 h

### Learn
1. PlatformIO project setup (ini file, folder structure, uploading via VS Code extension)
2. H-bridge motor driver wiring and PWM control on Arduino
3. Text-based serial protocol design: line framing, command parsing, ACK/NAK responses

### Implement
1. `platformio.ini` configured for Arduino Uno
2. Motor driver abstraction: `setMotors(leftSpeed, rightSpeed)` with direction logic
3. Serial command parser: receive text commands from Pi, send text responses
4. Design and document the **serial protocol** (text-based, all command types for all phases: drive commands, sensor telemetry, parameter updates) — lock it down here to avoid breaking changes later

### Milestone
Pi (or laptop) sends a drive command over serial → robot moves, stops, turns on command. Protocol spec written in `docs/serial-protocol.md`.

---

## Phase 2 — Arduino: Encoders & Closed-Loop Speed Control

**Goal:** Replace open-loop PWM guessing with measured wheel speed and a PID controller.

**Estimated time:** 10–15 h

### Learn
1. Interrupt-driven encoder reading on Arduino (pins 2 & 3 are INT0/INT1)
2. Velocity estimation from tick deltas
3. PID tuning on a real system (you have prior experience — build on it)

### Implement
1. Encoder ISRs with 200 µs debounce
2. Velocity estimator (ticks/s → m/s using 40 ticks/rev, r = 34 mm)
3. Per-wheel speed PID: setpoint from serial commands, output to PWM
4. Report actual wheel velocities back over serial

### Milestone
Command `SET_VEL 0.2 0` → robot drives straight at ~0.2 m/s, measured speed converges to setpoint. Encoders report back to Pi.

---

## Phase 3 — Arduino: IMU & Heading Stabilization

**Goal:** Drive in a straight line and execute precise angle turns using the gyroscope.

**Estimated time:** 10–15 h

### Learn
1. I2C on Arduino (`Wire.h`), scanning the bus
2. Adafruit MPU-6050 library: reading gyro Z for yaw rate
3. Yaw integration and drift accumulation
4. Parallel PID for heading hold: heading error → differential speed correction applied on top of per-wheel setpoints

### Implement
1. MPU-6050 init with 1 s calibration on startup
2. Yaw angle integrator at 21 Hz filter bandwidth
3. Heading-hold PID: corrects differential speed to maintain target heading during straight drive
4. Angle-turn mode: rotate to a target heading with a dedicated PID
5. Publish current heading over serial

### Milestone
`SET_VEL 0.3` → robot drives straight with minimal drift (heading held by IMU PID). `SET_VEL 0 30` → robot rotates at 30°/s; Pi stops it at 90° ± ~3°.

---

## Phase 4 — Arduino: Sensing & Reactive Safety

**Goal:** The robot must stop itself when it's about to hit something, independent of Pi commands.

**Estimated time:** 8–12 h

### Learn
1. HC-SR04 timing (trigger pulse, echo duration → distance)
2. PCF8574 I2C I/O expander (`PCF8574` library, reading active-LOW pins)
3. Servo control (`Servo.h`, mapping angles)
4. Interrupt-safe state machines on Arduino

### Implement
1. HC-SR04 driver: blocking pulse measurement with 15 000 µs timeout
2. Servo sweep: scan forward arc, report distance + angle over serial
3. PCF8574 bumper reader: poll both switches each loop
4. Safety layer: if obstacle < threshold OR bumper triggered → immediately stop motors, send `ESTOP OBSTACLE` or `ESTOP BUMPER` event over serial regardless of current command

### Milestone
Robot drives toward a wall → stops autonomously before contact. Bumper hit → stops and reports. Servo scan data streams to Pi over serial.

---

## Phase 5 — ROS2 Installation & Core Concepts

**Goal:** Understand ROS2 well enough to write nodes before connecting anything to the robot.

**Development approach:** Phases 5–7 run on the Ubuntu 24.04 desktop with the Arduino connected over USB. The Raspberry Pi 5 is introduced in Phase 8 once the ROS2 code is stable and autonomous features are ready to test on the robot untethered.

**Estimated time:** 20–30 h

### Learn
1. ROS2 Jazzy installation on Ubuntu 24.04 (desktop for Phases 5–7; RPi 5 for Phase 8+)
2. Core concepts: nodes, topics, messages, services, parameters
3. `colcon` build tool and workspace structure (`src/`, `install/`, overlay)
4. Python ROS2 package structure: `package.xml`, `setup.py`, entry points
5. Writing a Python ROS2 node from scratch
6. `ros2` CLI: `topic echo`, `node list`, `param get`, `bag record`

### Implement
1. ROS2 workspace: `ros2_ws/` inside the project directory
2. "Hello robot" node: publishes a `std_msgs/String` on a timer, no hardware
3. Practice launching with `ros2 run` and a `.launch.py` file

### Milestone
Two Python nodes running: one publishes, one subscribes and logs. Launch file starts both. No hardware involved yet.

---

## Phase 6 — ROS2: Arduino Serial Bridge Node

**Goal:** Replace the laptop terminal with a ROS2 node that owns the serial connection.

**Estimated time:** 18–25 h

### Learn
1. `pyserial` in a ROS2 node (non-blocking reads in a timer or thread)
2. Standard ROS2 message types: `geometry_msgs/Twist`, `nav_msgs/Odometry`, `sensor_msgs/Range`
3. Differential drive kinematics: integrating left/right wheel velocities into x, y, θ pose
4. TF2 basics: `base_link` → `odom` transform

### Implement
1. `arduino_bridge` ROS2 package:
   - Subscribes to `/cmd_vel` (`Twist`) → encodes and sends serial command to Arduino
   - Reads serial responses → publishes:
     - `/odom` (`Odometry`) — computed from encoder ticks
     - `/scan_range` (`Range`) — ultrasonic distance
     - `/bumper` (`std_msgs/Bool`) — collision state
   - Publishes `/tf` odom→base_link transform
2. Odometry math: integrate wheel velocities → x, y, θ

### Milestone
`ros2 topic pub /cmd_vel geometry_msgs/Twist ...` → robot moves. `ros2 topic echo /odom` → position updates as robot drives.

---

## Phase 7 — ROS2: Teleop & Manual Testing

**Goal:** Drive the robot manually through ROS2 before adding any autonomy. Flush out integration bugs.

**Estimated time:** 10–15 h

### Learn
1. `teleop_twist_keyboard` package
2. `rqt` and `rviz2` for visualization
3. `ros2 bag` for recording and replaying sensor data
4. ROS2 parameters for runtime tuning (PID gains, thresholds)

### Implement
1. Bring-up launch file: starts `arduino_bridge` + teleop
2. `rviz2` config: display odometry path and range sensor
3. Expose Arduino PID gains as ROS2 parameters; tune via `ros2 param set` without reflashing (requires adding a `SET_PARAM` command to the serial protocol defined in Phase 1)

### Milestone
Full manual control from keyboard over ROS2. Odom path visible in rviz2. Can record and replay a drive session with `ros2 bag`.

---

## Phase 8 — ROS2: Waypoint Path Following

**Goal:** Robot drives autonomously to a sequence of (x, y, θ) waypoints using odometry.

**Estimated time:** 15–20 h

### Learn
1. Go-to-goal controller: rotate to face target, drive until within acceptance radius, repeat for next waypoint
2. `geometry_msgs/PoseStamped` for representing a target waypoint

### Implement
1. `waypoint_follower` node: go-to-goal with proportional heading and speed control
2. Waypoints loaded from a YAML file
3. Node transitions: IDLE → DRIVING → ARRIVED → next waypoint
4. Integrate collision stop: if `bumper` or `scan_range` below threshold → pause navigation

### Milestone
Robot autonomously visits 3 waypoints in sequence in an open space. Stops if obstacle detected mid-route, resumes when cleared.

---

## Phase 9 — ROS2: Reactive Obstacle Avoidance

**Goal:** Robot navigates around obstacles it encounters, not just stops.

**Estimated time:** 15–20 h

### Learn
1. Heading-cone obstacle check: detecting obstacles within a forward arc and computing a steering correction
2. `sensor_msgs/LaserScan`: assembling a full scan array from sequential servo sweep readings

### Implement
1. Upgrade servo scan publisher: accumulate sweep readings into a `LaserScan` message
2. Avoidance layer in `waypoint_follower`: if obstacle detected in heading cone → steer away; resume course when clear

### Milestone
Robot navigates a simple obstacle course (2–3 static obstacles) to reach a waypoint without manual intervention.

---

## Phase 10 — Integration, Tuning & Documentation

**Goal:** Reliable full-system operation; project is demonstrable.

**Estimated time:** 8–12 h

### Implement
1. Deploy ROS2 workspace to RPi 5; systemd service: auto-start ROS2 bridge on boot
2. Full bring-up launch file: Arduino bridge + waypoint follower + rviz2
3. End-to-end test: power on → robot ready for waypoint commands within 30 s
4. Tune all PID gains and safety thresholds
5. Write `README.md` with setup instructions and demo

### Milestone
Robot completes a defined test course autonomously from cold start. Project is documented and reproducible.

---

## Suggested Tools & References

| Tool / Resource | Purpose |
|-----------------|---------|
| PlatformIO IDE extension (VS Code) | Arduino development |
| ROS2 Humble docs (docs.ros.org) | ROS2 reference |
| `ros2 bag` | Recording sensor data for offline debugging |
| `rviz2` | Visualizing odometry, transforms, sensor data |
| `rqt_plot` | Real-time plotting of PID signals |
