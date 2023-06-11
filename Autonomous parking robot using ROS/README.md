# Autonomous parking robot using ROS

- Task: To park the robot autonomously on provided spot (reflective tape) in an unknown environment without using any pre-built SLAM tools.
![image](https://github.com/kirtansoni1/Project_Portfolio/assets/73475665/813fd09d-cc9d-4015-9566-d61ce2f0acea)

- At the beginning the robot will start to follow the wall which is a way to do the scanning/mapping, in parallel the LiDAR values are read and saved to create the map, each point in the map has 3 attributes X,Y coordinates and intensity. Based on the intensity values the goals/parking spot will be defined.
- After the map generation, the map points and goal coordinates are sent to the path planning module. The path planning uses Informed RRT* algorithm which returns set of way points and the turtlebot follows the path using PID controller to the goal/parking spot.

![Alt text](https://github.com/kirtansoni1/Project_Portfolio/blob/main/Autonomous%20parking%20robot%20using%20ROS/Project%20screenshots/1%20-%20Wall%20Follower%2C%20Mapping.png)
