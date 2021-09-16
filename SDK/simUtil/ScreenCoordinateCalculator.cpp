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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgEarth/Horizon"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/Entity.h"
#include "simVis/View.h"
#include "simUtil/ScreenCoordinateCalculator.h"

namespace simUtil
{

ScreenCoordinate::ScreenCoordinate(const osg::Vec3& position, bool outOfViewport, bool overHorizon)
  : position_(position),
    isOffScreen_(outOfViewport),
    isOverHorizon_(overHorizon)
{
}

osg::Vec2 ScreenCoordinate::position() const
{
  return osg::Vec2(position_.x(), position_.y());
}

osg::Vec3 ScreenCoordinate::positionV3() const
{
  // Due to a current (12/18/18) bug in osgEarth::LineDrawable, zero out the returned position.
  // Note that we need the position Z value to test for isBehindCamera() internally.
  return osg::Vec3f(position_.x(), position_.y(), 0.f);
}

bool ScreenCoordinate::isBehindCamera() const
{
  return position_.z() > 1.f;
}

bool ScreenCoordinate::isOffScreen() const
{
  return isOffScreen_;
}

bool ScreenCoordinate::isOverHorizon() const
{
  return isOverHorizon_;
}

////////////////////////////////////////////////////////////////////////

namespace {

/** Screen coordinate that is off screen and behind the eye. */
static const ScreenCoordinate INVALID_COORDINATE(osg::Vec3(-1, -1, 0), true, true);

}

ScreenCoordinateCalculator::ScreenCoordinateCalculator()
  : dirtyMatrix_(true)
{
  // 11km is rough depth of Mariana Trench; decrease radius to help horizon culling work underwater
#if OSGEARTH_SOVERSION >= 110
  osgEarth::Ellipsoid em;
  // See also: Scenario.cpp.  We need a horizon here to detect behind-earth coordinates
  em.setSemiMajorAxis(em.getRadiusEquator() - 11000.0);
  em.setSemiMinorAxis(em.getRadiusPolar() - 11000.0);
#else
  osg::EllipsoidModel em;
  // See also: Scenario.cpp.  We need a horizon here to detect behind-earth coordinates
  em.setRadiusEquator(em.getRadiusEquator() - 11000.0);
  em.setRadiusPolar(em.getRadiusPolar() - 11000.0);
#endif
  horizon_ = new osgEarth::Horizon(em);
}

ScreenCoordinateCalculator::~ScreenCoordinateCalculator()
{
}

void ScreenCoordinateCalculator::updateMatrix(const simVis::View& view)
{
  dirtyMatrix_ = true;
  view_ = &view;
}

ScreenCoordinate ScreenCoordinateCalculator::calculate(const simVis::EntityNode& entity)
{
  // Refresh the VPW if needed, returning invalid coordinate if needed
  if (recalculateVPW_() != 0)
    return INVALID_COORDINATE;

  // Check entity active flag
  if (!entity.isActive())
    return INVALID_COORDINATE;

  if (!view_.valid() || !view_->isOverheadEnabled())
  {
    simCore::Vec3 locatorNodeEcef;
    if (0 != entity.getPosition(&locatorNodeEcef, simCore::COORD_SYS_ECEF))
      return INVALID_COORDINATE;
    return matrixCalculate_(osg::Vec3d(locatorNodeEcef.x(), locatorNodeEcef.y(), locatorNodeEcef.z()));
  }

  // Overhead mode: Get the LLA position, clamp to 0, then convert to ECEF
  simCore::Vec3 lla;
  if (0 != entity.getPosition(&lla, simCore::COORD_SYS_LLA))
    return INVALID_COORDINATE;
  lla.setAlt(0.0);
  simCore::Vec3 ecefOut;
  simCore::CoordinateConverter::convertGeodeticPosToEcef(lla, ecefOut);
  return matrixCalculate_(osg::Vec3d(ecefOut.x(), ecefOut.y(), ecefOut.z()));
}

ScreenCoordinate ScreenCoordinateCalculator::calculateLla(const simCore::Vec3& lla)
{
  // Refresh the VPW if needed, returning invalid coordinate if needed
  if (recalculateVPW_() != 0)
    return INVALID_COORDINATE;
  simCore::Vec3 ecefOut;
  if (!view_.valid() || !view_->isOverheadEnabled())
    simCore::CoordinateConverter::convertGeodeticPosToEcef(lla, ecefOut);
  else
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(lla.lat(), lla.lon(), 0.0), ecefOut);
  return matrixCalculate_(osg::Vec3d(ecefOut.x(), ecefOut.y(), ecefOut.z()));
}

ScreenCoordinate ScreenCoordinateCalculator::calculateEcef(const simCore::Vec3& ecef)
{
  // Refresh the VPW if needed, returning invalid coordinate if needed
  if (recalculateVPW_() != 0)
    return INVALID_COORDINATE;
  if (!view_.valid() || !view_->isOverheadEnabled())
    return matrixCalculate_(osg::Vec3d(ecef.x(), ecef.y(), ecef.z()));

  // Clamping is required in overhead mode, so we need to convert to LLA
  simCore::Vec3 llaPos;
  if (simCore::CoordinateConverter::convertEcefToGeodeticPos(ecef, llaPos) != 0)
    return INVALID_COORDINATE;
  llaPos.setAlt(0.0);
  simCore::Vec3 clampedEcef;
  simCore::CoordinateConverter::convertGeodeticPosToEcef(llaPos, clampedEcef);
  return matrixCalculate_(osg::Vec3d(clampedEcef.x(), clampedEcef.y(), clampedEcef.z()));
}

int ScreenCoordinateCalculator::recalculateVPW_()
{
  // Break out if no changes
  if (!dirtyMatrix_)
    return 0;
  // Break out early on invalid view
  if (!view_.valid())
    return 1;

  // Combine the matrices
  const osg::Camera* camera = view_->getCamera();
  const osg::Viewport* viewport = camera->getViewport();
  horizon_->setEye(osg::Vec3d(0, 0, 0) * osg::Matrix::inverse(camera->getViewMatrix()));
  viewProjectionWindow_ = camera->getViewMatrix() * camera->getProjectionMatrix() * viewport->computeWindowMatrix();
  dirtyMatrix_ = false;
  return 0;
}

ScreenCoordinate ScreenCoordinateCalculator::matrixCalculate_(const osg::Vec3d& ecefCoordinate) const
{
  // Calculate the info for the coordinate
  const osg::Vec3 coordinate = ecefCoordinate * viewProjectionWindow_;
  bool isInside = false;
  if (view_.valid() && view_->getCamera() && view_->getCamera()->getViewport())
  {
    const osg::Viewport* vp = view_->getCamera()->getViewport();
    isInside = (coordinate.x() >= vp->x() && coordinate.x() <= (vp->x() + vp->width())) &&
      (coordinate.y() >= vp->y() && coordinate.y() <= (vp->y() + vp->height()));
  }

  // Check horizon culling
  const bool overHorizon = !horizon_->isVisible(ecefCoordinate);
  return ScreenCoordinate(coordinate, !isInside, overHorizon);
}

}
