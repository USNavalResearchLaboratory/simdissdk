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
#include <array>
#include "osg/ComputeBoundsVisitor"
#include "osg/NodeCallback"
#include "simCore/Calc/Calculations.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/AnimatedLine.h"
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

/** Lightweight callback that will hide the manipulators if the GOG interface node is hidden */
class GogManipulator::SynchronizeCallback : public osg::NodeCallback
{
public:
  /**
   * Helper that will sync the displayed content to the GOG itself, per-update, if it is different.
   * This fires off only when the editing is enabled, only on edited GOGs, to make sure the draw flag
   * is correct, as well as the position/ori/scale.
   */
  void synchronize(simUtil::GogManipulator& manip, const simVis::GOG::GogNodeInterface& gog)
  {
    // Handle transient visibility (e.g. draw flag)
    if (!simUtil::GogManipulator::canEdit(gog) || !gog.getDraw())
    {
      manip.handlesGroup_->setNodeMask(0);
      return;
    }

    manip.handlesGroup_->setNodeMask(~0);
    if (manip.isDragging_) // Don't fight with our own dragger math
      return;

    // Poll for GUI-driven state changes so we don't fall out of sync
    osg::Vec3d currentPos = cachedPos_;
    gog.getReferencePosition(currentPos);
    osg::Vec3d currentScale = cachedScale_;
    gog.getScale(currentScale);

    // Orientation offset comes from the underlying GOG Shape, the node interface does not expose a getter
    double currentYawRad = cachedYawRad_;
    if (const auto* shape = gog.shapeObject())
      shape->getYawOffset(currentYawRad);

    // If anything changed externally (from GUI, plug-ins, etc), resync
    if (currentPos != cachedPos_ || currentYawRad != cachedYawRad_ || currentScale != cachedScale_)
    {
      manip.syncDraggersToGog_();

      // Update cache
      cachedPos_ = currentPos;
      cachedYawRad_ = currentYawRad;
      cachedScale_ = currentScale;
    }
  }

  // From NodeVisitor:
  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) override
  {
    auto* manip = dynamic_cast<simUtil::GogManipulator*>(node);
    if (manip && manip->hasTarget())
      synchronize(*manip, *manip->target());
    traverse(node, nv);
  }

private:
  osg::Vec3d cachedPos_;
  double cachedYawRad_ = 0.0;
  osg::Vec3d cachedScale_ = osg::Vec3d(1, 1, 1);
};

////////////////////////////////////////////////////////////

GogManipulator::GogManipulator(osgEarth::MapNode* mapNode)
  : mapNode_(mapNode),
  handlesGroup_(new osg::Group),
  guideGroup_(new osg::Group)
{
  addChild(handlesGroup_.get());

  // Setup Translation Dragger (Center)
  transDragger_ = new simUtil::IconDragger(mapNode_.get(), simVis::makeCursorTranslateImage());
  transDragger_->setNodeMask(0); // Hidden by default

  // Setup Rotation Dragger (Offset Handle)
  rotDragger_ = new simUtil::IconDragger(mapNode_.get(), simVis::makeCursorRotateImage());
  rotDragger_->setNodeMask(0);

  // Setup Scale Dragger (Corner Handle)
  scaleDragger_ = new simUtil::IconDragger(mapNode_.get(), simVis::makeCursorDiagonalLlUrImage());
  scaleDragger_->setNodeMask(0);

  handlesGroup_->addChild(transDragger_.get());
  handlesGroup_->addChild(rotDragger_.get());
  handlesGroup_->addChild(scaleDragger_.get());
  handlesGroup_->addChild(guideGroup_.get());

  // Center to Rotation handle
  centerToRotLine_ = new simVis::AnimatedLineNode(2.0f, false);
  centerToRotLine_->setColor1(osg::Vec4(1.0f, 0.8f, 0.0f, 0.8f)); // Yellow, a bit more solid
  centerToRotLine_->setStipple1(0x0F0F);
  centerToRotLine_->setStipple2(0);
  centerToRotLine_->setShiftsPerSecond(0.);
  guideGroup_->addChild(centerToRotLine_.get());

  // Rotation-aligned bounding box with four edges
  for (int i = 0; i < 4; ++i)
  {
    auto* boxLine = new simVis::AnimatedLineNode(1.5f, false);
    boxLine->setColor1(osg::Vec4(1.0f, 0.8f, 0.0f, 0.6f)); // Yellow, a bit less solid
    boxLine->setStipple1(0x3333);
    boxLine->setStipple2(0);
    boxLine->setShiftsPerSecond(0.);
    boxLines_.push_back(boxLine);
    guideGroup_->addChild(boxLine);
  }

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

  setUpdateCallback(new SynchronizeCallback());
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

  // Annotations cannot rotate or scale; scaling can sometimes work but is inconsistent
  if (activeGog_->shape() == simVis::GOG::GOG_ANNOTATION)
  {
    rotDragger_->setNodeMask(0);
    scaleDragger_->setNodeMask(0);
    guideGroup_->setNodeMask(0);
  }
  else
  {
    rotDragger_->setNodeMask(~0);
    scaleDragger_->setNodeMask(~0);
    // Only show the guides if rotation and scale are shown, else the BB doesn't make sense
    guideGroup_->setNodeMask(~0);
  }
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
  double rotLatRad = 0.;
  double rotLonRad = 0.;
  simCore::sodanoDirect(centerLatRad, centerLonRad, 0.0,
    rotationDistanceM_ * currentScale, yawOffsetRad,
    &rotLatRad, &rotLonRad);

  const osgEarth::GeoPoint rotPos(mapNode_->getMapSRS(),
    osg::RadiansToDegrees(rotLonRad), osg::RadiansToDegrees(rotLatRad),
    0., osgEarth::ALTMODE_ABSOLUTE);
  rotDragger_->setPosition(rotPos, false);

  double scaleLatRad = 0.;
  double scaleLonRad = 0.;
  simCore::sodanoDirect(centerLatRad, centerLonRad, 0.0,
    baseScaleDistanceM_ * currentScale, yawOffsetRad + DRAGGER_SCALE_ANGLE_RAD,
    &scaleLatRad, &scaleLonRad);

  const osgEarth::GeoPoint scalePos(mapNode_->getMapSRS(),
    osg::RadiansToDegrees(scaleLonRad), osg::RadiansToDegrees(scaleLatRad),
    0., osgEarth::ALTMODE_ABSOLUTE);
  scaleDragger_->setPosition(scalePos, false);

  // Update position of the line for center-to-rotation point
  const simCore::Coordinate centerCoord(simCore::COORD_SYS_LLA, simCore::Vec3(centerLatRad, centerLonRad, 0.0));
  const simCore::Coordinate rotCoord(simCore::COORD_SYS_LLA, simCore::Vec3(rotLatRad, rotLonRad, 0.0));
  centerToRotLine_->setEndPoints(centerCoord, rotCoord);

  // Defines the 4 corners around the shape, in radians
  static constexpr std::array<double, 4> cornerAnglesRad = {
    M_PI * 0.25, M_PI * 0.75, M_PI * 1.25, M_PI * 1.75
  };

  // Update Bounding Box lines using the range and corner angles
  std::array<simCore::Coordinate, 4> cornerCoords;
  for (int i = 0; i < 4; ++i)
  {
    double cornerLatRad = 0.;
    double cornerLonRad = 0.;
    simCore::sodanoDirect(centerLatRad, centerLonRad, 0.0,
      baseScaleDistanceM_ * currentScale, yawOffsetRad + cornerAnglesRad[i],
      &cornerLatRad, &cornerLonRad);
    cornerCoords[i] = simCore::Coordinate(simCore::COORD_SYS_LLA,
      simCore::Vec3(cornerLatRad, cornerLonRad, 0.0));
  }

  // Set all the end points consecutively
  for (int i = 0; i < 4; ++i)
    boxLines_[i]->setEndPoints(cornerCoords[i], cornerCoords[(i + 1) % 4]);
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
  // If user requested "never editable", then we never edit
  if (gog.editMode() == simCore::GOG::EditMode::LOCKED)
    return false;

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

}
