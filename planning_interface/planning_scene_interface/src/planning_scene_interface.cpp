/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2012, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan */

#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit/move_group/capability_names.h>
#include <moveit_msgs/GetPlanningScene.h>
#include <ros/ros.h>

namespace moveit
{
namespace planning_interface
{

class PlanningSceneInterface::PlanningSceneInterfaceImpl
{
public:
  
  PlanningSceneInterfaceImpl()
  {
    planning_scene_service_ = node_handle_.serviceClient<moveit_msgs::GetPlanningScene>(move_group::GET_PLANNING_SCENE_SERVICE_NAME);
  }
  
  std::vector<std::string> getKnownObjectNames(bool with_type)
  {
    moveit_msgs::GetPlanningScene::Request request;
    moveit_msgs::GetPlanningScene::Response response;
    std::vector<std::string> result;
    request.components.components = request.components.WORLD_OBJECT_NAMES;
    if (!planning_scene_service_.call(request, response))
      return result;
    if (with_type)
    {
      for (std::size_t i = 0 ; i < response.scene.world.collision_objects.size() ; ++i)
        if (!response.scene.world.collision_objects[i].type.key.empty())
          result.push_back(response.scene.world.collision_objects[i].id);
    }
    else
    {
      for (std::size_t i = 0 ; i < response.scene.world.collision_objects.size() ; ++i)
        result.push_back(response.scene.world.collision_objects[i].id);
    }
    return result;
  }

  std::vector<std::string> getKnownObjectNamesInROI(double minx, double miny, double minz, double maxx, double maxy, double maxz, bool with_type)
  {
    moveit_msgs::GetPlanningScene::Request request;
    moveit_msgs::GetPlanningScene::Response response;
    std::vector<std::string> result;
    request.components.components = request.components.WORLD_OBJECT_GEOMETRY;
    if (!planning_scene_service_.call(request, response))
      return result;
    for (std::size_t i = 0; i < response.scene.world.collision_objects.size() ; ++i)
    {
      if (with_type && !response.scene.world.collision_objects[i].type.key.empty())
        continue;   
      if (response.scene.world.collision_objects[i].mesh_poses.empty() && 
          response.scene.world.collision_objects[i].primitive_poses.empty())
        continue;
      bool good = true;
      for (std::size_t j = 0 ; j < response.scene.world.collision_objects[i].mesh_poses.size() ; ++j)
        if (!(response.scene.world.collision_objects[i].mesh_poses[j].position.x >= minx &&
              response.scene.world.collision_objects[i].mesh_poses[j].position.x <= maxx &&
              response.scene.world.collision_objects[i].mesh_poses[j].position.y >= miny &&
              response.scene.world.collision_objects[i].mesh_poses[j].position.y <= maxy &&
              response.scene.world.collision_objects[i].mesh_poses[j].position.z >= minz &&
              response.scene.world.collision_objects[i].mesh_poses[j].position.z <= maxz))
        {
          good = false;
          break;
        }
      for (std::size_t j = 0 ; j < response.scene.world.collision_objects[i].primitive_poses.size() ; ++j)
        if (!(response.scene.world.collision_objects[i].primitive_poses[j].position.x >= minx &&
              response.scene.world.collision_objects[i].primitive_poses[j].position.x <= maxx &&
              response.scene.world.collision_objects[i].primitive_poses[j].position.y >= miny &&
              response.scene.world.collision_objects[i].primitive_poses[j].position.y <= maxy &&
              response.scene.world.collision_objects[i].primitive_poses[j].position.z >= minz &&
              response.scene.world.collision_objects[i].primitive_poses[j].position.z <= maxz))
        {
          good = false;
          break;
        }
      if (good)
        result.push_back(response.scene.world.collision_objects[i].id);
    }
    return result;
  }
  
private:

  ros::NodeHandle node_handle_;
  ros::ServiceClient planning_scene_service_;
  robot_model::RobotModelConstPtr robot_model_;
};

PlanningSceneInterface::PlanningSceneInterface()
{
  impl_ = new PlanningSceneInterfaceImpl();  
}

PlanningSceneInterface::~PlanningSceneInterface()
{
  delete impl_;
}

std::vector<std::string> PlanningSceneInterface::getKnownObjectNames(bool with_type)
{
  return impl_->getKnownObjectNames(with_type);
}

std::vector<std::string> PlanningSceneInterface::getKnownObjectNamesInROI(double minx, double miny, double minz, double maxx, double maxy, double maxz, bool with_type)
{
  return impl_->getKnownObjectNamesInROI(minx, miny, minz, maxx, maxy, maxz, with_type);
}


}
}

