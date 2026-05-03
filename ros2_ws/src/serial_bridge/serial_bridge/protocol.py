"""
Pure functions for encoding commands and parsing responses.
No ROS2 imports — this module can be tested standalone.

Parse result is a tuple whose first element is the message type string:
  ('VEL',   millis: int, v: float, heading_deg: float)
  ('SCAN',  millis: int, angle_deg: float, dist_m: float)
  ('ESTOP', reason: str)
  ('OK',    cmd: str)
  ('ERR',   cmd: str, msg: str)
  ('DONE',  cmd: str)
  ('UNKNOWN', raw: str)
"""


def parse_line(line: str) -> tuple:
    """Parse one stripped serial line into a result tuple (see module docstring)."""
    parts = line.split()
    if not parts:
        return ('UNKNOWN', line)
    token = parts[0]
    try:
        if token == 'VEL' and len(parts) == 4:
            return ('VEL', int(parts[1]), float(parts[2]), float(parts[3]))
        if token == 'SCAN' and len(parts) == 4:
            return ('SCAN', int(parts[1]), float(parts[2]), float(parts[3]))
        if token == 'ESTOP' and len(parts) == 2:
            return ('ESTOP', parts[1])
        if token == 'OK' and len(parts) >= 2:
            return ('OK', ' '.join(parts[1:]))
        if token == 'ERR' and len(parts) >= 3:
            return ('ERR', parts[1], ' '.join(parts[2:]))
        if token == 'DONE' and len(parts) == 2:
            return ('DONE', parts[1])
    except (ValueError, IndexError):
        pass
    return ('UNKNOWN', line)


def encode_set_vel(v: float, omega_deg: float) -> str:
    """Return a SET_VEL command string ready to write to serial (newline included)."""
    return f'SET_VEL {v:.3f} {omega_deg:.3f}\n'


def encode_stop() -> str:
    """Return a STOP command string ready to write to serial (newline included)."""
    return 'STOP\n'


def encode_set_param(name: str, val: float) -> str:
    """Return a SET_PARAM command string ready to write to serial (newline included)."""
    return f'SET_PARAM {name} {val}\n'
