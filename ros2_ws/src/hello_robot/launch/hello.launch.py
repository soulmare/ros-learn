from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='hello_robot',
            executable='hello_node',
        ),
        Node(
            package='hello_robot',
            executable='listener_node',
        ),
    ])
