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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
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
  return position_;
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
  osg::EllipsoidModel em;
  // See also: Scenario.cpp.  We need a horizon here to detect behind-earth coordinates
  em.setRadiusEquator(em.getRadiusEquator() - 11000.0);
  em.setRadiusPolar(em.getRadiusPolar() - 11000.0);
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
  const simVis::Locator* locator = entity.getLocator();
  if (!entity.isActive() || !locator)
    return INVALID_COORDINATE;

  // Overhead mode: Get the LLA position, clamp to 0, then convert to ECEF
  if (view_.valid() && view_->isOverheadEnabled())
  {
    simCore::Vec3 lla;
    if (!locator->getLocatorPosition(&lla, simCore::COORD_SYS_LLA))
      return INVALID_COORDINATE;
    lla.setAlt(0.0);
    simCore::Vec3 ecefOut;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(lla, ecefOut);
    return matrixCalculate_(osg::Vec3d(ecefOut.x(), ecefOut.y(), ecefOut.z()));
  }

  // Non-overhead mode: Get the locator matrix and pull out the XYZ translate transform
  osg::Matrix locatorMatrix;
  if (!locator->getLocatorMatrix(locatorMatrix))
    return INVALID_COORDINATE;
  return matrixCalculate_(locatorMatrix.getTrans());
}

ScreenCoordinate ScreenCoordinateCalculator::calculate(const simCore::Vec3& lla)
{
  // Refresh the VPW if needed, returning invalid coordinate if needed
  if (recalculateVPW_() != 0)
    return INVALID_COORDINATE;

  // this could be simplified to a coord conversion
  double alt = lla.alt();
  if (view_.valid() && view_->isOverheadEnabled())
    alt = 0.0;
  osg::Matrix ecefMatrix;
  osgEarth::SpatialReference::create("wgs84")->getEllipsoid()->computeLocalToWorldTransformFromLatLongHeight(lla.lat(), lla.lon(), alt, ecefMatrix);

  return matrixCalculate_(ecefMatrix.getTrans());
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
