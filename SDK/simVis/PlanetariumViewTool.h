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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_PLANETERIUM_VIEW_TOOL_H
#define SIMVIS_PLANETERIUM_VIEW_TOOL_H

#include "simCore/Common/Common.h"
#include "simVis/Entity.h"
#include "simVis/EntityFamily.h"
#include "simVis/Tool.h"
#include "simVis/TargetDelegation.h"

#include <osg/ref_ptr>
#include <osg/observer_ptr>

namespace osg {
  class Geode;
  class Vec4f;
}

namespace simVis
{

class PlatformNode;
class ScenarioManager;

/**
 * PlanetariumViewTool is a tool that draws a translucent dome
 * or sphere around a platform and "projects" the platform's
 * sensors and targets onto the surface of the dome.
 */
class SDKVIS_EXPORT PlanetariumViewTool : public ScenarioTool
{
public:
  /**
   * Constructs a new dome/sensor viewing tool.
   * @param[in ] host View will center on this host.
   */
  PlanetariumViewTool(PlatformNode* host);

  /**
   * Range of the sensor intersection dome from the host
   * @param[in ] range Range from the host in meters
   */
  void setRange(double range);
  /** Retrieve the range of sensor intersection dome from the host, in meters */
  double getRange() const { return range_; }

  /**
   * Color of the transparent dome/sphere
   * @param[in ] color Color
   */
  void setColor(const osg::Vec4f& color);
  /** Retrieves the color of the transparent dome/sphere (RGBA OSG style) */
  const osg::Vec4f& getColor() const { return domeColor_; }

  /**
   * Whether to display target vectors - vectors from the dome's
   * surface to the target.
   */
  void setDisplayTargetVectors(bool value);
  /** Retrieves whether to show target vectors */
  bool getDisplayTargetVectors() const { return displayTargetVectors_; }

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

  /// returns the node to display in the scenario graph
  osg::Node* getNode() const { return root_.get(); }

public: // internal

  /// Updates the geometries on the dome when target delegation's UpdateGeometryCallback detects geometry changes
  void updateTargetGeometry(osg::MatrixTransform*, const osg::Vec3d&);

protected:

  virtual ~PlanetariumViewTool() { }

private:
  EntityFamily                   family_;

  osg::observer_ptr<PlatformNode> host_;
  osg::observer_ptr<LocatorNode>  root_;
  /// planetarium radius, in meters
  double                          range_;
  osg::Vec4f                      domeColor_;
  simData::BeamPrefs              beamPrefs_;
  simData::GatePrefs              gatePrefs_;
  bool                            displayTargetVectors_;

  osg::observer_ptr<const ScenarioManager> scenario_;

  osg::ref_ptr<TargetDelegation> targets_;
  osg::ref_ptr<HorizonGeoFence>  fence_;

  void applyOverrides_(bool enable);
  void applyOverrides_(EntityNode* node, bool enable);

  void updateDome_();
  /**
  * Scales the geometry used to represent targets, based on input range
  * @param[in ] geode  the osg::Geode that contains the geometry
  * @param[in ] range (the planetarium radius) in meters
  */
  void scaleTargetGeode_(osg::Geode* geode, double range) const;
  osg::Geode* buildVectorGeode_();

  osg::ref_ptr<osg::Geode> dome_;
  osg::ref_ptr<osg::Geode> targetGeode_;
};

} // namespace simVis

#endif // SIMVIS_PLANETERIUM_VIEW_TOOL_H
