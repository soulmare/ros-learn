from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='serial_bridge',
            executable='bridge_node',
            parameters=[{
                'serial_port': '/dev/ttyUSB0',
                'serial_baud': 115200,
            }],
        ),
    ])
