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
#include "osgEarth/Terrain"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/Utils.h"
#include "simVis/Entity.h"

#define LC "[EntityNode] "

//----------------------------------------------------------------------------
namespace simVis
{

CoordSurfaceClamping::CoordSurfaceClamping()
{}

CoordSurfaceClamping::~CoordSurfaceClamping()
{
}

void CoordSurfaceClamping::clampCoordToMapSurface(simCore::Coordinate& coord)
{
  // nothing to do if we don't have a valid map node
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

  double hamsl = 0.0; // not used
  double hae = 0.0; // height above ellipsoid, the rough elevation

  if (mapNode_->getTerrain()->getHeight(mapNode_->getMapSRS(), llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, &hamsl, &hae))
    llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), hae);
  else
    llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), 0.0);  // Assume over the ocean and clamp to zero

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
}

/////////////////////////////////////////////////////////////////////////////////

EntityNode::EntityNode(simData::ObjectType type, Locator* locator)
  : type_(type)
{
  setNodeMask(0);  // Draw is off until a valid update is received
  setLocator(locator);
}

EntityNode::~EntityNode()
{
  setLocator(NULL);
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

}

