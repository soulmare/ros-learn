# Arduino ↔ ROS2 Serial Protocol

Text-based, one message per line (`\n` terminated), 115200 baud.

---

## Commands — Pi → Arduino

| Message | Arguments | Response |
|---------|-----------|----------|
| `SET_VEL v [ω]` | floats, m/s and °/s; ω defaults to 0 if omitted | none |
| `STOP` | — | `OK STOP`; additionally `ERR TURN interrupted` if turn was in progress |
| `SET_PARAM NAME VAL` | string, float | `OK SET_PARAM` or `ERR SET_PARAM msg` |
| `TURN degrees` | float | `OK TURN` → heading updates via `VEL` → `DONE TURN` or `ERR TURN msg` |
| `SCAN` | — | full sweep, 5°–165° |
| `SCAN from to` | float°, float° | partial sweep between two angles |
| `SCAN angle` | float° | single point reading |
| `SCAN_STOP` | — | aborts active scan → `ERR SCAN interrupted` |

All `SCAN` variants: `OK SCAN` on start, `SCAN` telemetry lines during sweep, `DONE SCAN` on completion.

---

## Telemetry — Arduino → Pi

Streams continuously after the 1 s IMU calibration at boot.

| Message | Arguments |
|---------|-----------|
| `VEL millis v heading` | ms; float m/s or `-`; float degrees |
| `SCAN millis angle dist` | ms; float degrees; float meters |

`v` is `-` during a `TURN` — Pi should use heading only for odometry updates in that case.

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

- `TURN` and `SCAN` can run simultaneously — servo and wheels are independent.
- `ESTOP` stops motors only; an active scan continues to `DONE SCAN`.
- `STOP` sent during `TURN` produces `OK STOP` and `ERR TURN interrupted`.
- `SCAN_STOP` aborts an active scan and produces `ERR SCAN interrupted`.
- Single-point `SCAN angle` still sends `DONE SCAN` after its one reading.

---

## Example Exchange

```
Arduino: VEL 1001 - 0.0             ← streaming begins after calibration
Pi:      SET_VEL 0.2 0.0
Arduino: VEL 1523 0.200 0.1
Pi:      SCAN 60 120
Arduino: OK SCAN
Arduino: SCAN 1601 60.0 1.24
Arduino: VEL 1610 0.200 0.2
Arduino: SCAN 1635 90.0 0.88
Arduino: DONE SCAN
Pi:      TURN 90
Arduino: OK TURN
Arduino: VEL 2103 - 45.2
Arduino: VEL 2154 - 89.8
Arduino: DONE TURN
Pi:      STOP
Arduino: OK STOP
```
