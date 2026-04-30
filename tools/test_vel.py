#!/usr/bin/env python3
"""Send SET_VEL to the Arduino for a fixed duration, then STOP.

Usage:
    python3 test_vel.py <velocity> <duration> [--omega DEG_S] [--port PORT]
    python3 test_vel.py 0.3 5
    python3 test_vel.py 0.0 5 --omega 30
    python3 test_vel.py 0.3 5 --port /dev/ttyUSB0
"""

import argparse
import sys
import time
import serial


DEFAULT_PORT = "/dev/ttyUSB0"
BAUD_RATE    = 115200


def main():
    parser = argparse.ArgumentParser(description="Drive the robot at a set velocity for N seconds.")
    parser.add_argument("velocity", type=float, help="Forward speed in m/s")
    parser.add_argument("duration", type=float, help="How long to drive in seconds")
    parser.add_argument("--omega",  type=float, default=0.0, help="Angular speed in deg/s (default: 0)")
    parser.add_argument("--port",   default=DEFAULT_PORT, help=f"Serial port (default: {DEFAULT_PORT})")
    args = parser.parse_args()

    print(f"Connecting to {args.port} at {BAUD_RATE} baud...")
    try:
        # serial.Serial opens and configures the port.
        # timeout=1 means reads will give up after 1 second if no data arrives.
        ser = serial.Serial(args.port, BAUD_RATE, timeout=1)
    except serial.SerialException as e:
        print(f"Error: {e}")
        sys.exit(1)

    # Arduino resets when the serial port is opened.
    # Wait for "OK INIT HEADING" before sending commands — ensures IMU calibration is done.
    print("Waiting for Arduino init...")
    INIT_TIMEOUT = 10.0
    deadline = time.time() + INIT_TIMEOUT
    ready = False
    while time.time() < deadline:
        line = ser.readline()
        if line:
            decoded = line.decode(errors='replace').rstrip()
            print(f"  < {decoded}")
            if decoded.startswith("OK INIT HEADING"):
                ready = True
                break
    if not ready:
        print("Error: timed out waiting for Arduino init")
        ser.close()
        sys.exit(1)

    cmd = f"SET_VEL {args.velocity:.3f} {args.omega:.3f}\n"
    print(f"Sending: {cmd.strip()}")
    # str.encode() converts the Python string to bytes, which is what serial.write() requires.
    ser.write(cmd.encode())

    print(f"Running for {args.duration} s — press Ctrl+C to stop early")
    deadline = time.time() + args.duration
    try:
        while time.time() < deadline:
            # Print any debug lines the Arduino sends back
            line = ser.readline()
            if line:
                # bytes.decode() converts the received bytes back to a Python string
                print(f"  < {line.decode(errors='replace').rstrip()}")
    except KeyboardInterrupt:
        pass

    print("Sending: STOP")
    ser.write(b"STOP\n")

    # Drain any remaining output for 0.5 s
    time.sleep(0.5)
    while ser.in_waiting:
        line = ser.readline()
        if line:
            print(f"  < {line.decode(errors='replace').rstrip()}")

    ser.close()
    print("Done.")


if __name__ == "__main__":
    main()
