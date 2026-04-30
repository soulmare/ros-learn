# Arduino ↔ ROS2 Serial Protocol

Text-based, one message per line (`\n` terminated), 115200 baud.

---

## Commands — Pi → Arduino

| Message | Arguments | Response |
|---------|-----------|----------|
| `SET_VEL v [ω]` | floats, m/s and °/s; ω defaults to 0 if omitted | none |
| `STOP` | — | `OK STOP` |
| `SET_PARAM NAME VAL` | string, float | `OK SET_PARAM` or `ERR SET_PARAM msg` |
| `SCAN` | — | full sweep across the sensor's range |
| `SCAN from to` | float°, float° | partial sweep between two robot-frame angles |
| `SCAN angle` | float° | single point reading at one robot-frame angle |
| `SCAN_STOP` | — | aborts active scan → `ERR SCAN interrupted` |

All `SCAN` variants: `OK SCAN` on start, `SCAN` telemetry lines during sweep, `DONE SCAN` on completion.

Angles are in **robot frame**: 0° = straight ahead, positive = left, negative = right (ROS convention — right-hand rule around Z axis).
The Arduino translates to servo hardware angles internally — the Pi never needs to know the servo's mounting offset.

---

## Telemetry — Arduino → Pi

Streams continuously after the 1 s IMU calibration at boot.

| Message | Arguments |
|---------|-----------|
| `VEL millis v heading` | ms; float m/s; float degrees |
| `SCAN millis angle dist` | ms; robot-frame float degrees; float meters |

---

## Events — Arduino → Pi

No timestamp.

| Message | Arguments |
|---------|-----------|
| `OK CMD` | command name being acknowledged |
| `ERR CMD msg` | command name + error description |
| `DONE CMD` | `TURN` or `SCAN` — long-running command completed |
| `ESTOP reason` | `BUMPER` or `OBSTACLE` |

---

## Behavioral Rules

- `SCAN` can run simultaneously with drive commands — servo and wheels are independent.
- `ESTOP` stops motors only; an active scan continues to `DONE SCAN`.
- `SCAN_STOP` aborts an active scan and produces `ERR SCAN interrupted`.
- Single-point `SCAN angle` still sends `DONE SCAN` after its one reading.

---

## Example Exchange

```
Arduino: VEL 1001 0.000 0.0         ← streaming begins after calibration
Pi:      SET_VEL 0.2 0.0
Arduino: VEL 1523 0.200 0.1
Pi:      SCAN -21 39
Arduino: OK SCAN
Arduino: SCAN 1601 -21.0 1.24
Arduino: VEL 1610 0.200 0.2
Arduino: SCAN 1635 9.0 0.88
Arduino: DONE SCAN
Pi:      SET_VEL 0.0 30.0           ← turn at 30 deg/s; Pi watches heading and stops when done
Arduino: VEL 2103 0.000 45.2
Arduino: VEL 2154 0.000 89.8
Pi:      STOP
Arduino: OK STOP
```
