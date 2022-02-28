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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simVis/Entity.h"
#include "simVis/View.h"
#include "simVis/EarthManipulator.h"

namespace simVis
{
// When trying to zoom to an absolute value less than this, zoom will instead change the sign of the range
static const float DISTANCE_CROSS_ZERO_THRESHOLD = .01;

EarthManipulator::EarthManipulator()
  : osgEarth::Util::EarthManipulator(),
    lockHeading_(false),
    lockPitch_(false)
{
}

double EarthManipulator::fovY() const
{
  return _lastKnownVFOV;
}

void EarthManipulator::setFovY(double fovy)
{
  if (_lastKnownVFOV == fovy)
    return;
  _lastKnownVFOV = fovy;
}

void EarthManipulator::setHeadingLocked(bool lockHeading)
{
  lockHeading_ = lockHeading;
}

void EarthManipulator::setPitchLocked(bool lockPitch)
{
  lockPitch_ = lockPitch;
}

bool EarthManipulator::isHeadingLocked() const
{
  return lockHeading_;
}

bool EarthManipulator::isPitchLocked() const
{
  return lockPitch_;
}

void EarthManipulator::pan(double dx, double dy)
{
  // Drop dx/dy if locking both heading and pitch
  if (isHeadingLocked() && isPitchLocked())
    return;
  if (isHeadingLocked())
    dx = 0;
  if (isPitchLocked())
    dy = 0;
  osgEarth::Util::EarthManipulator::pan(dx, dy);
}

void EarthManipulator::rotate(double dx, double dy)
{
  // Drop dx/dy if locking both heading and pitch
  if (isHeadingLocked() && isPitchLocked())
    return;
  if (isHeadingLocked())
    dx = 0;
  if (isPitchLocked())
    dy = 0;
  osgEarth::Util::EarthManipulator::rotate(dx, dy);
}

void EarthManipulator::zoom(double dx, double dy, osg::View* view)
{
  if (_distance < DISTANCE_CROSS_ZERO_THRESHOLD && _distance > -DISTANCE_CROSS_ZERO_THRESHOLD)
  {
    _distance = ((dy < 0) != (_distance < 0)) ? -_distance : _distance;
  }
  // Prevents actions which zoom in or out from having the opposite effect at negative distances
  if (_distance < 0)
  {
    dy = -dy;
  }
  else if (_distance == 0)
  {
    _distance = (dy < 0) ? -DISTANCE_CROSS_ZERO_THRESHOLD : DISTANCE_CROSS_ZERO_THRESHOLD;
  }
  // recalculate the center since osgEarth no longer does this, SIM-10727
  if (!isTethering())
    recalculateCenterFromLookVector();
  osgEarth::Util::EarthManipulator::zoom(dx, dy, view);
}

void EarthManipulator::handleMovementAction(const ActionType& type, double dx, double dy, osg::View* view)
{
  // Some actions need to turn off watch mode before being processed
  simVis::View* simVisView = dynamic_cast<simVis::View*>(view);
  // WatchEnabled or TetherMode other than TETHER_CENTER requires extra processing to avoid leaving artifacts when breaking watch/tether
  bool tetherHeading = false;
  if (getSettings() && (getSettings()->getTetherMode() != osgEarth::EarthManipulator::TETHER_CENTER))
  {
    tetherHeading = true;
    // Setting the tether mode doesn't fix the rotation artifact, but it does prevent this block from being triggered repeatedly
    getSettings()->setTetherMode(osgEarth::EarthManipulator::TETHER_CENTER);
  }
  if (simVisView && (simVisView->isWatchEnabled() || tetherHeading))
  {
    // Disable watch mode if we're in watch mode and encounter a break-tether action
    const ActionTypeVector& atv = getSettings()->getBreakTetherActions();
    // Rotation doesn't break tether completely, but it does break the heading portion of a tether
    if (std::find(atv.begin(), atv.end(), type) != atv.end() || (tetherHeading && type == EarthManipulator::ACTION_ROTATE))
    {
      // Set up a tether node, which will get broken cleanly in the handleMovementAction().  Note
      // that calling enableWatchMode() here directly will not be clean because there are side
      // effects to enableWatchMode() that need to be delayed until later (i.e. during the
      // broken tether callback)
      simVis::Viewpoint vp;
      vp.setNode(simVisView->getWatcherNode());
      setViewpoint(vp);
    }
  }

  // Fall back to base class implementation
  osgEarth::Util::EarthManipulator::handleMovementAction(type, dx, dy, view);
}

void EarthManipulator::setNode(osg::Node* node)
{
  double tmpFov = fovY();
  osgEarth::Util::EarthManipulator::setNode(node);
  // the EarthManipulator resets its FOV to the default in setNode(), so make sure it gets updated properly
  setFovY(tmpFov);
}

}
