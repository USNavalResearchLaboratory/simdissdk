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
#include "simCore/Calc/Math.h"
#include "simVis/Beam.h"
#include "simVis/CustomRendering.h"
#include "simVis/Laser.h"
#include "simVis/LobGroup.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Scenario.h"
#include "simVis/View.h"
#include "simUtil/ScreenCoordinateCalculator.h"
#include "simUtil/DynamicSelectionPicker.h"

namespace simUtil {

// pixel amount to test if a mouse position has moved enough to initiate a new pick
static const float MOUSE_MOVEMENT_PICK_THRESHOLD = 10.0;

/** GUI Event Handler that forwards events to the picker */
class DynamicSelectionPicker::RepickEventHandler : public osgGA::GUIEventHandler
{
public:
  explicit RepickEventHandler(DynamicSelectionPicker& picker)
    : picker_(picker),
      repickNeeded_(false)
  {
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::DRAG:
      // on move or drag, update position and indicate a repick is needed
      picker_.lastMouseView_ = dynamic_cast<simVis::View*>(aa.asView());
      picker_.mouseXy_.set(ea.getX(), ea.getY());
      repickNeeded_ = true;
      break;

    case osgGA::GUIEventAdapter::PUSH:
      // if mouse position has changed, initiated a repick immediately, since push event can occur before move/drag has updated repick state
      if (fabs(ea.getX() - picker_.mouseXy_.x()) > MOUSE_MOVEMENT_PICK_THRESHOLD ||
        fabs(ea.getY() - picker_.mouseXy_.y()) > MOUSE_MOVEMENT_PICK_THRESHOLD)
      {
        picker_.lastMouseView_ = dynamic_cast<simVis::View*>(aa.asView());
        picker_.mouseXy_.set(ea.getX(), ea.getY());
        picker_.pickThisFrame_();
      }

    case osgGA::GUIEventAdapter::FRAME:
      // If the mouse moved, we need to re-pick to capture movement
      if (repickNeeded_)
      {
        repickNeeded_ = false;
        picker_.pickThisFrame_();
      }
      break;

    default:
      // Most events: do nothing
      break;
    }
    // Never intercept an event
    return false;
  }

private:
  DynamicSelectionPicker& picker_;
  bool repickNeeded_;
  osg::Vec2d mouseXy_;
};


///////////////////////////////////////////////////////

DynamicSelectionPicker::DynamicSelectionPicker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager)
  : Picker(scenarioManager->getOrCreateStateSet()),
    viewManager_(viewManager),
    scenario_(scenarioManager),
    maximumValidRange_(100.0), // pixels
    pickMask_(simVis::DISPLAY_MASK_PLATFORM|simVis::DISPLAY_MASK_PLATFORM_MODEL),
    platformAdvantagePct_(0.7)
{
  // By default, only platforms are picked.  Gates are feasibly pickable though.
  guiEventHandler_ = new RepickEventHandler(*this);
  addHandlerToViews_ = new simVis::AddEventHandlerToViews(guiEventHandler_.get());
  if (viewManager_.valid())
  {
    addHandlerToViews_->addToViews(*viewManager_);
    viewManager_->addCallback(addHandlerToViews_.get());
  }
}

DynamicSelectionPicker::~DynamicSelectionPicker()
{
  if (viewManager_.valid())
  {
    addHandlerToViews_->removeFromViews(*viewManager_);
    viewManager_->removeCallback(addHandlerToViews_.get());
  }
}

void DynamicSelectionPicker::setPlatformAdvantagePct(double platformAdvantage)
{
  platformAdvantagePct_ = osg::clampBetween(platformAdvantage, 0.0, 1.0);
}

void DynamicSelectionPicker::pickToVector(simVis::EntityVector& nodes, const osg::Vec2d& mouseXy, PickBehavior behavior)
{
  nodes.clear();
  mouseXy_ = mouseXy;

  // Figure out the view under the mouse
  lastMouseView_ = viewManager_->getViewByMouseXy(mouseXy);
  if (!lastMouseView_.valid())
    return;

  double mouseRangeSquaredPx = 0.;
  pickToVector_(nodes, behavior, mouseRangeSquaredPx);
}

void DynamicSelectionPicker::pickThisFrame_()
{
  simVis::EntityVector nodes;
  double mouseRangeSquaredPx = 0.;
  pickToVector_(nodes, PickBehavior::Closest, mouseRangeSquaredPx);

  simVis::EntityNode* picked = nullptr;
  if (nodes.size() == 1)
    picked = nodes.front().get();
  else if (!nodes.empty())
  {
    // Need to deconflict to pick best selection. We know:
    // * 0th item is going to be earliest created entity.
    // * Platforms must be created before attachments.
    // * Attachments to platforms (beams, lasers, LOBs, etc.) are most likely to be the colocated entity.
    // * It's also possible, though more rare, to have custom rendering as 0th item
    // Using this, we apply a preference to 0th item (most likely a platform) since they're most likely
    // to be the desired entity. However, fall back to later items (likely attachments) the farther the
    // mouse is from the center.
    const double mouseRangePx = sqrt(mouseRangeSquaredPx);
    const double platformAdvantagePx = platformAdvantagePct_ * maximumValidRange_;
    picked = (mouseRangePx < platformAdvantagePx) ? nodes[0].get() : nodes[1].get();
  }

  // Set the picked entity
  const unsigned int objectIndexTag = picked ? picked->objectIndexTag() : 0;
  setPicked_(objectIndexTag, picked);
}

void DynamicSelectionPicker::pickToVector_(simVis::EntityVector& nodes, PickBehavior behavior, double& mouseRangeSquaredPx) const
{
  // Create a calculator for screen coordinates
  simUtil::ScreenCoordinateCalculator calc;
  calc.updateMatrix(*lastMouseView_);
  nodes.clear();

  // Request all entities from the scenario
  simVis::EntityVector allEntities;
  scenario_->getAllEntities(allEntities);

  // We square the range to avoid sqrt() in a tight loop
  const double maximumValidRangeSquared = osg::square(maximumValidRange_);
  mouseRangeSquaredPx = maximumValidRangeSquared;

  // Loop through all entities
  for (const auto& entityRefPtr : allEntities)
  {
    auto* entityPtr = entityRefPtr.get();
    if (!isPickable_(entityPtr))
      continue;

    // Ask the calculator for the range from the mouse position
    double rangeSquared;
    if (calculateSquaredRange_(calc, *entityRefPtr, rangeSquared) != 0)
      continue;

    if (behavior == PickBehavior::AllInRange)
    {
      if (rangeSquared <= mouseRangeSquaredPx)
        nodes.emplace_back(entityPtr);
    }
    else
    {
      // PickBehavior::Closest: Choose the closest object
      if (rangeSquared < mouseRangeSquaredPx)
      {
        mouseRangeSquaredPx = rangeSquared;
        nodes = { entityPtr };
      }
      else if (rangeSquared == mouseRangeSquaredPx)
        nodes.emplace_back(entityPtr);
    }
  }
}

bool DynamicSelectionPicker::isPickable_(const simVis::EntityNode* entityNode) const
{
  // Avoid nullptr and things that don't match the mask
  if (entityNode == nullptr || (entityNode->getNodeMask() & pickMask_) == 0)
    return false;
  // Only pick entities with object index tags
  if (entityNode->objectIndexTag() == 0)
    return false;

  // Do not pick inactive or invisible entities
  return entityNode->isActive() && entityNode->isVisible();
}

int DynamicSelectionPicker::calculateSquaredRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::EntityNode& entityNode, double& rangeSquared) const
{
  // Fall back to the LOB case if it's requesting a LOB, since it picks individual points on the lines shown
  const simVis::LobGroupNode* lobNode = dynamic_cast<const simVis::LobGroupNode*>(&entityNode);
  if (lobNode)
    return calculateLobSquaredRange_(calc, *lobNode, rangeSquared);

  // Fall back to the CustomRender case if it's requesting a CustomRender, since it picks different points depending on its type
  const simVis::CustomRenderingNode* customNode = dynamic_cast<const simVis::CustomRenderingNode*>(&entityNode);
  if (customNode)
    return calculateCustomRenderRange_(calc, *customNode, rangeSquared);

  // Fall back to the Laser case if it's requesting a Laser, since it picks a line segment
  const simVis::LaserNode* laserNode = dynamic_cast<const simVis::LaserNode*>(&entityNode);
  if (laserNode)
    return calculateLaserRange_(calc, *laserNode, rangeSquared);

  // Fall back to the Beam case if it's requesting a Beam, since it picks along the boresight
  const simVis::BeamNode* beamNode = dynamic_cast<const simVis::BeamNode*>(&entityNode);
  if (beamNode)
    return calculateBeamRange_(calc, *beamNode, rangeSquared);

  const simUtil::ScreenCoordinate& pos = calc.calculate(entityNode);
  // Ignore objects that are off screen or behind the camera
  if (pos.isBehindCamera() || pos.isOffScreen() || pos.isOverHorizon())
    return 1;
  rangeSquared = (mouseXy_ - pos.position()).length2();
  return 0;
}

int DynamicSelectionPicker::calculateLobSquaredRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::LobGroupNode& lobNode, double& rangeSquared) const
{
  // Pull out the vector of all endpoints on the LOB that are visible
  std::vector<osg::Vec3d> ecefVec;
  lobNode.getVisibleEndPoints(ecefVec);

  // Check the distance from the whole line segment, not just the end points
  return calculateScreenRangeSegments_(calc, ecefVec, rangeSquared);
}

int DynamicSelectionPicker::calculateCustomRenderRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::CustomRenderingNode& customNode, double& rangeSquared) const
{
  // Pull out the vector of all pick points on the CustomRendering that are visible
  std::vector<osg::Vec3d> ecefVec;
  customNode.getPickingPoints(ecefVec);

  // for lines, check the distance from the whole line segment
  if (customNode.isLine())
    return calculateScreenRangeSegments_(calc, ecefVec, rangeSquared);
  // otherwise just check distance from the picking points
  return calculateScreenRangePoints_(calc, ecefVec, rangeSquared);
}

int DynamicSelectionPicker::calculateLaserRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::LaserNode& laserNode, double& rangeSquared) const
{
  // Pull out the vector of all endpoints on the Laser that are visible
  std::vector<osg::Vec3d> ecefVec;
  laserNode.getVisibleEndPoints(ecefVec);

  // Check the distance from the whole line segment, not just the end points
  return calculateScreenRangeSegments_(calc, ecefVec, rangeSquared);
}

int DynamicSelectionPicker::calculateBeamRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::BeamNode& beamNode, double& rangeSquared) const
{
  // Pull out the vector of all endpoints on the Beam that are visible
  std::vector<osg::Vec3d> ecefVec;
  beamNode.getVisibleEndPoints(ecefVec);

  // Check the distance from the whole line segment, not just the end points
  return calculateScreenRangeSegments_(calc, ecefVec, rangeSquared);
}

int DynamicSelectionPicker::calculateScreenRangePoints_(simUtil::ScreenCoordinateCalculator& calc, const std::vector<osg::Vec3d>& ecefVec, double& rangeSquared) const
{
  rangeSquared = std::numeric_limits<double>::max();

  for (auto i = ecefVec.begin(); i != ecefVec.end(); ++i)
  {
    const simUtil::ScreenCoordinate& pos = calc.calculateEcef(simCore::Vec3(i->x(), i->y(), i->z()));
    // Ignore objects that are off screen or behind the camera
    if (pos.isBehindCamera() || pos.isOffScreen() || pos.isOverHorizon())
      continue;
    rangeSquared = simCore::sdkMin(rangeSquared, (mouseXy_ - pos.position()).length2());
  }

  return (rangeSquared == std::numeric_limits<double>::max()) ? 1 : 0;
}

int DynamicSelectionPicker::calculateScreenRangeSegments_(simUtil::ScreenCoordinateCalculator& calc, const std::vector<osg::Vec3d>& ecefVec, double& rangeSquared) const
{
  const size_t numVerts = ecefVec.size();
  if (numVerts < 2)
    return 1;
  rangeSquared = std::numeric_limits<double>::max();

  osg::Vec3d ecefPoint = ecefVec.front();
  simUtil::ScreenCoordinate point1 = calc.calculateEcef(simCore::Vec3(ecefPoint.x(), ecefPoint.y(), ecefPoint.z()));
  for (unsigned int i = 1; i < numVerts; ++i)
  {
    osg::Vec3d ecefPoint = ecefVec[i];
    simUtil::ScreenCoordinate point2 = calc.calculateEcef(simCore::Vec3(ecefPoint.x(), ecefPoint.y(), ecefPoint.z()));
    rangeSquared = simCore::sdkMin(rangeSquared, lineSegmentDistanceSquared_(point1.position(), point2.position(), mouseXy_));
    point1 = point2;
  }

  return (rangeSquared == std::numeric_limits<double>::max()) ? 1 : 0;
}

double DynamicSelectionPicker::lineSegmentDistanceSquared_(const osg::Vec2d& a, const osg::Vec2d& b, const osg::Vec2d& p) const
{
  /*
  * Calculates the squared distance from point p to a line segment defined by (a,b).
  * http://www.randygaul.net/2014/07/23/distance-point-to-line-segment/
  */
  const osg::Vec2d normalizedSegment = b - a;
  const osg::Vec2d vecAtoP = a - p;

  const float c1 = normalizedSegment * vecAtoP;
  // Closest point is point a
  if (c1 > 0.f)
    return vecAtoP * vecAtoP;

  const osg::Vec2d vecPtoB = p - b;
  // Closest point is point b
  if (normalizedSegment * vecPtoB > 0.f)
    return vecPtoB * vecPtoB;

  osg::Vec2d e;
  if (b != a)
    // Closest point is between a and b, find the projection onto that line
    e = vecAtoP - normalizedSegment * (c1 / (normalizedSegment * normalizedSegment));
  else
    // Points are the same, no projection necessary (avoid divide-by-zero)
    e = vecAtoP;

  return e * e;
}

void DynamicSelectionPicker::setRange(double pixelsFromCenter)
{
  maximumValidRange_ = pixelsFromCenter;
}

void DynamicSelectionPicker::setPickMask(osg::Node::NodeMask pickMask)
{
  pickMask_ = pickMask;
}

osg::Node::NodeMask DynamicSelectionPicker::pickMask() const
{
  return pickMask_;
}

}
