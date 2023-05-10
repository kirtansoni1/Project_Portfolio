# Autonomous parking robot using ROS

- The objective of this project is to implement the automatic parking for turtlebot3 using self-written python functions.
- At the beginning the robot will start to follow the wall which is a way to do the scanning/mapping, in parallel the LiDAR values are read and saved to create the map, each point in the map has 3 attributes X,Y coordinates and intensity. Based on the intensity values the goals/parking spot will be defined.
- After the map generation, the map points and goal coordinates are sent to the path planning module. The path planning uses Informed RRT* algorithm which returns set of way points and the turtlebot follows the path using PID controller to the goal/parking spot.

- RaspberryPi 4, Python, ROS, LiDAR, TurtleBot3 Burger
