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
#ifndef SIMVIS_PLATFORM_AZ_EL_VIEW_TOOL_H
#define SIMVIS_PLATFORM_AZ_EL_VIEW_TOOL_H

#include "osg/observer_ptr"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simVis/EntityFamily.h"
#include "simVis/TargetDelegation.h"
#include "simVis/Tool.h"
#include "simVis/Types.h"

namespace simVis
{
class LocatorNode;
class ScenarioManager;
class HorizonGeoFence;
class TargetDelegation;

/**
* Tool that renders a top down view of a platform and its
* beams/gates projected onto an azimuth/elevation polar plot.
*/
class SDKVIS_EXPORT PlatformAzimElevViewTool : public ScenarioTool
{
public:
  /**
   * Constructs a new az/el view tool.
   * @param[in ] host View will center on this host.
   */
  explicit PlatformAzimElevViewTool(EntityNode* host);

  /**
   * Maximum range of the plot.
   * @param[in ] range Range from the host in meters
   */
  void setRange(double range);
  /** Retrieves maximum range of the plot in meters */
  double getRange() const { return range_; }

  /**
   * Sets a Beam Prefs template to use for the beam display
   * when projected on to the dome.
   * @param[in ] prefs Beam prefs template
   */
  void setBeamPrefs(const simData::BeamPrefs& prefs);
  /** Retrieves a reference to the beam prefs template for beam display */
  const simData::BeamPrefs& getBeamPrefs() const { return beamPrefs_; }

  /**
   * Sets a Gate Prefs template to use for the gate display
   * when projected on to the dome.
   * @param[in ] prefs Gate prefs template
   */
  void setGatePrefs(const simData::GatePrefs& prefs);
  /** Retrieve a reference to the gate prefs template for gate display */
  const simData::GatePrefs& getGatePrefs() const { return gatePrefs_; }

  /**
   * Angle at which to draw the elevation ring labels
   * @param[in ] angle in radians
   */
  void setElevLabelAngle(float angle);
  /** Retrieves angle to draw elevation ring labels, in radians */
  float getElevLabelAngle() const { return elevLabelAngle_; }


public: // ScenarioTool

  /** @see ScenarioTool::onInstall() */
  virtual void onInstall(const ScenarioManager& scenario);

  /** @see ScenarioTool::onUninstall() */
  virtual void onUninstall(const ScenarioManager& scenario);

  /** @see ScenarioTool::onEntityAdd() */
  virtual void onEntityAdd(const ScenarioManager& scenario, EntityNode* entity);

  /** @see ScenarioTool::onEntityRemove() */
  virtual void onEntityRemove(const ScenarioManager& scenario, EntityNode* entity);

  /** @see ScenarioTool::onUpdate() */
  virtual void onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates);

public: // Tool

  // returns the node to display in the scenario graph
  osg::Node* getNode() const;

public: // internal

  /// Updates the geometries when target delegation's UpdateGeometryCallback detects geometry changes
  void updateTargetGeometry(osg::MatrixTransform* xform, const osg::Vec3d& ecef);

protected:
  virtual ~PlatformAzimElevViewTool();

private:
  osg::MatrixTransform* createAzElGrid_();

  void rebuild_();
  osg::Node* buildTargetGeometry_();
  void applyOverrides_();
  void applyOverrides_(bool enable);
  void applyOverrides_(EntityNode* node);
  void applyOverrides_(EntityNode* node, bool enable);
  bool isInstalled_() const;

private:
  osg::observer_ptr<EntityNode>  host_;
  EntityFamily                   family_;
  osg::observer_ptr<LocatorNode> root_;
  double                         range_;
  float                          elevLabelAngle_;
  simData::BeamPrefs             beamPrefs_;
  simData::GatePrefs             gatePrefs_;

  osg::ref_ptr<TargetDelegation>     targets_;
  osg::ref_ptr<HorizonGeoFence>      fence_;

  osg::ref_ptr<osg::MatrixTransform> grid_;
  osg::ref_ptr<osg::StateAttribute>  warpingProgram_;
  osg::ref_ptr<osg::Node>            targetGeom_;
};

} // namespace simVis

#endif // SIMVIS_PLATFORM_AZ_EL_VIEW_TOOL_H
