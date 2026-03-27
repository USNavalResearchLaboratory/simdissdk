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
#pragma once

#include <memory>
#include <vector>
#include "osg/ref_ptr"
#include "osg/Node"
#include "osg/Observer"
#include "osg/Uniform"
#include "osg/Vec3f"
#include "entt/container/dense_map.hpp"

namespace simVis {

/** Represents a single localized heat source on a 3D model. */
struct HeatMapPoint
{
  osg::Vec3f position;  // Model-space coordinates
  float radius = 1.f;     // Max distance of effect
  float intensity = 1.f;    // Heat value at center (0.0 to 1.0)
  float falloff = 2.f;    // Curve of the fade (1.0 = linear, 2.0 = quadratic)
};

/**
 * Manages point-based heat map gradients for 3D entities.
 * Decouples the simulation logic from the OpenSceneGraph uniform packing.
 */
class HeatMapSystem
{
public:
  /// The maximum number of heat points allowed per model.
  static constexpr int MAX_HEAT_POINTS = 4;

  HeatMapSystem();
  virtual ~HeatMapSystem();

  /**
   * Injects the necessary osgEarth VirtualProgram shaders into the target node.
   * Call this ONCE when the aircraft/model is created.
   */
  void installShaders(osg::Node* targetNode);

  /** Replaces all heat points for a specific target. */
  void setPoints(osg::Node* targetNode, const std::vector<HeatMapPoint>& points);

  /**
   * Fast-path update: Modifies ONLY the intensity of an existing point.
   * Use this in the main simulation loop for high-frequency updates.
   */
  void setPointIntensity(osg::Node* targetNode, size_t pointIndex, float intensity);

  /** Clears all heat points from a target, returning it to normal colors. */
  void clearPoints(osg::Node* targetNode);

  /**
   * Updates the OpenSceneGraph Uniforms for all tracked nodes. Should be called
   * once per frame during the rendering loop.
   */
  void update();

private:
  /** Internal struct to hold OSG Uniform pointers so we don't look them up every frame */
  struct TargetUniforms
  {
    /** Number of configured sources */
    osg::ref_ptr<osg::Uniform> numSourcesUniform;
    /** Positions, vec3 array */
    osg::ref_ptr<osg::Uniform> positionsUniform;
    /** Radius, intensity, and falloff, in vec3 format */
    osg::ref_ptr<osg::Uniform> parametersUniform;

    /** Dirty flag so we only upload uniforms that actually changed */
    bool isDirty = false;
  };

  /** Nested Observer class to hook into OSG's native memory management. */
  class NodeTracker : public osg::Observer
  {
  public:
    explicit NodeTracker(HeatMapSystem* sys);

    /** Called in ~HeatMapSystem() to completely ensure no dangling pointers left */
    void clearSystem();
    virtual void objectDeleted(void* ptr) override;

  private:
    HeatMapSystem* system_ = nullptr;
  };

  /** Helper to initialize uniforms if they don't exist yet */
  void ensureUniformsExist_(osg::Node* targetNode);
  /** Internal callback invoked by OSG when a tracked node is being deleted */
  void handleNodeDeletion_(osg::Node* targetNode);

  /** The master data store. */
  entt::dense_map<osg::Node*, std::vector<HeatMapPoint>> heatData_;
  /** Cache of the OSG uniforms tied to each node's StateSet */
  entt::dense_map<osg::Node*, TargetUniforms> uniformCache_;
  /** Responsible for being notified of node deletion */
  std::unique_ptr<NodeTracker> nodeTracker_;
};

}
