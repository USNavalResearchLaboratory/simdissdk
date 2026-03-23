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
#include <limits>
#include "osgDB/ReadFile"
#include "osgEarth/NodeUtils"
#include "osgEarth/GeoPositionNode"
#include "osgEarth/LocalGeometryNode"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/GOG/GogUtils.h"
#include "simCore/GOG/Parser.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/Registry.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth;

namespace {
/// Same default priority as the simData.commonPrefs.labelPrefs.priority value
static const float DEFAULT_LABEL_PRIORITY = -1.f;
}

//------------------------------------------------------------------------

#undef LC
#define LC "[GOG::UnitsState] "

namespace simVis { namespace GOG {

void Utils::getGeometryPoints(const osgEarth::Geometry* geometry, std::vector<osg::Vec3d>& points)
{
  if (!geometry)
    return;

  // if geometry is empty could be a MultiGeometry (for linesegs)
  if (geometry->empty())
  {
    const osgEarth::MultiGeometry* multiGeometry = dynamic_cast<const osgEarth::MultiGeometry*>(geometry);
    if (multiGeometry)
    {
      osgEarth::GeometryCollection segs = multiGeometry->getComponents();
      for (osgEarth::GeometryCollection::const_iterator iter = segs.begin(); iter != segs.end(); ++iter)
      {
        for (osgEarth::Geometry::const_iterator geoIter = (*iter)->begin(); geoIter != (*iter)->end(); ++geoIter)
        {
          points.push_back(*geoIter);
        }
      }
    }
  }
  else
  {
    for (osgEarth::Geometry::const_iterator geoIter = geometry->begin(); geoIter != geometry->end(); ++geoIter)
    {
      points.push_back(*geoIter);
    }
  }
}

Utils::LineStyle Utils::getLineStyleFromStipple(unsigned short stipple)
{
  // special case, no stipple same as solid
  if (stipple == 0x0 || stipple == simVis::GOG::GogSolidStipple)
    return LINE_SOLID;
  // call anything between dotted-solid dashed
  else if (stipple >= simVis::GOG::GogDashStipple)
    return LINE_DASHED;

  return LINE_DOTTED;
}

unsigned short Utils::getStippleFromLineStyle(LineStyle lineStyle)
{
  switch (lineStyle)
  {
  case LINE_DASHED:
    return simVis::GOG::GogDashStipple;
  case LINE_DOTTED:
    return simVis::GOG::GogDotStipple;
  case LINE_SOLID:
    return simVis::GOG::GogSolidStipple;
  }
  return simVis::GOG::GogSolidStipple;
}

void Utils::serializeShapeGeometry(const osgEarth::Geometry* geometry, bool relativeShape, std::ostream& gogOutputStream)
{
  if (!geometry)
    return;

  // if geometry is empty could be a MultiGeometry (for linesegs)
  if (geometry->empty())
  {
    const osgEarth::MultiGeometry* multiGeometry = dynamic_cast<const osgEarth::MultiGeometry*>(geometry);
    if (multiGeometry)
    {
      osgEarth::GeometryCollection segs = multiGeometry->getComponents();
      for (osgEarth::GeometryCollection::const_iterator iter = segs.begin(); iter != segs.end(); ++iter)
      {
        Utils::serializeGeometry(iter->get(), relativeShape, gogOutputStream);
      }
    }
  }
  else
    Utils::serializeGeometry(geometry, relativeShape, gogOutputStream);

}

void Utils::serializeGeometry(const osgEarth::Geometry* geometry, bool relativeShape, std::ostream& gogOutputStream)
{
  // iterate through the geometry and serialize out the position information
  for (osgEarth::Geometry::const_iterator iter = geometry->begin(); iter != geometry->end(); ++iter)
  {
    if (relativeShape)
      gogOutputStream << "xyz " << iter->x() << " " << iter->y() << " " << iter->z() << "\n";
    else
    gogOutputStream << "lla " << iter->y() << " " << iter->x() << " " << iter->z() << "\n";
  }
}

std::string Utils::serializeOsgColor(const osg::Vec4f& colorVec)
{
  simVis::Color color(colorVec);
  std::ostringstream os;
  os << "0x" << std::hex << std::setfill('0') << std::setw(8) << color.asRGBA();
  return os.str();
}

std::string Utils::serializeLineStyle(LineStyle lineStyle)
{
  switch (lineStyle)
  {
  case LINE_SOLID:
    return "solid";
  case LINE_DASHED:
    return "dash";
  case LINE_DOTTED:
    return "dot";
  }
  // invalid line style passed in
  assert(false);
  return "solid";
}


bool Utils::isGeometry2D(const Geometry* geom)
{
  // we're checking to see if this geometry is "2D", i.e. it has no 2D info and it either
  // going to be flat on the ground or flat at Z=0.

  // Next check for non-zero Z values in the geometry.
  if (!geom)
    return false;

  ConstGeometryIterator iter(geom, true);
  while (iter.hasMore())
  {
    const Geometry* part = iter.next();
    for (Geometry::const_iterator p = part->begin(); p != part->end(); ++p)
    {
      if (p->z() != 0.0)
        return false;
    }
  }

  // It's 2D.
  return true;
}

void Utils::configureStyleForClipping(Style& style)
{
  // disable depth testing, prioritize the draw order, and activate the
  // visible-horizon clip plane.
  style.getOrCreate<RenderSymbol>()->depthTest() = false;
  style.getOrCreate<RenderSymbol>()->order()->setLiteral(BIN_GOG_FLAT);
  style.getOrCreate<RenderSymbol>()->renderBin() = BIN_GLOBAL_SIMSDK;
  style.getOrCreate<RenderSymbol>()->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
}

osg::ref_ptr<osg::Image> Utils::readRefImage(const std::string& addr)
{
  return osgDB::readRefImageFile(simCore::GOG::GogUtils::processUrl(addr));
}

std::string Utils::getKeywordFromShape(GogShape shape)
{
  switch (shape)
  {
  case GOG_ANNOTATION:
    return "annotation";
  case GOG_CIRCLE:
    return "circle";
  case GOG_ELLIPSE:
    return "ellipse";
  case GOG_ELLIPSOID:
    return "ellipsoid";
  case GOG_ARC:
    return "arc";
  case GOG_CYLINDER:
    return "cylinder";
  case GOG_HEMISPHERE:
    return "hemisphere";
  case GOG_SPHERE:
    return "sphere";
  case GOG_POINTS:
    return "points";
  case GOG_LINE:
    return "line";
  case GOG_POLYGON:
    return "polygon";
  case GOG_LINESEGS:
    return "linesegs";
  case GOG_LATLONALTBOX:
    return "latlonaltbox";
  case GOG_CONE:
    return "cone";
  case GOG_IMAGEOVERLAY:
    return "imageoverlay";
  case GOG_ORBIT:
    return "orbit";
  case GOG_UNKNOWN:
    return "";
  }
  return "";
}

} }
