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
#ifndef SIMVIS_TARGET_DELEGATION_H
#define SIMVIS_TARGET_DELEGATION_H

#include "simCore/Common/Common.h"
#include "simVis/GeoFence.h"
#include "osg/Group"
#include "osg/MatrixTransform"
#include <osg/ref_ptr>

namespace simVis
{

class PlatformNode;

/**
 * This object will add a set of Platform "delegates" to the scene
 * graph. A delegate in this context is a secondary representation
 * of a target platform within the display, like a target projected
 * onto a local sensor display for example. The PlanetariumViewTool
 * and the PlatformAzimElevTool both use this class to show the
 * location of targets "projected" into a localized space.
 */
class SDKVIS_EXPORT TargetDelegation : public osg::Group
{
public:
  /**
   * Constructs a new target delegation.
   */
  TargetDelegation();

  /**
   * Sets a geofence to apply to the delegation
   */
  void setGeoFence(const GeoFence* fence);

  /**
   * Adds a delegate for a platform, or updates the delegate associated
   * with the platform if it already exists.
   */
  void addOrUpdate(const PlatformNode* platform);

  /**
   * Removes the delegate associated with a platform if it exists.
   */
  void remove(const PlatformNode* platform);

  /**
  * Removes all delegates
  */
  void removeAll();

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "TargetDelegation"; }

public:
  /// Callback that will create or update geometry when the
  /// target location changes.
  struct UpdateGeometryCallback : public osg::Referenced
  {
    /** Called when target location changes */
    virtual void operator()(osg::MatrixTransform* xform, const osg::Vec3d& ecef) = 0;
  };

  /**
   * Adds a callback that the Delegation will call to update the graphical
   * appearance of a target delegate.
   */
  void addUpdateGeometryCallback(UpdateGeometryCallback* cb);

protected:
  virtual ~TargetDelegation() { }

private:
  typedef std::map< const PlatformNode*, osg::ref_ptr<osg::MatrixTransform> > TargetNodeMap;

  TargetNodeMap            targetNodes_;
  osg::ref_ptr<const GeoFence>   fence_;

  typedef std::vector< osg::ref_ptr<UpdateGeometryCallback> > UpdateGeometryCallbacks;
  UpdateGeometryCallbacks updateGeometryCallbacks_;

  void updateGeometry_(osg::MatrixTransform* xform, const osg::Vec3d& ecef);
};

} // namespace simVis

#endif // SIMVIS_TARGET_DELEGATION_H
