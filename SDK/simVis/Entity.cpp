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
#include "simVis/ElevationQueryProxy.h"
#include "simVis/Utils.h"
#include "simVis/Entity.h"

#define LC "[EntityNode] "

//----------------------------------------------------------------------------
namespace simVis
{

CoordSurfaceClamping::CoordSurfaceClamping(osg::Group* scene)
  : elevationQuery_(new simVis::ElevationQueryProxy(NULL, scene))
{}

CoordSurfaceClamping::~CoordSurfaceClamping()
{
  delete elevationQuery_;
}

void CoordSurfaceClamping::clampCoordToMapSurface(simCore::Coordinate& coord)
{
  // nothing to do if we don't have a valid elevation query
  if (elevationQuery_ == NULL)
  {
    assert(0); // called this method without setting the map
    return;
  }
  if (coord.coordinateSystem() != simCore::COORD_SYS_LLA && coord.coordinateSystem() != simCore::COORD_SYS_ECEF)
  {
    // coordinate type must be LLA to work with osgEarth elevation query
    assert(0);
    return;
  }

  // convert from ECEF to LLA if necessary, since osgEarth elevation query requires LLA
  simCore::Coordinate llaCoord;
  if (coord.coordinateSystem() == simCore::COORD_SYS_ECEF)
    simCore::CoordinateConverter::convertEcefToGeodetic(coord, llaCoord);
  else
    llaCoord = coord;

  osg::ref_ptr<osgEarth::SpatialReference> srs = osgEarth::SpatialReference::create("wgs84");
  osgEarth::GeoPoint lonLatAlt(srs, llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, 0.0, osgEarth::ALTMODE_ABSOLUTE);
  double alt;
  static const double angluarResolution = 0.00001;
  if (elevationQuery_->getElevation(lonLatAlt, alt, angluarResolution))
    llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), alt);
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
  return elevationQuery_ != NULL;
}

void CoordSurfaceClamping::setMap(const osgEarth::Map* map)
{
  elevationQuery_->setMap(map);
}

/////////////////////////////////////////////////////////////////////////////////

EntityNode::EntityNode(simData::DataStore::ObjectType type, Locator* locator)
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
}

