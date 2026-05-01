import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class HelloNode(Node):
    def __init__(self):
        super().__init__('hello_node')
        self.pub = self.create_publisher(String, 'hello', 10)
        self.create_timer(1.0, self.tick)

    def tick(self):
        msg = String()
        msg.data = 'Hello from ROS2!'
        self.pub.publish(msg)
        self.get_logger().info(msg.data)

def main():
    rclpy.init()
    node = HelloNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()