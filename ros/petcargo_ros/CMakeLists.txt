cmake_minimum_required(VERSION 3.0.2)
project(petcargo_ros)

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  nav_msgs
  rospy
  std_msgs
)

catkin_python_setup()

catkin_package(CATKIN_DEPENDS geometry_msgs nav_msgs rospy std_msgs)

catkin_install_python(PROGRAMS
  scripts/serial_bridge.py
  scripts/motion_executor.py
  scripts/safety_gateway.py
  scripts/dashboard_server.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY launch config udev
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

