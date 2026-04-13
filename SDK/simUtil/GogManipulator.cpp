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
#include "osg/ComputeBoundsVisitor"
#include "simCore/Calc/Calculations.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simUtil/IconDragger.h"
#include "simUtil/GogManipulator.h"

namespace simUtil {

/** Given an invalid bounding sphere, the north offset value for rotation */
constexpr double DRAGGER_DEFAULT_ROTATE_NORTH_OFFSET_M = 10000.0;
/** Given a tiny GOG, the north offset minimum in meters */
constexpr double DRAGGER_MINIMUM_NORTH_OFFSET_M = 100.0;
/** Default distance relative to rotate dragger distance, for the scale dragger (sqrt(2)) */
constexpr double DRAGGER_DEFAULT_SCALE_MULTIPLIER = 1.4142136;
/** Angle for the scale dragger, in radians, from center */
constexpr double DRAGGER_SCALE_ANGLE_RAD = M_PI * 0.75; // 135 deg

GogManipulator::GogManipulator(osgEarth::MapNode* mapNode)
  : mapNode_(mapNode)
{
  // Setup Translation Dragger (Center)
  transDragger_ = new simUtil::IconDragger(mapNode_.get(), simVis::makeCursorTranslateImage());
  transDragger_->setNodeMask(0); // Hidden by default

  // Setup Rotation Dragger (Offset Handle)
  rotDragger_ = new simUtil::IconDragger(mapNode_.get(), simVis::makeCursorRotateImage());
  rotDragger_->setNodeMask(0);

  // Setup Scale Dragger (Corner Handle)
  scaleDragger_ = new simUtil::IconDragger(mapNode_.get(), simVis::makeCursorDiagonalLlUrImage());
  scaleDragger_->setNodeMask(0);

  addChild(transDragger_.get());
  addChild(rotDragger_.get());
  addChild(scaleDragger_.get());

  // Generate drag event callbacks
  auto makeDragHandler = [this](auto updateFunc) {
    return [this, updateFunc](const auto* sender, const auto& geoPoint) {
      if (!sender)
        return;

      if (sender->getDragging())
      {
        isDragging_ = true;
        (this->*updateFunc)(geoPoint);
      }
      else if (isDragging_)
      {
        isDragging_ = false;
        (this->*updateFunc)(geoPoint); // Ensure final position is applied

        // Notify the application layer that an edit just finished
        if (editFinishedCallback_ && activeGog_)
          editFinishedCallback_(activeGog_);
      }
      };
    };

  // Attach Callbacks
  transDragger_->onPositionChanged(makeDragHandler(&GogManipulator::handleTranslation_));
  rotDragger_->onPositionChanged(makeDragHandler(&GogManipulator::handleRotation_));
  scaleDragger_->onPositionChanged(makeDragHandler(&GogManipulator::handleScale_));
}

GogManipulator::~GogManipulator()
{
}

void GogManipulator::setTarget(std::shared_ptr<simVis::GOG::GogNodeInterface> gog)
{
  if (gog == activeGog_)
    return;

  activeGog_ = gog;
  if (!activeGog_ || !GogManipulator::canEdit(*activeGog_))
  {
    clearTarget();
    return;
  }

  isDragging_ = false;

  // Retrieve the current scale first, so we can normalize the bounding sphere
  osg::Vec3d scale(1., 1., 1.);
  activeGog_->getScale(scale);
  const double currentScale = (scale.x() == 0. ? 1.0 : scale.x());

  // Update the rotation distance based on bounding sphere; default to 10km
  rotationDistanceM_ = DRAGGER_DEFAULT_ROTATE_NORTH_OFFSET_M;
  auto* node = gog->osgNode();
  if (node)
  {
    // Use a ComputeBoundsVisitor instead of relying on bounding sphere due to GPU
    // artifacts on GOG making bounding radius potentially unreliable
    osg::ComputeBoundsVisitor cbv;
    node->accept(cbv);
    const auto& bb = cbv.getBoundingBox();

    if (bb.valid())
    {
      // Need the unscaled radius to position / calculate handle locations
      const double unscaledRadius = static_cast<double>(bb.radius()) / currentScale;
      rotationDistanceM_ = std::max(unscaledRadius, DRAGGER_MINIMUM_NORTH_OFFSET_M);
    }
  }

  // Upscale the scale position based on rotation distance
  baseScaleDistanceM_ = DRAGGER_DEFAULT_SCALE_MULTIPLIER * rotationDistanceM_;

  syncDraggersToGog_();

  // There are no absolute GOGs where translation applies
  if (activeGog_->isRelative())
    transDragger_->setNodeMask(~0);
  else
    transDragger_->setNodeMask(0);

  // Annotations cannot rotate
  if (activeGog_->shape() == simVis::GOG::GOG_ANNOTATION)
    rotDragger_->setNodeMask(0);
  else
    rotDragger_->setNodeMask(~0);

  // Everyone can scale
  scaleDragger_->setNodeMask(~0);
}

std::shared_ptr<simVis::GOG::GogNodeInterface> GogManipulator::target() const
{
  return activeGog_;
}

void GogManipulator::clearTarget()
{
  activeGog_.reset();
  transDragger_->setNodeMask(0);
  rotDragger_->setNodeMask(0);
  scaleDragger_->setNodeMask(0);
}

bool GogManipulator::hasTarget() const
{
  return activeGog_ != nullptr;
}

void GogManipulator::syncDraggersToGog_()
{
  if (!activeGog_ || !mapNode_)
    return;

  // Ensure the GOG has a valid, resolvable position before continuing
  osg::Vec3d pos;
  if (activeGog_->getReferencePosition(pos) != 0)
    return;

  // Construct a valid GeoPoint from the converted coordinates
  const osgEarth::GeoPoint centerGeo(mapNode_->getMapSRS(),
    pos.x(), pos.y(), 0., osgEarth::ALTMODE_ABSOLUTE);
  transDragger_->setPosition(centerGeo, false);

  double yawOffsetRad = 0.0;
  simCore::Vec3 scale(1, 1, 1);
  if (const auto* shape = activeGog_->shapeObject())
  {
    shape->getYawOffset(yawOffsetRad);
    shape->getScale(scale);
  }

  const double currentScale = scale.x();
  const double centerLatRad = osg::DegreesToRadians(centerGeo.y());
  const double centerLonRad = osg::DegreesToRadians(centerGeo.x());

  // Extract out the rotation node lat/lon based on yaw and distance
  double outLatRad = 0.;
  double outLonRad = 0.;
  simCore::sodanoDirect(centerLatRad, centerLonRad, 0.0,
    rotationDistanceM_ * currentScale, yawOffsetRad,
    &outLatRad, &outLonRad);

  // Create a GeoPoint and set the rotation dragger position
  const osgEarth::GeoPoint rotPos(mapNode_->getMapSRS(),
    osg::RadiansToDegrees(outLonRad), osg::RadiansToDegrees(outLatRad),
    0., osgEarth::ALTMODE_ABSOLUTE);
  rotDragger_->setPosition(rotPos, false);

  simCore::sodanoDirect(centerLatRad, centerLonRad, 0.0,
    baseScaleDistanceM_ * currentScale, yawOffsetRad + DRAGGER_SCALE_ANGLE_RAD,
    &outLatRad, &outLonRad);

  const osgEarth::GeoPoint scalePos(mapNode_->getMapSRS(),
    osg::RadiansToDegrees(outLonRad), osg::RadiansToDegrees(outLatRad),
    0., osgEarth::ALTMODE_ABSOLUTE);
  scaleDragger_->setPosition(scalePos, false);
}

void GogManipulator::handleTranslation_(const osgEarth::GeoPoint& newPos)
{
  if (!activeGog_)
    return;

  activeGog_->setReferencePosition(osg::Vec3d{ newPos.x(), newPos.y(), 0.0 });
  syncDraggersToGog_();
}

void GogManipulator::handleRotation_(const osgEarth::GeoPoint& handlePos)
{
  if (!activeGog_)
    return;

  const osgEarth::GeoPoint centerPos = transDragger_->getPosition();
  double bearingRad = 0.0;
  // Return value is geodesic distance in meters, which is ignored
  simCore::sodanoInverse(
    osg::DegreesToRadians(centerPos.y()), osg::DegreesToRadians(centerPos.x()), 0.0,
    osg::DegreesToRadians(handlePos.y()), osg::DegreesToRadians(handlePos.x()),
    &bearingRad);

  activeGog_->setYawOffset(bearingRad);
  syncDraggersToGog_();
}

void GogManipulator::handleScale_(const osgEarth::GeoPoint& handlePos)
{
  if (!activeGog_)
    return;

  const osgEarth::GeoPoint centerPos = transDragger_->getPosition();
  const double currentDistanceM = simCore::sodanoInverse(
    osg::DegreesToRadians(centerPos.y()), osg::DegreesToRadians(centerPos.x()), 0.0,
    osg::DegreesToRadians(handlePos.y()), osg::DegreesToRadians(handlePos.x()));

  // Prevent scaling all the way to 0 or negative
  if (currentDistanceM < 1.0)
    return;

  // Calculate uniform scale ratio
  const double scaleFactor = currentDistanceM / baseScaleDistanceM_;
  activeGog_->setScale({ scaleFactor, scaleFactor, 1.0 });

  syncDraggersToGog_();
}

bool GogManipulator::canEdit(const simVis::GOG::GogNodeInterface& gog)
{
  // No editing attached GOGs at all
  if (gog.isAttached())
    return false;

  // No editing Lat/Lon box or ImageOverlay; does not work
  const auto shape = gog.shape();
  if (shape == simVis::GOG::GOG_LATLONALTBOX || shape == simVis::GOG::GOG_IMAGEOVERLAY)
    return false;

  // Must be able to get a reference position cleanly
  osg::Vec3d refGeoPoint;
  if (gog.getReferencePosition(refGeoPoint) != 0)
    return false;
  return true;
}

void GogManipulator::setEditFinishedCallback(EditFinishedCallback cb)
{
  editFinishedCallback_ = std::move(cb);
}

bool GogManipulator::isOptInForGlobalEditing(const simVis::GOG::GogNodeInterface& gog)
{
  if (!GogManipulator::canEdit(gog))
    return false;
  return gog.editMode() == simCore::GOG::EditMode::GLOBAL;
}

}
