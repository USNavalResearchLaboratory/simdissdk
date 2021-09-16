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
#include "osgEarth/Terrain"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/LabelContentManager.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/Utils.h"
#include "simVis/Entity.h"
#include "simVis/Projector.h"

#undef LC
#define LC "[EntityNode] "

/// The highest available Level of Detail from ElevationPool
static const unsigned int MAX_LOD = 23;

//----------------------------------------------------------------------------
namespace simVis
{

CoordSurfaceClamping::CoordSurfaceClamping()
  : useMaxElevPrec_(false)
{}

CoordSurfaceClamping::~CoordSurfaceClamping()
{
}

void CoordSurfaceClamping::clampCoordToMapSurface(simCore::Coordinate& coord)
{
  // nothing to do if we don't have valid ways of accessing elevation
  if (!mapNode_.valid())
  {
    assert(0); // called this method without setting the map node
    return;
  }
  if (coord.coordinateSystem() != simCore::COORD_SYS_LLA && coord.coordinateSystem() != simCore::COORD_SYS_ECEF)
  {
    // coordinate type must be LLA to work with osgEarth elevation query
    assert(0);
    return;
  }

  // convert from ECEF to LLA if necessary, since osgEarth Terrain getHeight requires LLA
  simCore::Coordinate llaCoord;
  if (coord.coordinateSystem() == simCore::COORD_SYS_ECEF)
    simCore::CoordinateConverter::convertEcefToGeodetic(coord, llaCoord);
  else
    llaCoord = coord;

  // If getting the elevation fails, default to 0 to clamp to sea level
  double elevation = 0;

  // Both methods for getting terrain elevation have drawbacks that make them undesirable in certain situations. SIM-10423
  // getHeight() can give inaccurate results depending on how much map data is loaded into the scene graph, while ElevationEnvelope can be prohibitively slow if there are many clamped entities
  if (useMaxElevPrec_)
  {
    osgEarth::GeoPoint point(mapNode_->getMapSRS(), llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, 0, osgEarth::ALTMODE_ABSOLUTE);
    osgEarth::ElevationSample sample = mapNode_->getMap()->getElevationPool()->getSample(point, osgEarth::Distance(1.0, osgEarth::Units::METERS), &workingSet_);
    if (sample.hasData())
      elevation = sample.elevation().as(osgEarth::Units::METERS);
  }
  else
  {
    double hamsl = 0.0; // not used
    double hae = 0.0; // height above ellipsoid, the rough elevation

    if (mapNode_->getTerrain()->getHeight(mapNode_->getMapSRS(), llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, &hamsl, &hae))
      elevation = hae;
  }

  llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), elevation);

  // convert back to ECEF if necessary
  if (coord.coordinateSystem() == simCore::COORD_SYS_ECEF)
    simCore::CoordinateConverter::convertGeodeticToEcef(llaCoord, coord);
  else
    coord = llaCoord;
}

void CoordSurfaceClamping::clampCoordToMapSurface(simCore::Coordinate& coord, osgEarth::ElevationPool::WorkingSet& workingSet)
{
  // nothing to do if we don't have valid ways of accessing elevation
  if (!mapNode_.valid())
  {
    assert(0); // called this method without setting the map node
    return;
  }
  if (coord.coordinateSystem() != simCore::COORD_SYS_LLA && coord.coordinateSystem() != simCore::COORD_SYS_ECEF)
  {
    // coordinate type must be LLA to work with osgEarth elevation query
    assert(0);
    return;
  }

  // convert from ECEF to LLA if necessary, since osgEarth Terrain getHeight requires LLA
  simCore::Coordinate llaCoord;
  if (coord.coordinateSystem() == simCore::COORD_SYS_ECEF)
    simCore::CoordinateConverter::convertEcefToGeodetic(coord, llaCoord);
  else
    llaCoord = coord;

  // If getting the elevation fails, default to 0 to clamp to sea level
  double elevation = 0;

  // Both methods for getting terrain elevation have drawbacks that make them undesirable in certain situations. SIM-10423
  // getHeight() can give inaccurate results depending on how much map data is loaded into the scene graph, while ElevationEnvelope can be prohibitively slow if there are many clamped entities
  if (useMaxElevPrec_)
  {
    osgEarth::GeoPoint point(mapNode_->getMapSRS(), llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, 0, osgEarth::ALTMODE_ABSOLUTE);
    osgEarth::ElevationSample sample = mapNode_->getMap()->getElevationPool()->getSample(point, osgEarth::Distance(1.0, osgEarth::Units::METERS), &workingSet_);
    if (sample.hasData())
      elevation = sample.elevation().as(osgEarth::Units::METERS);
  }
  else
  {
    double hamsl = 0.0; // not used
    double hae = 0.0; // height above ellipsoid, the rough elevation

    if (mapNode_->getTerrain()->getHeight(mapNode_->getMapSRS(), llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, &hamsl, &hae))
      elevation = hae;
  }

  llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), elevation);

  // convert back to ECEF if necessary
  if (coord.coordinateSystem() == simCore::COORD_SYS_ECEF)
    simCore::CoordinateConverter::convertGeodeticToEcef(llaCoord, coord);
  else
    coord = llaCoord;
}

bool CoordSurfaceClamping::isValid() const
{
  return mapNode_.valid();
}

void CoordSurfaceClamping::setMapNode(const osgEarth::MapNode* map)
{
  mapNode_ = map;
#ifdef HAVE_WORKINGSET_STRONGLRU
  workingSet_.clear();
#else
  workingSet_._lru.clear();
#endif
}

void CoordSurfaceClamping::setUseMaxElevPrec(bool useMaxElevPrec)
{
  if (useMaxElevPrec_ == useMaxElevPrec)
    return;

  useMaxElevPrec_ = useMaxElevPrec;
}

/////////////////////////////////////////////////////////////////////////////////

EntityNode::EntityNode(simData::ObjectType type, Locator* locator)
  : type_(type),
    contentCallback_(new NullEntityCallback())
{
  setNodeMask(0);  // Draw is off until a valid update is received
  setLocator(locator);
}

EntityNode::~EntityNode()
{
  acceptProjector(nullptr);
  setLocator(nullptr);
}

bool EntityNode::isVisible() const
{
  return getNodeMask() != 0;
}

void EntityNode::setLocator(simVis::Locator* locator)
{
  if (locator_.valid() && locator_ == locator)
    return; // nothing to do
  locator_ = locator;
  if (locator_)
    locator_->dirty();
}

int EntityNode::getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys) const
{
  if (!locator_.valid())
    return 1;
  if (locator_->getLocatorPosition(out_position, coordsys))
    return 0;
  return 2;
}

int EntityNode::getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, simCore::CoordinateSystem coordsys) const
{
  if (!locator_.valid())
    return 1;
  if (locator_->getLocatorPositionOrientation(out_position, out_orientation, coordsys))
    return 0;
  return 2;
}

void EntityNode::attach(osg::Node* node, unsigned int comp)
{
  if (comp == Locator::COMP_NONE)
  {
    this->addChild(node);
  }
  else if (comp == Locator::COMP_ALL)
  {
    this->addChild(new LocatorNode(getLocator(), node));
  }
  else
  {
    this->addChild(new LocatorNode(new Locator(getLocator(), comp), node));
  }
}

void EntityNode::attach(osg::Node* node)
{
  unsigned int comp = Locator::COMP_ALL;
  EntityAttachable* attachable = dynamic_cast<EntityAttachable*>(node);
  if (attachable)
  {
    comp = attachable->getLocatorComponents();
  }

  attach(node, comp);
}

std::string EntityNode::getEntityName_(const simData::CommonPrefs& common, EntityNode::NameType nameType, bool allowBlankAlias) const
{
  switch (nameType)
  {
  case EntityNode::REAL_NAME:
    return common.name();
  case EntityNode::ALIAS_NAME:
    return common.alias();
  case EntityNode::DISPLAY_NAME:
    if (common.usealias())
    {
      if (!common.alias().empty() || allowBlankAlias)
        return common.alias();
    }
    return common.name();
  }
  return "";
}

int EntityNode::applyProjectorPrefs_(const simData::CommonPrefs& lastPrefs, const simData::CommonPrefs& prefs)
{
  if (!PB_FIELD_CHANGED(&lastPrefs, &prefs, acceptprojectorid))
    return 1;

  auto id = prefs.acceptprojectorid();
  if (id == 0)
    return acceptProjector(nullptr);

  auto projectorNode = dynamic_cast<ProjectorNode*>(nodeGetter_(id));
  if (projectorNode == nullptr)
    return 1;

  return acceptProjector(projectorNode);
}

void EntityNode::setLabelContentCallback(LabelContentCallback* cb)
{
  if (cb == nullptr)
    contentCallback_ = new NullEntityCallback();
  else
    contentCallback_ = cb;
}

LabelContentCallback& EntityNode::labelContentCallback() const
{
  return *contentCallback_;
}

int EntityNode::acceptProjector(ProjectorNode* proj)
{
  // Stop accepting the previous projector node, if one exists
  osg::ref_ptr<simVis::ProjectorNode> lock;
  if (acceptedProjectorNode_.lock(lock))
  {
    acceptedProjectorNode_->removeProjectionFromNode(this);
    acceptedProjectorNode_ = nullptr;
  }

  // Passing in NULL clears the pairing, not an error
  if (proj == nullptr)
    return 0;

  int rv = proj->addProjectionToNode(this, this);
  if (rv == 0)
    acceptedProjectorNode_ = proj;
  return 0;
}

void EntityNode::setNodeGetter(std::function<simVis::EntityNode* (simData::ObjectId)> getter)
{
  nodeGetter_ = getter;
}

}
