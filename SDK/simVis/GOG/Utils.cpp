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
#include <limits>
#include "osgEarth/NodeUtils"
#include "osgEarth/GeoPositionNode"
#include "osgEarth/LocalGeometryNode"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Parser.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth;

namespace {
/// Same default priority as the simData.commonPrefs.labelPrefs.priority value
static const float DEFAULT_LABEL_PRIORITY = 100.f;
}

//------------------------------------------------------------------------

#define LC "[GOG::UnitsState] "

namespace simVis { namespace GOG {

void Utils::applyLocalGeometryOffsets(LocalGeometryNode& node, ParserData& data, GOGNodeType nodeType, bool ignoreOffset)
{
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // if this is a geographic node, set position and local rotation directly on the node; note that un-attached relative GOGs are treated as geographic
    node.setPosition(data.getMapPosition(ignoreOffset));
    osg::Quat yaw(data.localHeadingOffset_->as(Units::RADIANS), -osg::Vec3(0, 0, 1));
    osg::Quat pitch(data.localPitchOffset_->as(Units::RADIANS), osg::Vec3(1, 0, 0));
    osg::Quat roll(data.localRollOffset_->as(Units::RADIANS), osg::Vec3(0, 1, 0));
    node.setLocalRotation(roll * pitch * yaw);
  }
  else
  {
    // if this is a hosted node, it will need to set any offsets in the attitude transform's position, since it's position is ignored
    osg::PositionAttitudeTransform* trans = node.getPositionAttitudeTransform();
    if (trans != NULL)
      trans->setPosition(data.getLTPOffset());
    // hosted nodes don't set orientation offsets directly, they are instead applied through a Locator attached to the host
  }
}


bool Utils::canSerializeGeometry_(simVis::GOG::GogShape shape)
{
  switch (shape)
  {
  case simVis::GOG::GOG_LINE:
  case simVis::GOG::GOG_LINESEGS:
  case simVis::GOG::GOG_POLYGON:
  case simVis::GOG::GOG_POINTS:
    return true;
  case simVis::GOG::GOG_ARC:
  case simVis::GOG::GOG_CIRCLE:
  case simVis::GOG::GOG_ELLIPSE:
  case simVis::GOG::GOG_ELLIPSOID:
  case simVis::GOG::GOG_CYLINDER:
  case simVis::GOG::GOG_SPHERE:
  case simVis::GOG::GOG_HEMISPHERE:
  case simVis::GOG::GOG_UNKNOWN:
  case simVis::GOG::GOG_ANNOTATION:
  case simVis::GOG::GOG_LATLONALTBOX:
  case simVis::GOG::GOG_CONE:
    break;
  }
  return false;
}

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

std::string Utils::decodeAnnotation(const std::string& anno)
{
  const std::string r1 = simCore::StringUtils::substitute(anno, "_", " ");
  return simCore::StringUtils::substitute(r1, "\\n", "\n");
}

UnitsState::UnitsState()
{
  //defaults
  altitudeUnits_ = simCore::Units::FEET;
  rangeUnits_ = simCore::Units::YARDS;
  timeUnits_ = simCore::Units::SECONDS;
  angleUnits_ = simCore::Units::DEGREES;
}

void UnitsState::parse(const ParsedShape& parsedShape, const simCore::UnitsRegistry& unitsRegistry)
{
  if (parsedShape.hasValue(GOG_ANGLEUNITS))
    parse(parsedShape.stringValue(GOG_ANGLEUNITS), unitsRegistry, angleUnits_);
  if (parsedShape.hasValue(GOG_ALTITUDEUNITS))
    parse(parsedShape.stringValue(GOG_ALTITUDEUNITS), unitsRegistry, altitudeUnits_);
  if (parsedShape.hasValue(GOG_RANGEUNITS))
    parse(parsedShape.stringValue(GOG_RANGEUNITS), unitsRegistry, rangeUnits_);
  if (parsedShape.hasValue(GOG_TIMEUNITS))
    parse(parsedShape.stringValue(GOG_TIMEUNITS), unitsRegistry, timeUnits_);
}

void UnitsState::parse(const std::string& s, const simCore::UnitsRegistry& unitsRegistry, simCore::Units& units)
{
  if (s == "secs") units = simCore::Units::SECONDS;
  else if (s == "mins") units = simCore::Units::MINUTES;
  else if (s == "hrs") units = simCore::Units::HOURS;
  else if (s == "sm") units = simCore::Units::MILES;
  if (unitsRegistry.unitsByAbbreviation(s, units) == 0)
    return;
  unitsRegistry.unitsByName(s, units);
}

//------------------------------------------------------------------------

#undef  LC
#define LC "[GOG::ModifierState] "

void ModifierState::apply(ParsedShape& shape)
{
  if (lineColor_.isSet()) shape.set(GOG_LINECOLOR, *lineColor_);
  if (lineWidth_.isSet()) shape.set(GOG_LINEWIDTH, *lineWidth_);
  if (lineStyle_.isSet()) shape.set(GOG_LINESTYLE, *lineStyle_);
  if (fillColor_.isSet()) shape.set(GOG_FILLCOLOR, *fillColor_);
  if (pointSize_.isSet()) shape.set(GOG_POINTSIZE, *pointSize_);
  if (altitudeMode_.isSet()) shape.set(GOG_ALTITUDEMODE, *altitudeMode_);
  if (altitudeUnits_.isSet()) shape.set(GOG_ALTITUDEUNITS, *altitudeUnits_);
  if (rangeUnits_.isSet()) shape.set(GOG_RANGEUNITS, *rangeUnits_);
  if (timeUnits_.isSet()) shape.set(GOG_TIMEUNITS, *timeUnits_);
  if (angleUnits_.isSet()) shape.set(GOG_ANGLEUNITS, *angleUnits_);
  if (verticalDatum_.isSet()) shape.set(GOG_VERTICALDATUM, *verticalDatum_);
  if (priority_.isSet()) shape.set(GOG_PRIORITY, *priority_);
  if (textOutlineColor_.isSet()) shape.set(GOG_TEXTOUTLINECOLOR, *textOutlineColor_);
  if (textOutlineThickness_.isSet()) shape.set(GOG_TEXTOUTLINETHICKNESS, *textOutlineThickness_);
}

//------------------------------------------------------------------------

#undef  LC
#define LC "[GOG::ParserData] "

ParserData::ParserData(const ParsedShape& parsedShape, const GOGContext& context, GogShape shape)
: context_(context)
{
  init();

  // extract the units modifiers:
  if (context.unitsRegistry_)
    units_.parse(parsedShape, *context.unitsRegistry_);
  else
  {
    simCore::UnitsRegistry registry;
    registry.registerDefaultUnits();
    units_.parse(parsedShape, registry);
  }

  // check for a reference position for NED coordinates
  if (parsedShape.hasValue(GOG_REF_LAT))
  {
    refPointLLA_->set(
      parseAngle(parsedShape.stringValue(GOG_REF_LON), 0.0),
      parseAngle(parsedShape.stringValue(GOG_REF_LAT), 0.0),
      units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_REF_ALT, 0.0)));
  }

  // The centerLLA and centerXYZ doe not apply to points, lines, line segments and polygons
  if ((shape != GOG_POINTS) && (shape != GOG_POLYGON) && (shape != GOG_LINE) && (shape != GOG_LINESEGS))
  {
    if (parsedShape.hasValue(GOG_CENTERLL))
    {
      const PositionStrings& p = parsedShape.positionValue(GOG_CENTERLL);
      // Convert altitude value from string
      double altitude = 0.;
      simCore::isValidNumber(p.z, altitude);
      // units as per the SIMDIS user manual:
      centerLLA_->set(
        parseAngle(p.y, 0.0),  // longitude
        parseAngle(p.x, 0.0),  // latitude
        units_.altitudeUnits_.convertTo(simCore::Units::METERS, altitude));
    }

    if (parsedShape.hasValue(GOG_CENTERXY))
    {
      const PositionStrings& p = parsedShape.positionValue(GOG_CENTERXY);
      // Convert Z value from string
      double xyz[3] = {0.};
      simCore::isValidNumber(p.x, xyz[0]);
      simCore::isValidNumber(p.y, xyz[1]);
      simCore::isValidNumber(p.z, xyz[2]);
      // units as per the SIMDIS user manual:
      centerXYZ_->set(
        units_.rangeUnits_.convertTo(simCore::Units::METERS, xyz[0]),
        units_.rangeUnits_.convertTo(simCore::Units::METERS, xyz[1]),
        units_.altitudeUnits_.convertTo(simCore::Units::METERS, xyz[2]));
      // if this is a relative GOG with no reference point defined, use the default reference point
      if (!refPointLLA_.isSet())
        refPointLLA_->set(context_.refPoint_->vec3d());
    }
  }

  if (parsedShape.hasValue(GOG_LINEPROJECTION))
  {
    if (simCore::caseCompare(parsedShape.stringValue(GOG_LINEPROJECTION), "greatcircle") == 0)
      geoInterp_ = GEOINTERP_GREAT_CIRCLE;
    else if (simCore::caseCompare(parsedShape.stringValue(GOG_LINEPROJECTION), "rhumbline") == 0)
      geoInterp_ = GEOINTERP_RHUMB_LINE;
  }

  if (parsedShape.hasValue(GOG_VERTICALDATUM))
  {
    const std::string& vdatum = parsedShape.stringValue(GOG_VERTICALDATUM);
    if (simCore::caseCompare(vdatum, "egm1984") == 0 || simCore::caseCompare(vdatum, "egm84") == 0)
      srs_ = SpatialReference::create("wgs84", "egm84");
    else if (simCore::caseCompare(vdatum, "egm1996") == 0 || simCore::caseCompare(vdatum, "egm96") == 0)
      srs_ = SpatialReference::create("wgs84", "egm96");
    else if (simCore::caseCompare(vdatum, "egm2008") == 0 || simCore::caseCompare(vdatum, "egm08") == 0)
      srs_ = SpatialReference::create("wgs84", "egm2008");
    else if (simCore::caseCompare(vdatum, "wgs84") == 0)
      srs_ = SpatialReference::create("wgs84");

    if (!srs_.valid())
    {
      SIM_WARN << LC << "Failed to load vertical datum \"" << vdatum << "\"" << std::endl;
      srs_ = SpatialReference::create("wgs84");
    }
  }

  // parse any locator components (for GOG attachments):
  parseOffsetsAndTracking(parsedShape);

  // Fill out the priority data on annotations
  if (shape == GOG_ANNOTATION)
  {
    float priority = DEFAULT_LABEL_PRIORITY;
    // Note that this if() statement will assign priority value if isValidNumber succeeds.
    if (parsedShape.hasValue(GOG_PRIORITY) && !simCore::isValidNumber(parsedShape.stringValue(GOG_PRIORITY), priority))
    {
      SIM_WARN << LC << "Invalid priority value \"" << parsedShape.stringValue(GOG_PRIORITY) << "\", expected numeric value.\n";
    }
    auto ts = style_.getOrCreateSymbol<TextSymbol>();
    // Negative priority means to always show
    if (priority < 0.0)
      priority = std::numeric_limits<float>::max();
    ts->priority() = priority;

    if (parsedShape.hasValue(GOG_TEXTOUTLINECOLOR))
      ts->halo()->color() = osgEarth::Color(parsedShape.stringValue(GOG_TEXTOUTLINECOLOR));
    else
      ts->halo()->color() = osgEarth::Color::Black;

    // Print the priority for debugging purposes
    SIM_DEBUG << "GOG Annotation \"" << parsedShape.stringValue(GOG_TEXT, "<None>") << "\" priority: "
      << (priority == std::numeric_limits<float>::max() ? -1.f : priority) << "\n";
  }

  // name.
  name_ = parsedShape.stringValue(GOG_3D_NAME);
  if (name_.empty())
    name_ = simVis::GOG::Parser::getKeywordFromShape(shape);
}

void ParserData::init()
{
  srs_ = SpatialReference::create("wgs84"); // default
  geomIsLLA_ = true;
  locatorComps_ = Locator::COMP_POSITION;
}

void ParserData::parseOffsetsAndTracking(const ParsedShape& parsedShape)
{
  if (parsedShape.hasValue(GOG_ORIENT))
  {
    locatorComps_ &= ~Locator::COMP_ORIENTATION; // reset first
    const std::string& value = parsedShape.stringValue(GOG_ORIENT);
    if (value.find("c") != std::string::npos) locatorComps_ |= Locator::COMP_HEADING;
    if (value.find("p") != std::string::npos) locatorComps_ |= Locator::COMP_PITCH;
    if (value.find("r") != std::string::npos) locatorComps_ |= Locator::COMP_ROLL;
  }

  if (parsedShape.hasValue(GOG_3D_FOLLOW))
  {
    locatorComps_ &= ~Locator::COMP_ORIENTATION; // reset first
    const std::string& value = parsedShape.stringValue(GOG_3D_FOLLOW);
    if (value.find("c") != std::string::npos) locatorComps_ |= Locator::COMP_HEADING;
    if (value.find("p") != std::string::npos) locatorComps_ |= Locator::COMP_PITCH;
    if (value.find("r") != std::string::npos) locatorComps_ |= Locator::COMP_ROLL;
  }

  if (parsedShape.hasValue(GOG_3D_OFFSETCOURSE))
  {
    locatorComps_ |= Locator::COMP_HEADING;
    localHeadingOffset_ = Angle(units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_3D_OFFSETCOURSE, 0.0)), Units::DEGREES);
  }
  if (parsedShape.hasValue(GOG_3D_OFFSETPITCH))
  {
    locatorComps_ |= Locator::COMP_PITCH;
    localPitchOffset_ = Angle(units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_3D_OFFSETPITCH, 0.0)), Units::DEGREES);
  }
  if (parsedShape.hasValue(GOG_3D_OFFSETROLL))
  {
    locatorComps_ |= Locator::COMP_ROLL;
    localRollOffset_ = Angle(units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_3D_OFFSETROLL, 0.0)), Units::DEGREES);
  }

  if (parsedShape.hasValue(GOG_ORIENT_HEADING))
  {
    localHeadingOffset_ = Angle(units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ORIENT_HEADING, 0.0)), Units::DEGREES);
  }
  if (parsedShape.hasValue(GOG_ORIENT_PITCH))
  {
    localPitchOffset_ = Angle(units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ORIENT_PITCH, 0.0)), Units::DEGREES);
  }
  if (parsedShape.hasValue(GOG_ORIENT_ROLL))
  {
    localRollOffset_ = Angle(units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ORIENT_ROLL, 0.0)), Units::DEGREES);
  }

  // scale
  scale_->set(
    parsedShape.doubleValue(GOG_SCALEX, 1.0f),
    parsedShape.doubleValue(GOG_SCALEY, 1.0f),
    parsedShape.doubleValue(GOG_SCALEZ, 1.0f));
}

void ParserData::parsePoints(const ParsedShape& parent, const UnitsState& us, Geometry* geom, bool& isLLA)
{
  if (parent.pointType() == ParsedShape::LLA)
  {
    const auto& lls = parent.positions();
    for (auto i = lls.begin(); i != lls.end(); ++i)
    {
      const PositionStrings& lla = *i;
      const osg::Vec3d& point = llaPositionToVec(us, lla);

      // Avoid adding the same point twice
      if (geom->empty() || (*geom->rbegin()) != point)
        geom->push_back(point);
    }
    isLLA = true;
  }
  else if (parent.pointType() == ParsedShape::XYZ)
  {
    const auto& xys = parent.positions();
    for (auto i = xys.begin(); i != xys.end(); ++i)
    {
      const PositionStrings& xy = *i;
      const osg::Vec3d& point = xyzPositionToVec(us, xy);

      // Avoid adding the same point twice
      if (geom->empty() || (*geom->rbegin()) != point)
        geom->push_back(point);
    }
    isLLA = false;
  }
}

osg::Vec3d ParserData::llaPositionToVec(const UnitsState& us, const PositionStrings& posStrings) const
{
  osg::Vec3d point;
  simCore::getAngleFromDegreeString(posStrings.y, false, point.x()); // longitude
  simCore::getAngleFromDegreeString(posStrings.x, false, point.y()); // latitude
  simCore::isValidNumber(posStrings.z, point.z());
  point.z() = us.altitudeUnits_.convertTo(simCore::Units::METERS, point.z());

  // normalize to -180/+180
  point.x() = simCore::angFix180(point.x());
  return point;
}

osg::Vec3d ParserData::xyzPositionToVec(const UnitsState& us, const PositionStrings& posStrings) const
{
  osg::Vec3d point;
  simCore::isValidNumber(posStrings.x, point.x());
  simCore::isValidNumber(posStrings.y, point.y());
  simCore::isValidNumber(posStrings.z, point.z());
  // Convert into proper units
  point.set(us.rangeUnits_.convertTo(simCore::Units::METERS, point.x()),
    us.rangeUnits_.convertTo(simCore::Units::METERS, point.y()),
    us.altitudeUnits_.convertTo(simCore::Units::METERS, point.z()));
  return point;
}

void ParserData::parseLineSegmentPoints(const ParsedShape& parent, const UnitsState& us, Geometry* geom, bool& isLLA)
{
  if (parent.pointType() == ParsedShape::LLA)
  {
    const auto& lls = parent.positions();
    for (auto i = lls.begin(); i != lls.end(); /* two places in the loop*/)
    {
      const PositionStrings& lla1 = *i;
      const osg::Vec3d& point1 = llaPositionToVec(us, lla1);

      ++i; //<< increment
      if (i != lls.end())
      {
        const PositionStrings& lla2 = *i;
        const osg::Vec3d& point2 = llaPositionToVec(us, lla2);

        // Avoid adding the same point twice
        if (point1 != point2)
        {
          geom->push_back(point1);
          geom->push_back(point2);
        }
        ++i; //<< increment
      }
    }
    isLLA = true;
  }
  else if (parent.pointType() == ParsedShape::XYZ)
  {
    const auto& xys = parent.positions();
    for (auto i = xys.begin(); i != xys.end(); /* two places in the loop*/)
    {
      const PositionStrings& xy1 = *i;
      const osg::Vec3d& point1 = xyzPositionToVec(us, xy1);

      ++i; //<< increment
      if (i != xys.end())
      {
        const PositionStrings& xy2 = *i;
        const osg::Vec3d& point2 = xyzPositionToVec(us, xy2);

        // Avoid adding the same point twice
        if (point1 != point2)
        {
          geom->push_back(point1);
          geom->push_back(point2);
        }
        ++i; //<< increment
      }
    }
    isLLA = false;
  }
}

// copied from osgEarth LatLongFormatter
double ParserData::parseAngle(const std::string& input, double fallback)
{
  if (input.empty())
    return fallback;

  const char* c = input.c_str();
  double d=0.0, m=0.0, s=0.0;

  if (sscanf(c, "%lf:%lf:%lf",     &d, &m, &s) == 3 ||
      sscanf(c, "%lf\xb0%lf'%lf\"",   &d, &m, &s) == 3 ||
      sscanf(c, "%lf\xb0 %lf' %lf\"", &d, &m, &s) == 3 ||
      sscanf(c, "%lfd %lf' %lf\"", &d, &m, &s) == 3 ||
      sscanf(c, "%lfd %lfm %lfs",  &d, &m, &s) == 3 ||
      sscanf(c, "%lf %lf' %lf\"",  &d, &m, &s) == 3)
  {
    return simCore::sign(d) * (fabs(d) + m/60.0 + s/3600.0);
  }
  else if (
      sscanf(c, "%lf:%lf",   &d, &m) == 2 ||
      sscanf(c, "%lf\xb0 %lf'", &d, &m) == 2 ||
      sscanf(c, "%lf\xb0%lf'",  &d, &m) == 2 ||
      sscanf(c, "%lfd %lf'", &d, &m) == 2 ||
      sscanf(c, "%lfd %lfm", &d, &m) == 2 ||
      sscanf(c, "%lfd%lf'",  &d, &m) == 2 ||
      sscanf(c, "%lf %lf'",  &d, &m) == 2)
  {
    return simCore::sign(d) * (fabs(d) + m/60.0);
  }
  else if (
      sscanf(c, "%lf\xb0", &d) == 1 ||
      sscanf(c, "%lfd", &d) == 1 ||
      sscanf(c, "%lf",  &d) == 1)
  {
    return d;
  }

  return fallback;
}

bool ParserData::hasMapPosition() const
{
  if (refPointLLA_.isSet()) return true;
  if (centerLLA_.isSet()) return true;
  if (context_.refPoint_.isSet()) return true;
  if (geom_.valid()) return geomIsLLA_;
  return false;
}

GeoPoint ParserData::getMapPosition(bool ignoreOffset) const
{
  if (refPointLLA_.isSet())
  {
    osg::Vec3d xyz = getLTPOffset();
    if (ignoreOffset || (xyz.x() == 0 && xyz.y() == 0 && xyz.z() == 0))
      return GeoPoint(srs_.get(), *refPointLLA_, ALTMODE_ABSOLUTE);

    // apply any xyz offset to the map position ref point if there is one
    simCore::CoordinateConverter cc;
    cc.setReferenceOrigin(refPointLLA_->y() * simCore::DEG2RAD, refPointLLA_->x() * simCore::DEG2RAD, refPointLLA_->z());
    simCore::Coordinate coord(simCore::COORD_SYS_ENU, simCore::Vec3(xyz.x(), xyz.y(), xyz.z()));
    simCore::Coordinate outCoord;
    cc.convert(coord, outCoord, simCore::COORD_SYS_LLA);

    xyz.y() = outCoord.lat() * simCore::RAD2DEG;
    xyz.x() = outCoord.lon() * simCore::RAD2DEG;
    xyz.z() = outCoord.alt();
    return  GeoPoint(srs_.get(), xyz, ALTMODE_ABSOLUTE);
  }
  else if (centerLLA_.isSet())
  {
    return GeoPoint(srs_.get(), *centerLLA_, ALTMODE_ABSOLUTE);
  }
  else if (geom_.valid() && geomIsLLA_)
  {
    return GeoPoint(srs_.get(), geom_->getBounds().center(), ALTMODE_ABSOLUTE);
  }
  else if (context_.refPoint_.isSet())
  {
    return context_.refPoint_.get();
  }
  else
  {
    return GeoPoint::INVALID;
  }
}

osg::Vec3d ParserData::getLTPOffset() const
{
  if (centerXYZ_.isSet())
  {
    return *centerXYZ_;
  }
  else if (hasMapPosition() && hasRelativeGeometry() && geom_->size() == 1)
  {
    return (*geom_)[0];
  }
  else
  {
    return osg::Vec3d(0, 0, 0);
  }
}

simCore::Coordinate ParserData::getCoordinate(MapNode* mapNode, Style& style) const
{
  simCore::Coordinate result;

  if (hasMapPosition())
  {
    GeoPoint pos = getMapPosition();
    if (pos.z() != 0.0)
    {
      style.getOrCreate<AltitudeSymbol>()->verticalOffset() = pos.z();
      pos.z() = 0.0;
    }
    simVis::convertGeoPointToCoord(pos, result, mapNode);
  }
  else
  {
    result = simVis::convertOSGtoSimCoord(
      getLTPOffset(),
      simCore::COORD_SYS_XEAST);
  }

  return result;
}

bool ParserData::hasAbsoluteGeometry() const
{
  return geom_.valid() && geomIsLLA_;
}

bool ParserData::hasRelativeGeometry() const
{
  return geom_.valid() && geom_->size() > 0 && !geomIsLLA_;
}

bool ParserData::geometryRequiresClipping() const
{
  // check all the conditions under which geometry would render on the ground
  // and potentially cause z-fighting.

  // non-relative terrain clamping: yes.
  if (style_.has<AltitudeSymbol>() &&
      style_.get<AltitudeSymbol>()->clamping().isSetTo(AltitudeSymbol::CLAMP_TO_TERRAIN))
  {
    return true;
  }

  // 3D geometry? Never clip. But in the case of 2D geometry, we need to check whether
  // it is absolute or relative.
  bool geomIs2D = Utils::isGeometry2D(geom_.get());
  if (geomIs2D == false)
  {
    return false;
  }

  // 2D and absolute? yes, clip.
  if (hasAbsoluteGeometry())
  {
    return true;
  }

  // 2D, relative to a map position, and Z=0 on the map position? Clip.
  if (hasMapPosition() && getMapPosition().alt() == 0.0 && getLTPOffset().z() == 0.0)
  {
    return true;
  }

  // Out of things to check. No clip.
  return false;
}

void ParserData::applyToAnnotationNode(osg::Node* annoGraph)
{
  AnnotationNode* node = osgEarth::findTopMostNodeOfType<AnnotationNode>(annoGraph);
  if (!node)
    return;

  if (dynamic_cast<GeoPositionNode*>(node))
  {
    GeoPositionNode* local = static_cast<GeoPositionNode*>(node);

    if (scale_.isSet())
      local->setScale(scale_.get());

    // Don't apply the orientation offsets to the local rotation, it will be handled later through the Locator when adding to the parent node
  }

  // name the node after the GOG :)
  node->setName(name_);
}

} }
