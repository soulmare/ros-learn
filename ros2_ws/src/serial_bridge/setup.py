from setuptools import find_packages, setup

package_name = 'serial_bridge'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch', ['launch/robot.launch.py']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='alex',
    maintainer_email='o.bolokhovetskyi@mayabproject.com',
    description='ROS2 node that owns the serial connection to the rover firmware',
    license='TODO',
    extras_require={'test': ['pytest']},
    entry_points={
        'console_scripts': [
            'bridge_node = serial_bridge.bridge_node:main',
        ],
    },
)
