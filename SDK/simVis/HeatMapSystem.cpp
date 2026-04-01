/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <algorithm>
#include <cassert>
#include <osgEarth/VirtualProgram>
#include "simVis/Shaders.h"
#include "simVis/HeatMapSystem.h"

namespace simVis {

namespace {

// Centralized constexpr strings for OpenSceneGraph Uniform linkage
constexpr const char* UNIFORM_NUM_SOURCES = "svheat_NumSources";
constexpr const char* UNIFORM_POSITIONS = "svheat_Positions";
constexpr const char* UNIFORM_PARAMS = "svheat_Params";

}

HeatMapSystem::NodeTracker::NodeTracker(HeatMapSystem* sys)
 : system_(sys)
{
}

void HeatMapSystem::NodeTracker::clearSystem()
{
  system_ = nullptr;
}

void HeatMapSystem::NodeTracker::objectDeleted(void* ptr)
{
  // Safe to cast purely for pointer-address matching in the map
  if (system_)
    system_->handleNodeDeletion_(static_cast<osg::Node*>(ptr));
}

/////////////////////////////////////////////////////////////

HeatMapSystem::HeatMapSystem()
{
  nodeTracker_ = std::make_unique<NodeTracker>(this);
}

HeatMapSystem::~HeatMapSystem()
{
  // Node Tracker is allocated in constructor and owned by us, no way it is invalid
  assert(nodeTracker_);

  // Disconnect the callback immediately
  nodeTracker_->clearSystem();

  // Cleanly detach the observer from all tracked nodes
  for (const auto& [node, data] : heatData_)
  {
    if (node)
      node->removeObserver(nodeTracker_.get());
  }
}

void HeatMapSystem::installShaders(osg::Node* targetNode)
{
  if (!targetNode)
    return;

  osg::StateSet* ss = targetNode->getOrCreateStateSet();
  osg::ref_ptr<osgEarth::VirtualProgram> vp = osgEarth::VirtualProgram::getOrCreate(ss);
  simVis::Shaders shaders;
  shaders.load(vp, shaders.heatMapSystem());

  osg::ref_ptr<osg::Uniform> defaultNumSources = new osg::Uniform(osg::Uniform::INT, UNIFORM_NUM_SOURCES);
  defaultNumSources->set(0);
  ss->addUniform(defaultNumSources.get());

  ss->addUniform(new osg::Uniform(osg::Uniform::FLOAT_VEC3, UNIFORM_POSITIONS, MAX_HEAT_POINTS));
  ss->addUniform(new osg::Uniform(osg::Uniform::FLOAT_VEC3, UNIFORM_PARAMS, MAX_HEAT_POINTS));
}

void HeatMapSystem::ensureUniformsExist_(osg::Node* targetNode)
{
  // If it's already cached, we're done
  if (uniformCache_.contains(targetNode))
    return;

  osg::StateSet* ss = targetNode->getOrCreateStateSet();

  TargetUniforms cache;
  cache.numSourcesUniform = new osg::Uniform(osg::Uniform::INT, UNIFORM_NUM_SOURCES);
  cache.positionsUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, UNIFORM_POSITIONS, MAX_HEAT_POINTS);
  cache.parametersUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, UNIFORM_PARAMS, MAX_HEAT_POINTS);

  // Initialize defaults
  cache.numSourcesUniform->set(0);
  cache.isDirty = false;

  // Attach uniforms to the scene graph
  ss->addUniform(cache.numSourcesUniform.get());
  ss->addUniform(cache.positionsUniform.get());
  ss->addUniform(cache.parametersUniform.get());

  uniformCache_[targetNode] = cache;

  // Begin watching this node for destruction
  targetNode->addObserver(nodeTracker_.get());
}

void HeatMapSystem::handleNodeDeletion_(osg::Node* targetNode)
{
  heatData_.erase(targetNode);
  uniformCache_.erase(targetNode);
}

void HeatMapSystem::setPoints(osg::Node* targetNode, const std::vector<HeatMapPoint>& points)
{
  if (!targetNode)
    return;

  ensureUniformsExist_(targetNode);

  heatData_[targetNode] = points;
  uniformCache_[targetNode].isDirty = true;
}

void HeatMapSystem::setPointIntensity(osg::Node* targetNode, size_t pointIndex, float intensity)
{
  if (!targetNode || !heatData_.contains(targetNode))
    return;

  auto& points = heatData_[targetNode];
  if (pointIndex < points.size())
  {
    // Fast-path update
    points[pointIndex].intensity = intensity;
    uniformCache_[targetNode].isDirty = true;
  }
}

void HeatMapSystem::clearPoints(osg::Node* targetNode)
{
  if (!targetNode || !heatData_.contains(targetNode))
    return;

  heatData_.erase(targetNode);
  if (auto iter = uniformCache_.find(targetNode); iter != uniformCache_.end())
  {
    // Remove the overriding uniforms from the child's StateSet
    osg::StateSet* ss = targetNode->getStateSet();
    if (ss)
    {
      ss->removeUniform(UNIFORM_NUM_SOURCES);
      ss->removeUniform(UNIFORM_POSITIONS);
      ss->removeUniform(UNIFORM_PARAMS);
    }
    uniformCache_.erase(iter);
  }
  // No longer care about the observer
  targetNode->removeObserver(nodeTracker_.get());
}

void HeatMapSystem::update()
{
  for (auto [node, cache] : uniformCache_)
  {
    if (!cache.isDirty)
      continue;

    const auto& points = heatData_[node];

    // Clamp the number of sources we pass to the shader
    const int activeSources = std::min(static_cast<int>(points.size()), MAX_HEAT_POINTS);
    cache.numSourcesUniform->set(activeSources);

    for (int i = 0; i < activeSources; ++i)
    {
      const auto& pt = points[i];

      // Upload the position directly
      cache.positionsUniform->setElement(i, pt.position);

      // Pack radius, intensity, and falloff into a single vec3
      cache.parametersUniform->setElement(i, osg::Vec3f(pt.radius, pt.intensity, pt.falloff));
    }

    // Reset the flag so we don't spam OpenGL with updates next frame
    cache.isDirty = false;
  }
}

void HeatMapSystem::setPoint(osg::Node* targetNode, size_t pointIndex, const HeatMapPoint& point)
{
  if (!targetNode || pointIndex >= MAX_HEAT_POINTS)
    return;

  ensureUniformsExist_(targetNode);

  auto& points = heatData_[targetNode];
  if (pointIndex >= points.size())
  {
    const size_t oldSize = points.size();
    points.resize(pointIndex + 1);

    // Zero out any skipped elements so they don't accidentally render default heat spheres
    for (size_t i = oldSize; i < pointIndex; ++i)
    {
      points[i].intensity = 0.f;
      points[i].radius = 0.f;
    }
  }

  points[pointIndex] = point;
  uniformCache_[targetNode].isDirty = true;
}

}
