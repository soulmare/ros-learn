import math
import queue
import sys
import threading

import serial
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TransformStamped, Twist
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Range
from std_msgs.msg import Bool
from tf2_ros import TransformBroadcaster

from . import protocol


class BridgeNode(Node):
    def __init__(self):
        super().__init__('serial_bridge')

        self.declare_parameter('serial_port', '/dev/ttyUSB0')
        self.declare_parameter('serial_baud', 115200)

        self._odom_pub   = self.create_publisher(Odometry, '/odom', 10)
        self._range_pub  = self.create_publisher(Range,    '/scan_range', 10)
        self._bumper_pub = self.create_publisher(Bool,     '/bumper', 10)
        self._tf         = TransformBroadcaster(self)

        self.create_subscription(Twist, '/cmd_vel', self._on_cmd_vel, 10)

        port = self.get_parameter('serial_port').get_parameter_value().string_value
        baud = self.get_parameter('serial_baud').get_parameter_value().integer_value
        try:
            self._serial = serial.Serial(port, baud, timeout=1.0)
        except serial.SerialException as e:
            self.get_logger().fatal(f'Cannot open serial port {port}: {e}')
            sys.exit(1)
        self.get_logger().info(f'Opened serial port {port} at {baud} baud')

        # Odometry state — updated only inside _handle_vel
        self._x           = 0.0
        self._y           = 0.0
        self._heading_rad = 0.0
        self._last_millis: int | None = None

        self._queue   = queue.Queue()
        self._running = True
        self._thread  = threading.Thread(target=self._serial_thread_fn, daemon=True)
        self._thread.start()

        self.create_timer(0.01, self._process_queue)  # drain at 100 Hz

    # ------------------------------------------------------------------
    # Background serial thread
    # ------------------------------------------------------------------

    def _serial_thread_fn(self):
        # errors='ignore': corrupt bytes on a noisy serial line must not crash the thread
        while self._running:
            line = self._serial.readline().decode('utf-8', errors='ignore').strip()
            if line:
                self._queue.put(line)

    # ------------------------------------------------------------------
    # Queue drain (runs in the ROS2 executor — safe to publish here)
    # ------------------------------------------------------------------

    def _process_queue(self):
        # Runs in the ROS2 executor — publishing from the serial thread directly is not thread-safe
        while True:
            try:
                line = self._queue.get_nowait()
            except queue.Empty:
                break
            result = protocol.parse_line(line)
            kind = result[0]
            if kind == 'VEL':
                self._handle_vel(*result[1:])
            elif kind == 'SCAN':
                self._handle_scan(*result[1:])
            elif kind == 'ESTOP':
                self._handle_estop(*result[1:])
            elif kind == 'ERR':
                self.get_logger().warning(f'Firmware error: {result[1]} {result[2]}')

    # ------------------------------------------------------------------
    # Telemetry handlers
    # ------------------------------------------------------------------

    def _handle_vel(self, millis: int, v: float, heading_deg: float):
        # heading comes from the IMU (absolute), so we use it directly instead of integrating angular velocity
        heading_rad = math.radians(heading_deg)
        if self._last_millis is not None:
            dt = (millis - self._last_millis) / 1000.0
            self._x += v * math.cos(heading_rad) * dt
            self._y += v * math.sin(heading_rad) * dt
        self._heading_rad = heading_rad
        self._last_millis = millis
        self._publish_odom(millis)

    def _handle_scan(self, millis: int, angle_deg: float, dist_m: float):
        # angle_deg unused here — Range has no direction field; LaserScan assembly comes in Phase 9
        msg = Range()
        msg.header.stamp    = self.get_clock().now().to_msg()
        msg.header.frame_id = 'base_scan'
        msg.radiation_type  = Range.ULTRASOUND
        msg.field_of_view   = 0.26
        msg.min_range       = 0.05
        msg.max_range       = 4.0
        msg.range           = dist_m
        self._range_pub.publish(msg)

    def _handle_estop(self, reason: str):
        self.get_logger().warning(f'ESTOP received: {reason}')
        self._bumper_pub.publish(Bool(data=True))

    # ------------------------------------------------------------------
    # Command publisher
    # ------------------------------------------------------------------

    def _on_cmd_vel(self, msg: Twist):
        # ROS2 convention is rad/s; firmware protocol expects deg/s
        omega_deg = math.degrees(msg.angular.z)
        self._serial.write(protocol.encode_set_vel(msg.linear.x, omega_deg).encode())

    # ------------------------------------------------------------------
    # Odometry + TF publishing (call from _handle_vel after state update)
    # ------------------------------------------------------------------

    def _publish_odom(self, millis: int):
        now = self.get_clock().now().to_msg()

        tf_msg = TransformStamped()
        tf_msg.header.stamp    = now
        tf_msg.header.frame_id = 'odom'
        tf_msg.child_frame_id  = 'base_link'
        tf_msg.transform.translation.x = self._x
        tf_msg.transform.translation.y = self._y
        tf_msg.transform.rotation.z = math.sin(self._heading_rad / 2.0)
        tf_msg.transform.rotation.w = math.cos(self._heading_rad / 2.0)
        self._tf.sendTransform(tf_msg)

        odom = Odometry()
        odom.header.stamp    = now
        odom.header.frame_id = 'odom'
        odom.child_frame_id  = 'base_link'
        odom.pose.pose.position.x = self._x
        odom.pose.pose.position.y = self._y
        odom.pose.pose.orientation.z = math.sin(self._heading_rad / 2.0)
        odom.pose.pose.orientation.w = math.cos(self._heading_rad / 2.0)
        odom.twist.twist.linear.x  = 0.0  # instantaneous — filled by caller if needed
        odom.twist.twist.angular.z = 0.0
        self._odom_pub.publish(odom)

    # ------------------------------------------------------------------

    def destroy_node(self):
        self._running = False
        self._serial.close()
        super().destroy_node()


def main():
    rclpy.init()
    node = BridgeNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
