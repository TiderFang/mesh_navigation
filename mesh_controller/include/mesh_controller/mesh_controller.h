/*
 *  Copyright 202ß, Sebastian Pütz, Sabrina Frohn
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  authors:
 *    Sabrina Frohn <sfrohn@uni-osnabrueck.de>
 *
 */

#ifndef MESH_NAVIGATION__MESH_CONTROLLER_H
#define MESH_NAVIGATION__MESH_CONTROLLER_H

#include <mbf_mesh_core/mesh_controller.h>
#include <mbf_msgs/GetPathResult.h>
#include <mesh_controller/MeshControllerConfig.h>
#include <mesh_map/mesh_map.h>
#include <nav_msgs/Path.h>
#include <visualization_msgs/MarkerArray.h>

namespace mesh_controller
{
class MeshController : public mbf_mesh_core::MeshController
{
public:
  typedef boost::shared_ptr<mesh_controller::MeshController> Ptr;

  MeshController();

  /**
   * @brief Destructor
   */
  virtual ~MeshController();

  /**
   * @brief Given the current position, orientation, and velocity of the robot,
   * compute velocity commands to send to the base.
   * @param pose The current pose of the robot.
   * @param velocity The current velocity of the robot.
   * @param cmd_vel Will be filled with the velocity command to be passed to the
   * robot base.
   * @param message Optional more detailed outcome as a string
   * @return Result code as described on ExePath action result:
   *         SUCCESS         = 0
   *         1..9 are reserved as plugin specific non-error results
   *         FAILURE         = 100   Unspecified failure, only used for old,
   * non-mfb_core based plugins CANCELED        = 101 NO_VALID_CMD    = 102
   *         PAT_EXCEEDED    = 103
   *         COLLISION       = 104
   *         OSCILLATION     = 105
   *         ROBOT_STUCK     = 106
   *         MISSED_GOAL     = 107
   *         MISSED_PATH     = 108
   *         BLOCKED_PATH    = 109
   *         INVALID_PATH    = 110
   *         TF_ERROR        = 111
   *         NOT_INITIALIZED = 112
   *         INVALID_PLUGIN  = 113
   *         INTERNAL_ERROR  = 114
   *         121..149 are reserved as plugin specific errors
   */
  virtual uint32_t computeVelocityCommands(const geometry_msgs::PoseStamped& pose,
                                           const geometry_msgs::TwistStamped& velocity,
                                           geometry_msgs::TwistStamped& cmd_vel, std::string& message);

  /**
   * @brief Check if the goal pose has been achieved by the local planner
   * @param pose The current pose of the robot.
   * @param angle_tolerance The angle tolerance in which the current pose will
   * be partly accepted as reached goal
   * @param dist_tolerance The distance tolerance in which the current pose will
   * be partly accepted as reached goal
   * @return True if achieved, false otherwise
   */
  virtual bool isGoalReached(double dist_tolerance, double angle_tolerance);

  /**
   * @brief Set the plan that the local planner is following
   * @param plan The plan to pass to the local planner
   * @return True if the plan was updated successfully, false otherwise
   */
  virtual bool setPlan(const std::vector<geometry_msgs::PoseStamped>& plan);

  /**
   * @brief Requests the planner to cancel, e.g. if it takes too much time.
   * @return True if a cancel has been successfully requested, false if not
   * implemented.
   */
  virtual bool cancel();

  /**
   * Results in a slow increase of velocity in the beginning and decrease
   * towards end. Can be modulated via fading variable.
   * @param current pose of robot (for meshGradient use)
   * @return factor by which to multiply the linear velocity for smooth start
   * and end
   */
  float fadingFactor(const geometry_msgs::PoseStamped& pose);

  /**
   * Transforms a PoseStamped into a direction mesh_map Vector
   * @param pose      any geometry_msgs PoseStamped
   * @return          direction mesh_map Vector
   */
  mesh_map::Vector poseToDirectionVector(const geometry_msgs::PoseStamped& pose);

  /**
   * Transforms a PoseStamped into a position vector
   * @param pose      any geometry_msgs PoseStamped
   * @return          position mesh_map Vector
   */
  mesh_map::Vector poseToPositionVector(const geometry_msgs::PoseStamped& pose);

  /**
   * Calculates the (smaller) angle between two vectors
   * @param pos       mesh_map Vector of the current robot heading
   * @param plan      mesh_map Vector of the supposed (planned) heading
   * @param leftRight is set to -1 if a left turn has to be made, or +1 for
   * right turn
   * @return          (smaller) angle between the vectors
   */
  float angleBetweenVectors(mesh_map::Vector pos, mesh_map::Vector plan);

  /**
   * A linear function to return a phased response value given a value
   * @param max_hight     maximum value that will be returned
   * @param x_axis        determines the intersection point with the x-axis
   * @param max_width     range that the given value can take before the maximum
   * is returned
   * @param value         value on x axis
   * @return              value on y axis that corresponds to input value
   */
  float linValue(float max_hight, float x_axis, float max_width, float value);

  /**
   * A *normal distribution / gaussian* function to return a phased response
   * value given a value
   * @param max_hight     maximum value that will be returned
   * @param max_width     range that the given value can take before the maximum
   * is returned
   * @param value         value on x axis
   * @return              value on y axis that corresponds to input value
   */
  float gaussValue(float max_hight, float max_width, float value);

  /**
   * Calculates in which direction the robot has to turn
   * @param current       current heading of the robot
   * @param supposed      supposed heading of the robot
   * @return              -1 for left turn, 1 else
   */
  float direction(mesh_map::Vector& robot_heading, mesh_map::Vector& planned_heading);

  /**
   * Checks if the robot has driven too far of the plan
   * @param robot_pose    current pose of the robot
   * @return              true, if the robot is too far off, false otherwise
   */
  bool offPlan(const geometry_msgs::PoseStamped& robot_pose);

  /**
   * returns the euclidean distance between two poses
   * @param pose              first pose
   * @param plan_position     second pose
   * @return                  distance as float
   */
  float euclideanDistance(const geometry_msgs::PoseStamped& pose);

  float euclideanDistance(lvr2::BaseVector<float>& current, lvr2::BaseVector<float>& planned);

  /**
   * Calculates the Euclidean Distance between the current robot pose and the
   * next x poses towards the goal to find the closest part of the plan.
   * @param pose      current position of the robot
   * @param velocity  current velocity of the robot
   */
  uint32_t updatePlanPos(const geometry_msgs::PoseStamped& pose, float velocity);

  /**
   * FOR PLANNED PATH NAVIGATION
   * Checks the difficulty of the next vertex / vertices (depending on velocity)
   * to calculate a factor by which to change the velocity
   * @param pose          current position of the robot
   * @param velocity      speed of the robot
   * @return              factor to increase/decrease/keep the robots {angular
   * and linear} velocity between 0 and 1 returns infinity for linear and
   * angular value for the first call as no velocity is available
   */
  std::vector<float> lookAhead(const geometry_msgs::PoseStamped& pose, float velocity);

  /**
   * Combines angle difference, look ahead etc. to determine angular and linear
   * velocities
   * @param pose          Current robot pose.
   * @param velocity      Current velocity of the robot.
   * @param plan_vec      vector of supposed position (necessary if using the
   * mesh Gradient and not the plan)
   * @return              vector with new angular velocity and new linear
   * velocity
   */
  std::vector<float> naiveControl(const geometry_msgs::PoseStamped& pose, mesh_map::Vector supposed_dir,
                                  const float& cost);

  void reconfigureCallback(mesh_controller::MeshControllerConfig& cfg, uint32_t level);

  virtual bool initialize(const std::string& plugin_name, const boost::shared_ptr<tf2_ros::Buffer>& tf_ptr,
                          const boost::shared_ptr<mesh_map::MeshMap>& mesh_map_ptr);

protected:
private:
  // TODO sort out unnecessary variables
  boost::shared_ptr<mesh_map::MeshMap> map_ptr;
  vector<geometry_msgs::PoseStamped> current_plan;

  int plan_iter;

  ros::Time last_call;
  ros::Time last_lookahead_call;

  geometry_msgs::PoseStamped goal;
  // distance between the current robot position and the goal - important when
  // using mesh gradient
  float goal_dist_mesh;
  geometry_msgs::PoseStamped plan_position;

  // for pid control
  float int_dis_error;
  float int_dir_error;
  float prev_dis_error;
  float prev_dir_error;

  bool goalSet;
  float set_linear_velocity;

  lvr2::OptionalFaceHandle current_face;
  // angle between pose vector and planned / supposed vector
  geometry_msgs::Pose current_pose;

  float goal_distance;

  float angle;
  // stores the current vector map containing vectors pointing to the source
  // (path goal)
  lvr2::DenseVertexMap<mesh_map::Vector> vector_map;
  // face handle to store the face of the position ahead
  lvr2::OptionalFaceHandle mesh_ahead_face;

  // Server for Reconfiguration
  std::string name;
  ros::NodeHandle private_nh;
  boost::shared_ptr<dynamic_reconfigure::Server<mesh_controller::MeshControllerConfig>> reconfigure_server_ptr;
  dynamic_reconfigure::Server<mesh_controller::MeshControllerConfig>::CallbackType config_callback;
  bool first_config;
  MeshControllerConfig config;

  ros::Publisher angle_pub;
  ros::Publisher ahead_angle_pub;
  ros::Publisher ahead_cost_pub;

  ros::Publisher position_pub;

  float initial_dist;
  float last_fading;

  std::atomic_bool cancel_requested;

  const float E = 2.718281;
};

} /* namespace mesh_controller */
#endif /* MESH_NAVIGATION__MESH_CONTROLLER_H */
