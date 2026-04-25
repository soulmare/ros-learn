# Glossary

Definitions of terms and abbreviations used throughout this project.

---

**ACK (Acknowledgement)** — a response sent to confirm a command was received and accepted. The opposite is NAK (Not Acknowledged), sent when a command fails or is invalid.

**analogWrite** — Arduino function that outputs a PWM signal on a pin. Takes a value 0–255 where 0 is always off and 255 is always on. Despite the name, the output is not a true analog voltage — it's a rapidly switching digital signal.

**atof** — C standard library function ("ASCII to float"). Converts a string like `"0.5"` to the floating-point number `0.5`. Used on Arduino instead of `sscanf("%f")` because AVR-libc strips float support from scanf by default.

**ATmega328P** — the microcontroller chip on the Arduino Uno. Made by Microchip (formerly Atmel). Has 32 KB flash, 2 KB SRAM, and runs at 16 MHz.

**AVR** — the microcontroller architecture used by Arduino Uno (ATmega328P). Also refers to the toolchain and standard library (avr-libc) that compiles Arduino firmware.

**avr-libc** — the C standard library for AVR microcontrollers. It omits some features present on desktop systems (e.g. float support in sscanf) to save flash and RAM.

**Closed-loop control** — a control system that measures its own output and adjusts to reduce the difference from the target. Opposite of open-loop. Example: PID speed control using encoder feedback.

**Colcon** — the build tool used by ROS2. Compiles all packages in a workspace with a single command.

**constrain(x, low, high)** — Arduino macro that clamps a value to a range. Returns `low` if `x < low`, `high` if `x > high`, otherwise `x`.

**Cooperative state machine** — a concurrency pattern for microcontrollers without an RTOS. Each module does a small slice of work per `loop()` iteration and returns immediately, allowing all modules to share the CPU fairly. The opposite would be blocking (one module holds the CPU while waiting).

**Deadband** — the range of PWM values too small to overcome motor static friction. Any command in this range produces no movement. In our firmware, values below `PWM_DEADBAND` are snapped up to the minimum effective value.

**Differential drive** — a drive system where left and right wheel speeds are controlled independently. Straight motion requires equal speeds; turning is achieved by speeding up one side relative to the other.

**Encoder** — a sensor attached to a wheel axle that produces pulses as the wheel turns. Used to measure how far and how fast the wheel has moved. Our encoders produce 40 pulses per full wheel revolution.

**F() macro** — Arduino macro that stores a string literal in flash memory instead of SRAM. Usage: `Serial.print(F("text"))`. Without it, all string literals are copied into SRAM at boot, quickly exhausting the Uno's 2 KB.

**Flash** — non-volatile memory on the Arduino that stores the program. 32 KB on the Uno. Survives power loss. Slower to read than SRAM but much larger.

**H-bridge** — a motor driver circuit that can apply voltage in either direction across a motor, allowing forward and reverse rotation. Controlled via direction pins (DIR1, DIR2) and a PWM pin for speed.

**I2C** — a two-wire serial communication protocol (SDA + SCL) for connecting peripherals to a microcontroller. Supports multiple devices on the same two wires, each identified by a unique address. Used by our IMU (MPU-6050) and bumper expander (PCF8574).

**IMU (Inertial Measurement Unit)** — a sensor that measures motion and orientation. Our MPU-6050 provides accelerometer and gyroscope data; we use the gyroscope Z axis to track yaw (rotation around the vertical axis).

**INT0 / INT1** — the two hardware interrupt input lines on the Arduino Uno, mapped to pins 2 and 3 respectively. Only these two pins can trigger an ISR on a rising or falling edge.

**Integral windup** — a PID problem where the integral term accumulates to a very large value while the output is saturated (e.g. motor at full power but not yet at setpoint). When the error finally reduces, the large integral causes the output to overshoot badly. Fixed by clamping the integral term.

**ISR (Interrupt Service Routine)** — a function that the CPU calls automatically when a hardware event occurs, interrupting whatever the main program was doing. Must be short, must not use Serial or delay(), and all variables shared with the main program must be declared `volatile`.

**millis()** — Arduino function that returns the number of milliseconds since the board started. Driven by a hardware timer, so it stays accurate regardless of how busy the main loop is. Returns a `uint32_t` (overflows after ~49 days).

**NAK** — see ACK.

**Odometry** — estimating a robot's position (x, y, heading) by integrating wheel movement over time. Accumulates error over distance but requires no external sensors.

**Open-loop control** — a control system that sends commands without measuring the result. Example: sending a fixed PWM and assuming a fixed speed. Simple but inaccurate — motor speed varies with load, battery voltage, and friction.

**PID (Proportional-Integral-Derivative)** — a feedback control algorithm. Computes a correction from three terms: P (proportional to current error), I (proportional to accumulated error over time), D (proportional to rate of change of error). Together they reduce error, eliminate steady-state offset, and dampen oscillation.

**PWM (Pulse Width Modulation)** — a technique for simulating a variable voltage using a digital pin that rapidly switches on and off. The duty cycle (fraction of time the pin is HIGH) controls the effective voltage. Used to control motor speed.

**Quadrature encoder** — a two-channel encoder that can detect both speed and direction of rotation from the encoder alone, by comparing the phase of two pulse signals. Our encoders are single-channel (one signal only) — direction is inferred from the motor command.

**ROS2 (Robot Operating System 2)** — a middleware framework for robot software. Provides a message-passing system between nodes, a build system, tools for visualization and logging, and a large library of standard packages.

**RTOS (Real-Time Operating System)** — an operating system designed to guarantee that tasks run within strict time deadlines. Not used on our Arduino — instead we use a cooperative state machine pattern in the main loop.

**Setpoint** — the desired target value in a control system. The PID controller works to make the measured value match the setpoint. In our case: the desired wheel speed in m/s.

**SRAM (Static Random Access Memory)** — the working memory on the Arduino where variables, the stack, and the heap live. Only 2 KB on the Uno. Lost when power is removed. Much faster than flash but very limited in size.

**strchr(str, ch)** — C standard library function. Scans a string and returns a pointer to the first occurrence of character `ch`, or NULL if not found. Does not modify the string.

**strtok(str, delimiters)** — C standard library function. Splits a string into tokens by replacing delimiter characters with `\0` and returning one token per call. First call takes the string; subsequent calls pass NULL to continue. Modifies the original buffer in place and uses a hidden global variable, so only one tokenisation sequence can be active at a time.

**Tick** — a single pulse from an encoder. Each tick represents the wheel turning by `(2π × radius) / ticks_per_rev` metres. Our wheels have 40 ticks per revolution and a 34 mm radius, so one tick ≈ 5.34 mm.

**TF2** — the ROS2 transform library. Tracks the geometric relationship between coordinate frames over time (e.g. `odom` → `base_link`). Used by visualisation tools and navigation algorithms.

**volatile** — a C/C++ keyword that prevents the compiler from caching a variable in a CPU register. Required for any variable shared between an ISR and the main program, because without it the compiler may never re-read the variable from RAM and the main loop will always see a stale value.
