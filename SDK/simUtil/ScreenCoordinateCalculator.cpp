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
#include "simVis/View.h"
#include "simUtil/ScreenCoordinateCalculator.h"

namespace simUtil
{

ScreenCoordinate::ScreenCoordinate(const osg::Vec3& position, bool outOfViewport)
  : position_(position),
    isOffScreen_(outOfViewport)
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

////////////////////////////////////////////////////////////////////////

ScreenCoordinateCalculator::ScreenCoordinateCalculator()
  : dirtyMatrix_(true)
{
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
  {
    return ScreenCoordinate(osg::Vec3(-1, -1, 0), true);
  }

  // Check for invalid locator
  osg::Matrix locatorMatrix;
  if (!entity.isActive() || !entity.getLocator() || !entity.getLocator()->getLocatorMatrix(locatorMatrix))
  {
    return ScreenCoordinate(osg::Vec3(-1, -1, 0), true);
  }
  return matrixCalculate_(locatorMatrix);
}

ScreenCoordinate ScreenCoordinateCalculator::calculate(const simCore::Vec3& lla)
{
  // Refresh the VPW if needed, returning invalid coordinate if needed
  if (recalculateVPW_() != 0)
  {
    return ScreenCoordinate(osg::Vec3(-1, -1, 0), true);
  }

  osg::Matrix ecefMatrix;
  osgEarth::SpatialReference::create("wgs84")->getEllipsoid()->computeLocalToWorldTransformFromLatLongHeight(lla.lat(), lla.lon(), lla.alt(), ecefMatrix);

  return matrixCalculate_(ecefMatrix);
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
  viewProjectionWindow_ = camera->getViewMatrix() * camera->getProjectionMatrix() * viewport->computeWindowMatrix();
  dirtyMatrix_ = false;
  return 0;
}

ScreenCoordinate ScreenCoordinateCalculator::matrixCalculate_(const osg::Matrix& coordinateMatrix) const
{
  // Calculate the info for the coordinate
  const osg::Vec3 coordinate = osg::Vec3(0, 0, 0) * coordinateMatrix * viewProjectionWindow_;
  bool isInside = false;
  if (view_.valid() && view_->getCamera() && view_->getCamera()->getViewport())
  {
    const osg::Viewport* vp = view_->getCamera()->getViewport();
    isInside = (coordinate.x() >= vp->x() && coordinate.x() <= (vp->x() + vp->width())) &&
      (coordinate.y() >= vp->y() && coordinate.y() <= (vp->y() + vp->height()));
  }
  return ScreenCoordinate(coordinate, !isInside);
}

}
