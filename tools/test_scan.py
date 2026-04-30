#!/usr/bin/env python3
"""Send a SCAN command to the Arduino and print the distance readings.

Usage:
    python3 test_scan.py                        # full sweep 5°–165°
    python3 test_scan.py --from 60 --to 120     # partial sweep
    python3 test_scan.py --angle 81             # single point (forward)
    python3 test_scan.py --angle 81 --continuous  # poll forward distance until Ctrl+C
    python3 test_scan.py --port /dev/ttyUSB0
"""

import argparse
import sys
import time
import serial


DEFAULT_PORT = "/dev/ttyUSB0"
BAUD_RATE    = 115200


def build_scan_command(args) -> str:
    if args.angle is not None:
        return f"SCAN {args.angle:.1f}"
    if args.from_deg is not None and args.to_deg is not None:
        return f"SCAN {args.from_deg:.1f} {args.to_deg:.1f}"
    return "SCAN"


def main():
    parser = argparse.ArgumentParser(description="Run a distance scan and print results.")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--angle",   type=float, metavar="DEG",
                       help="Single-point reading at this angle")
    group.add_argument("--from",    type=float, dest="from_deg", metavar="DEG",
                       help="Sweep start angle (requires --to)")
    parser.add_argument("--to",     type=float, dest="to_deg",   metavar="DEG",
                       help="Sweep end angle (requires --from)")
    parser.add_argument("--continuous", action="store_true",
                       help="Re-send SCAN after each reading until Ctrl+C (requires --angle)")
    parser.add_argument("--port",   default=DEFAULT_PORT,
                       help=f"Serial port (default: {DEFAULT_PORT})")
    args = parser.parse_args()

    if (args.from_deg is None) != (args.to_deg is None):
        parser.error("--from and --to must be used together")
    if args.continuous and args.angle is None:
        parser.error("--continuous requires --angle")

    print(f"Connecting to {args.port} at {BAUD_RATE} baud...")
    try:
        ser = serial.Serial(args.port, BAUD_RATE, timeout=1)
    except serial.SerialException as e:
        print(f"Error: {e}")
        sys.exit(1)

    # Arduino resets when the serial port is opened.
    # Wait for "OK INIT SAFETY" (last init message) before sending commands.
    print("Waiting for Arduino init...")
    INIT_TIMEOUT = 10.0
    deadline = time.time() + INIT_TIMEOUT
    ready = False
    while time.time() < deadline:
        line = ser.readline()
        if line:
            decoded = line.decode(errors='replace').rstrip()
            print(f"  < {decoded}")
            if decoded.startswith("OK INIT SAFETY"):
                ready = True
                break
    if not ready:
        print("Error: timed out waiting for Arduino init")
        ser.close()
        sys.exit(1)

    cmd = build_scan_command(args)

    def send_scan():
        ser.write(f"{cmd}\n".encode())

    send_scan()
    if args.continuous:
        print(f"Polling {args.angle:.1f}° continuously — press Ctrl+C to stop")
    else:
        print(f"Sending: {cmd}")
        print("Waiting for scan to complete — press Ctrl+C to abort...")

    readings = []  # list of (angle, dist_m) tuples collected during the sweep

    SCAN_TIMEOUT = 60.0  # full sweep at 5° steps × 100 ms = ~3.2 s; budget is generous
    deadline = time.time() + SCAN_TIMEOUT
    try:
        while time.time() < deadline:
            line = ser.readline()
            if not line:
                continue
            decoded = line.decode(errors='replace').rstrip()

            if decoded.startswith("SCAN "):
                # Format: SCAN millis angle dist
                # str.split() splits on any whitespace and returns a list of tokens.
                parts = decoded.split()
                if len(parts) == 4:
                    try:
                        angle  = float(parts[2])
                        dist_m = float(parts[3])
                        label  = f"{dist_m:.2f} m" if dist_m >= 0 else "no echo"
                        if args.continuous:
                            # \r rewrites the current line in the terminal so readings
                            # update in place rather than scrolling.
                            print(f"  {angle:6.1f}°  {label}    ", end="\r", flush=True)
                        else:
                            print(f"  {angle:6.1f}°  {label}")
                        readings.append((angle, dist_m))
                    except ValueError:
                        print(f"  < {decoded}")
                else:
                    print(f"  < {decoded}")

            elif decoded == "DONE SCAN":
                if args.continuous:
                    deadline = time.time() + SCAN_TIMEOUT  # reset watchdog
                    send_scan()
                else:
                    print("Scan complete.")
                    break

            elif decoded.startswith("ERR SCAN"):
                print(f"  < {decoded}")
                break

            elif decoded.startswith("ESTOP") and not decoded.startswith("ESTOP OBSTACLE"):
                print(f"\n  [!] {decoded}")

            else:
                # VEL telemetry or other messages — suppress to keep output clean
                pass

    except KeyboardInterrupt:
        if args.continuous:
            print()  # newline after the \r line
        else:
            print("\nInterrupting scan...")
            ser.write(b"SCAN_STOP\n")
            # Drain the interrupted-scan acknowledgement
            time.sleep(0.5)
            while ser.in_waiting:
                line = ser.readline()
                if line:
                    print(f"  < {line.decode(errors='replace').rstrip()}")

    if readings:
        valid = [(a, d) for a, d in readings if d >= 0]
        if not args.continuous:
            print(f"\n{len(readings)} readings, {len(valid)} valid.")
        if valid:
            closest = min(valid, key=lambda x: x[1])
            print(f"Closest: {closest[1]:.2f} m at {closest[0]:.1f}°")

    ser.close()
    print("Done.")


if __name__ == "__main__":
    main()
