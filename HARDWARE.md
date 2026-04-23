# Robot Hardware Reference

## Platform

- **Board:** Arduino Uno (ATmega328P)
- **Build system:** PlatformIO  
- **Serial baud rate:** 115200

---

## Hardware Components

### Motors

Two DC motors controlled via an H-bridge motor driver.

| Motor | Dir Pin 1 | Dir Pin 2 | PWM Enable |
|-------|-----------|-----------|------------|
| Left  | 9         | 7         | 6          |
| Right | 10        | 4         | 5          |

### Wheel Encoders

Single-channel encoders on front wheels.

| Encoder | Pin |
|---------|-----|
| Left    | 3   |
| Right   | 2   |

- Ticks per revolution: **40**
- Debounce: **200 µs**
- Wheel radius: **34 mm**

### Servo Motor

Rotates the ultrasonic sensor for directional distance scanning.
"Initial angle" means looking strictly forward.

| Signal Pin | Initial Angle | Min Angle | Max Angle |
|------------|---------------|-----------|-----------|
| 8          | 81°           | 5°        | 165°      |

### Ultrasonic Distance Sensor (HC-SR04)

Measures distance to obstacles; mounted on the servo for scanning.

| Trigger Pin | Echo Pin | Echo Timeout |
|-------------|----------|--------------|
| 11          | 12       | 15 000 µs    |

### MPU-6050 Gyroscope / Accelerometer

6-axis IMU used for yaw angle integration and rotation tracking.

- **I2C address:** 0x68
- **Filter bandwidth:** 21 Hz
- **Calibration duration:** 1 000 ms at startup

### Bumper Switches (×2)

Collision switches on front bumper connected through a PCF8574 I/O expander.
Usually work together, but on some side collisions only one switch may be triggered.

| Bumper | Expander Pin |
|--------|--------------|
| Left   | 0            |
| Right  | 1            |

Active LOW — pulled HIGH by the expander's internal pull-ups.

### PCF8574 I/O Expander

8-bit I2C I/O expander used to read the bumper switches.
Can be used for connecting additional hardware.

- **I2C address:** 0x24

---

## Arduino Pin Map

| Pin    | Direction | Function              | Component          |
|--------|-----------|-----------------------|--------------------|
| 2      | INPUT     | Right encoder (INT0)  | Wheel encoder      |
| 3      | INPUT     | Left encoder (INT1)   | Wheel encoder      |
| 4      | OUTPUT    | Right motor dir 2     | Motor driver       |
| 5      | OUTPUT    | Right motor PWM       | Motor driver       |
| 6      | OUTPUT    | Left motor PWM        | Motor driver       |
| 7      | OUTPUT    | Left motor dir 2      | Motor driver       |
| 8      | OUTPUT    | Servo signal          | Servo motor        |
| 9      | OUTPUT    | Left motor dir 1      | Motor driver       |
| 10     | OUTPUT    | Right motor dir 1     | Motor driver       |
| 11     | OUTPUT    | Ultrasonic trigger    | HC-SR04            |
| 12     | INPUT     | Ultrasonic echo       | HC-SR04            |
| A4/SDA | I2C       | I2C data              | MPU-6050, PCF8574  |
| A5/SCL | I2C       | I2C clock             | MPU-6050, PCF8574  |

---

## Libraries

| Library | Version | Purpose |
|---------|---------|---------|
| [Adafruit MPU6050](https://github.com/adafruit/Adafruit_MPU6050) | ≥ 2.2.6 | MPU-6050 gyro/accel driver |
| [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor) | dependency | Sensor abstraction layer for Adafruit drivers |
| [Servo](https://www.arduino.cc/reference/en/libraries/servo/) | built-in | PWM servo control |
| [PCF8574 (robtillaart)](https://github.com/RobTillaart/PCF8574) | — | PCF8574 I/O expander driver |
| [Wire](https://www.arduino.cc/reference/en/language/functions/communication/wire/) | built-in | I2C communication |
