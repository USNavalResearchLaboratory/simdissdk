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
#include "osg/Texture1D"
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/HeatMapSystem.h"

namespace simVis {

namespace {

// Centralized constexpr strings for OpenSceneGraph Uniform linkage
constexpr const char* UNIFORM_NUM_SOURCES = "svheat_NumSources";
constexpr const char* UNIFORM_POSITIONS = "svheat_Positions";
constexpr const char* UNIFORM_PARAMS = "svheat_Params";
constexpr const char* UNIFORM_GRADIENT = "svheat_Gradient";
constexpr int HEAT_LUT_TEXTURE_UNIT = 15; // High unit to avoid osgEarth conflicts

/** Fast CPU-side linear interpolation for building the texture for the gradient */
osg::Vec4 interpolateColor(const std::map<float, osg::Vec4>& grad, float val)
{
  if (grad.empty())
    return osg::Vec4(0,0,0,0);
  const auto upper = grad.lower_bound(val);
  if (upper == grad.end())
    return grad.rbegin()->second;
  if (upper == grad.begin() || upper->first == val)
    return upper->second;

  const auto lower = std::prev(upper);
  // Impossible to have a divide-by-zero with these data types
  const float t = (val - lower->first) / (upper->first - lower->first);
  return lower->second * (1.0f - t) + upper->second * t;
}

/** Retrieve a default gradient */
std::map<float, osg::Vec4> getDefaultHeatGradient()
{
  return {
    { 0.0f, osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f) }, // Yellow, transparent
    { 0.5f, osg::Vec4(1.0f, 0.5f, 0.0f, 1.0f) }, // Orange, opaque
    { 1.0f, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) } // Red, opaque
  };
}

std::map<float, osg::Vec4> getWhiteHotGradient()
{
  return {
    { 0.0f, osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f) }, // Transparent Black
    { 1.0f, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) }  // Opaque White
  };
}

std::map<float, osg::Vec4> getBlackHotGradient()
{
  return {
    { 0.0f, osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f) }, // Transparent White
    { 1.0f, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) }  // Opaque Black
  };
}

std::map<float, osg::Vec4> getIronbowGradient()
{
  return {
    { 0.00f, osg::Vec4(0.00f, 0.00f, 0.00f, 0.0f) }, // Transparent Black
    { 0.25f, osg::Vec4(0.40f, 0.00f, 0.60f, 0.5f) }, // Purple
    { 0.50f, osg::Vec4(0.90f, 0.20f, 0.10f, 0.8f) }, // Orange-Red
    { 0.75f, osg::Vec4(1.00f, 0.80f, 0.00f, 1.0f) }, // Yellow
    { 1.00f, osg::Vec4(1.00f, 1.00f, 1.00f, 1.0f) }  // White
  };
}

std::map<float, osg::Vec4> getJetGradient()
{
  return {
    // Fade the alpha in over the first 5% to prevent a harsh visual "cliff" at the radius edge
    { 0.000f, osg::Vec4(0.0f, 0.0f, 0.5f, 0.0f) }, // Dark Blue, Fully Transparent
    { 0.050f, osg::Vec4(0.0f, 0.0f, 0.7f, 1.0f) }, // Dark Blue, Fully Opaque
    { 0.125f, osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) }, // Blue
    { 0.375f, osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f) }, // Cyan
    { 0.625f, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f) }, // Yellow
    { 0.875f, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) }, // Red
    { 1.000f, osg::Vec4(0.5f, 0.0f, 0.0f, 1.0f) }  // Dark Red
  };
}

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

  // Initialize the per-node 1D Lookup Texture
  cache.lutImage = new osg::Image;
  cache.lutImage->allocateImage(256, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
  cache.lutTexture = new osg::Texture1D(cache.lutImage);
  cache.lutTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
  cache.lutTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
  cache.lutTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);

  // Initialize defaults
  cache.numSourcesUniform->set(0);
  cache.isDirty = false;

  // Attach uniforms to the scene graph
  ss->addUniform(cache.numSourcesUniform.get());
  ss->addUniform(cache.positionsUniform.get());
  ss->addUniform(cache.parametersUniform.get());

  // Attach the per-node 1D texture to the node's StateSet
  ss->setTextureAttributeAndModes(HEAT_LUT_TEXTURE_UNIT, cache.lutTexture.get(), osg::StateAttribute::ON);
  osg::ref_ptr<osg::Uniform> gradientUniform = new osg::Uniform(osg::Uniform::SAMPLER_1D, UNIFORM_GRADIENT);
  gradientUniform->set(HEAT_LUT_TEXTURE_UNIT);
  ss->addUniform(gradientUniform.get());

  uniformCache_[targetNode] = cache;

  // Begin watching this node for destruction
  targetNode->addObserver(nodeTracker_.get());

  // Bake the default colors into the image immediately
  setGradient(targetNode, HeatGradientType::Heat);
}

void HeatMapSystem::setGradient(osg::Node* targetNode, const std::map<float, osg::Vec4>& gradient)
{
  if (!targetNode)
    return;

  ensureUniformsExist_(targetNode);
  auto& cache = uniformCache_[targetNode];
  unsigned char* rowData = cache.lutImage->data();
  for (int x = 0; x < 256; ++x)
  {
    const float pct = static_cast<float>(x) / 255.0f;
    const osg::Vec4 c = interpolateColor(gradient, pct);
    rowData[x * 4 + 0] = static_cast<unsigned char>(c.r() * 255.0f);
    rowData[x * 4 + 1] = static_cast<unsigned char>(c.g() * 255.0f);
    rowData[x * 4 + 2] = static_cast<unsigned char>(c.b() * 255.0f);
    rowData[x * 4 + 3] = static_cast<unsigned char>(c.a() * 255.0f);
  }

  // Tell OpenGL the pixel buffer for this specific node changed
  cache.lutImage->dirty();
}

void HeatMapSystem::setGradient(osg::Node* targetNode, HeatGradientType gradientType)
{
  switch (gradientType)
  {
  case HeatGradientType::Heat:
    setGradient(targetNode, getDefaultHeatGradient());
    break;
  case HeatGradientType::WhiteHot:
    setGradient(targetNode, getWhiteHotGradient());
    break;
  case HeatGradientType::BlackHot:
    setGradient(targetNode, getBlackHotGradient());
    break;
  case HeatGradientType::Ironbow:
    setGradient(targetNode, getIronbowGradient());
    break;
  case HeatGradientType::Jet:
    setGradient(targetNode, getJetGradient());
    break;
  }
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
      ss->removeUniform(UNIFORM_GRADIENT);
      ss->removeTextureAttribute(HEAT_LUT_TEXTURE_UNIT, osg::StateAttribute::TEXTURE);
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
