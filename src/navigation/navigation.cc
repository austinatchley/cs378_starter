//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
\file    navigation.cc
\brief   Starter code for navigation.
\author  Joydeep Biswas, (C) 2019
*/
//========================================================================

#include "navigation.h"
#include "eigen3/Eigen/Dense"
#include "eigen3/Eigen/Geometry"
#include "f1tenth_course/AckermannCurvatureDriveMsg.h"
#include "f1tenth_course/Pose2Df.h"
#include "f1tenth_course/VisualizationMsg.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "ros/ros.h"
#include "shared/math/math_util.h"
#include "shared/ros/ros_helpers.h"
#include "shared/util/timer.h"
#include "visualization/visualization.h"

using Eigen::Vector2f;
using f1tenth_course::AckermannCurvatureDriveMsg;
using f1tenth_course::VisualizationMsg;
using std::string;
using std::vector;

using namespace math_util;
using namespace ros_helpers;

namespace {
ros::Publisher drive_pub_;
ros::Publisher viz_pub_;
VisualizationMsg local_viz_msg_;
VisualizationMsg global_viz_msg_;
AckermannCurvatureDriveMsg drive_msg_;
// Epsilon value for handling limited numerical precision.
const float kEpsilon = 1e-5;
} // namespace

namespace navigation {

Navigation::Navigation(const string& map_file, ros::NodeHandle* n)
    : _startTime(now()), _timeOfLastNav(0.f), _navTime(0.f), _rampUpTime(0.f), _timeAtFullVel(0.f),
      robot_loc_(0, 0), robot_angle_(0), robot_vel_(0, 0), robot_omega_(0), nav_complete_(true),
      nav_goal_loc_(0, 0), nav_goal_angle_(0) {
  drive_pub_ = n->advertise<AckermannCurvatureDriveMsg>("ackermann_curvature_drive", 1);
  viz_pub_ = n->advertise<VisualizationMsg>("visualization", 1);
  local_viz_msg_ = visualization::NewVisualizationMessage("base_link", "navigation_local");
  global_viz_msg_ = visualization::NewVisualizationMessage("map", "navigation_global");
  InitRosHeader("base_link", &drive_msg_.header);
}

void Navigation::SetNavGoal(const Vector2f& loc, float angle) {
  std::cout << "set nav goal" << std::endl;
  _timeOfLastNav = now();

  _rampUpTime = (MAX_VEL - robot_vel_[0]) / MAX_ACCEL;
  _navTime = 10.f; // TODO: find total nav time
  _timeAtFullVel = _navTime - 2.f * _rampUpTime;

  nav_complete_ = false;

  // nav_goal_loc_ = loc;
  // nav_goal_angle_ = angle;
}

void Navigation::UpdateLocation(const Eigen::Vector2f& loc, float angle) {
  // robot_loc_ = loc;
  // robot_angle_ = angle;
}

void Navigation::UpdateOdometry(const Vector2f& loc, float angle, const Vector2f& vel,
                                float ang_vel) {}

void Navigation::ObservePointCloud(const vector<Vector2f>& cloud, double time) {}

void Navigation::Run() {
  const float timeSinceLastNav = (now() - _timeOfLastNav) / 1000.f;

  AckermannCurvatureDriveMsg msg;
  if (!nav_complete_) {
      if (timeSinceLastNav < _rampUpTime) {
          msg.velocity = lerp(0, MAX_VEL, timeSinceLastNav / _rampUpTime); // accelerate
      } else if (timeSinceLastNav > _navTime - _rampUpTime) {
          msg.velocity = lerp(MAX_VEL, 0, timeSinceLastNav / _rampUpTime); // decelerate
      }
      else {
          msg.velocity = MAX_VEL;
      }
  } else {
    msg.velocity = 0;
  }

  // msg.curvature = 1.f; // 1m radius of turning

  std::cout << "Time elapsed since last nav command: " << timeSinceLastNav << std::endl;
  std::cout << "Sending velocity: " << msg.velocity << std::endl;

  drive_pub_.publish(msg);

  // Milestone 3 will complete the rest of navigation.
}

float Navigation::lerp(float a, float b, float t) {
  if (t > 1.f) {
    t = 1.f;
  }
  return a + t * (b - a);
}

float Navigation::now() {
  return static_cast<float>(std::clock());
}

} // namespace navigation
