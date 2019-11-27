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
#ifndef SIMVIS_EARTHMANIPULATOR_H
#define SIMVIS_EARTHMANIPULATOR_H

#include "osgEarth/EarthManipulator"
#include "simCore/Common/Export.h"

namespace osg { class View; }

namespace simVis
{

/**
 * Specialization of the osgEarth EarthManipulator for the SIMDIS SDK that permits
 * the locking of the heading and pitch, to prevent the end user to manipulate them
 * using the mouse.
 */
class SDKVIS_EXPORT EarthManipulator : public osgEarth::Util::EarthManipulator
{
public:
  EarthManipulator();

  /** Get the current fov */
  double fovY() const;
  /** Set the current fov */
  void setFovY(double fovy);

  /** Locks the heading.  When locked, the user cannot change the heading/azimuth of the camera. */
  void setHeadingLocked(bool lockHeading);
  /** Locks the pitch.  When locked, the user cannot change the pitch/elevation of the camera. */
  void setPitchLocked(bool lockPitch);

  /** Returns true if the heading is locked. */
  bool isHeadingLocked() const;
  /** Returns true if the pitch is locked. */
  bool isPitchLocked() const;

  /** @see osgEarth::Util::EarthManipulator::pan() */
  virtual void pan(double dx, double dy);
  /** @see osgEarth::Util::EarthManipulator::rotate() */
  virtual void rotate(double dx, double dy);
  /** @see osgEarth::Util::EarthManipulator::zoom() */
  virtual void zoom(double dx, double dy, osg::View* view);

  /** @see osgEarth::Util::EarthManipulator::handleMovementAction() */
  virtual void handleMovementAction(const ActionType& type, double dx, double dy, osg::View* view);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "EarthManipulator"; }

  /** Attach a node to the manipulator. Need to override so we maintain the original FOV value */
  virtual void setNode(osg::Node* node);

private:
  bool lockHeading_;
  bool lockPitch_;
};

}

#endif /* SIMVIS_EARTHMANIPULATOR_H */
