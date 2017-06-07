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
#include "osg/Depth"
#include "osgEarth/NodeUtils"
#include "osgEarthAnnotation/GeoPositionNode"
#include "osgEarthAnnotation/LocalGeometryNode"
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Angle.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/String/ValidNumber.h"
#include "simVis/Constants.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/GOG/Parser.h"
#include "simVis/GOG/Utils.h"

using namespace simVis;
using namespace simVis::GOG;
using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Symbology;

namespace {
/// Same default priority as the simData.commonPrefs.labelPrefs.priority value
static const float DEFAULT_LABEL_PRIORITY = 100.f;
}

//------------------------------------------------------------------------

#define LC "[GOG::UnitsState] "

void Utils::applyLocalGeometryOffsets(LocalGeometryNode& node, ParserData& data)
{
  node.setPosition(data.getMapPosition());
  node.setLocalOffset(data.getLTPOffset());
  osg::Quat yaw(data.localHeadingOffset_->as(Units::RADIANS), -osg::Vec3(0, 0, 1));
  osg::Quat pitch(data.localPitchOffset_->as(Units::RADIANS), osg::Vec3(1, 0, 0));
  osg::Quat roll(data.localRollOffset_->as(Units::RADIANS), osg::Vec3(0, 1, 0));
  node.setLocalRotation(roll * pitch * yaw);
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
    break;
  }
  return false;
}

void Utils::getGeometryPoints(const osgEarth::Symbology::Geometry* geometry, std::vector<osg::Vec3d>& points)
{
  if (!geometry)
    return;

  // if geometry is empty could be a MultiGeometry (for linesegs)
  if (geometry->empty())
  {
    const osgEarth::Symbology::MultiGeometry* multiGeometry = dynamic_cast<const osgEarth::Symbology::MultiGeometry*>(geometry);
    if (multiGeometry)
    {
      osgEarth::Symbology::GeometryCollection segs = multiGeometry->getComponents();
      for (osgEarth::Symbology::GeometryCollection::const_iterator iter = segs.begin(); iter != segs.end(); ++iter)
      {
        for (osgEarth::Symbology::Geometry::const_iterator geoIter = (*iter)->begin(); geoIter != (*iter)->end(); ++geoIter)
        {
          points.push_back(*geoIter);
        }
      }
    }
  }
  else
  {
    for (osgEarth::Symbology::Geometry::const_iterator geoIter = geometry->begin(); geoIter != geometry->end(); ++geoIter)
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

void Utils::serializeShapeGeometry(const osgEarth::Symbology::Geometry* geometry, bool relativeShape, std::ostream& gogOutputStream)
{
  if (!geometry)
    return;

  // if geometry is empty could be a MultiGeometry (for linesegs)
  if (geometry->empty())
  {
    const osgEarth::Symbology::MultiGeometry* multiGeometry = dynamic_cast<const osgEarth::Symbology::MultiGeometry*>(geometry);
    if (multiGeometry)
    {
      osgEarth::Symbology::GeometryCollection segs = multiGeometry->getComponents();
      for (osgEarth::Symbology::GeometryCollection::const_iterator iter = segs.begin(); iter != segs.end(); ++iter)
      {
        Utils::serializeGeometry(iter->get(), relativeShape, gogOutputStream);
      }
    }
  }
  else
    Utils::serializeGeometry(geometry, relativeShape, gogOutputStream);

}

void Utils::serializeGeometry(const osgEarth::Symbology::Geometry* geometry, bool relativeShape, std::ostream& gogOutputStream)
{
  // iterate through the geometry and serialize out the position information
  for (osgEarth::Symbology::Geometry::const_iterator iter = geometry->begin(); iter != geometry->end(); ++iter)
  {
    if (relativeShape)
      gogOutputStream << "xyz " << iter->x() << " " << iter->y() << " " << iter->z() << "\n";
    else
    gogOutputStream << "lla " << iter->y() << " " << iter->x() << " " << iter->z() << "\n";
  }
}

std::string Utils::serializeOsgColor(const osg::Vec4f& colorVec)
{
  osgEarth::Symbology::Color color(colorVec);
  std::ostringstream os;
  os << std::hex << std::showbase << color.asRGBA();
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

UnitsState::UnitsState()
{
  //defaults
  altitudeUnits_ = Units::FEET;
  rangeUnits_ = Units::YARDS;
  timeUnits_ = Units::SECONDS;
  angleUnits_ = Units::DEGREES;
}

void UnitsState::parse(const osgEarth::Config& conf)
{
  if (conf.hasValue("angleunits"))
    parse(conf.value("angleunits"), Units::TYPE_ANGULAR, angleUnits_);
  if (conf.hasValue("altitudeunits"))
    parse(conf.value("altitudeunits"), Units::TYPE_LINEAR, altitudeUnits_);
  if (conf.hasValue("rangeunits"))
    parse(conf.value("rangeunits"), Units::TYPE_LINEAR, rangeUnits_);
  if (conf.hasValue("timeunits"))
    parse(conf.value("timeunits"), Units::TYPE_TEMPORAL, timeUnits_);
}

void UnitsState::parse(const std::string& s, osgEarth::Units::Type type, osgEarth::Units& units)
{
  if (type == Units::TYPE_LINEAR)
  {
    if (s == "mm" || s == "millimeters") units = Units::MILLIMETERS;
    else if (s == "cm" || s == "centimeters") units = Units::CENTIMETERS;
    else if (s == "in" || s == "inches") units = Units::INCHES;
    else if (s == "ft" || s == "feet") units = Units::FEET;
    else if (s == "yd" || s == "yards") units = Units::YARDS;
    else if (s == "m"  || s == "meters") units = Units::METERS;
    else if (s == "fm" || s == "fathoms") units = Units::FATHOMS;
    else if (s == "kf" || s == "kilofeet") units = Units::KILOFEET;
    else if (s == "kyd"|| s == "kiloyards") units = Units::KILOYARDS;
    else if (s == "km" || s == "kilometers") units = Units::KILOMETERS;
    else if (s == "sm" || s == "miles") units = Units::MILES;
    else if (s == "nm" || s == "nautical miles") units = Units::NAUTICAL_MILES;
    else if (s == "dm" || s == "data miles") units = Units::DATA_MILES;
  }
  else if (type == Units::TYPE_TEMPORAL)
  {
    if (s == "secs" || s == "seconds") units = Units::SECONDS;
    else if (s == "mins" || s == "minutes") units = Units::MINUTES;
    else if (s == "hrs"  || s == "hours") units = Units::HOURS;
  }
  else if (type == Units::TYPE_ANGULAR)
  {
    if (s == "bam") units = Units::BAM;
    else if (s == "mil" || s == "mils") units = Units::NATO_MILS;
    else if (s == "rad" || s == "radians") units = Units::RADIANS;
    else if (s == "deg" || s == "degrees") units = Units::DEGREES;
  }
}

//------------------------------------------------------------------------

#undef  LC
#define LC "[GOG::ModifierState] "

void ModifierState::apply(Config& conf)
{
  if (lineColor_.isSet()) conf.set("linecolor", *lineColor_);
  if (lineWidth_.isSet()) conf.set("linewidth", *lineWidth_);
  if (lineStyle_.isSet()) conf.set("linestyle", *lineStyle_);
  if (fillColor_.isSet()) conf.set("fillcolor", *fillColor_);
  if (pointSize_.isSet()) conf.set("pointsize", *pointSize_);
  if (altitudeMode_.isSet()) conf.set("altitudemode", *altitudeMode_);
  if (altitudeUnits_.isSet()) conf.set("altitudeunits", *altitudeUnits_);
  if (rangeUnits_.isSet()) conf.set("rangeunits", *rangeUnits_);
  if (timeUnits_.isSet()) conf.set("timeunits", *timeUnits_);
  if (angleUnits_.isSet()) conf.set("angleunits", *angleUnits_);
  if (verticalDatum_.isSet()) conf.set("verticaldatum", *verticalDatum_);
  if (priority_.isSet()) conf.set("priority", *priority_);
}

//------------------------------------------------------------------------

#undef  LC
#define LC "[GOG::ParserData] "

ParserData::ParserData(const Config& conf, const GOGContext& context, GogShape shape)
: context_(context)
{
  init();

  // extract the units modifiers:
  units_.parse(conf);

  // check for a reference position for NED coordinates
  if (conf.hasValue("lat"))
  {
    refPointLLA_->set(
      parseAngle(conf.value("lon"), 0.0),
      parseAngle(conf.value("lat"), 0.0),
      units_.altitudeUnits_.convertTo(Units::METERS, conf.value("alt", 0.0)));
  }

  // The centerLLA and centerXYZ doe not apply to points, lines, line segments and polygons
  if ((shape != GOG_POINTS) && (shape != GOG_POLYGON) && (shape != GOG_LINE) && (shape != GOG_LINESEGS))
  {
    if (conf.hasChild("centerll"))
    {
      const Config& c = conf.child("centerll");
      // units as per the SIMDIS user manual:
      centerLLA_->set(
        parseAngle(c.value("lon"), 0.0),
        parseAngle(c.value("lat"), 0.0),
        units_.altitudeUnits_.convertTo(Units::METERS, c.value("alt", 0.0)));
    }

    if (conf.hasChild("centerxy"))
    {
      const Config& c = conf.child("centerxy");
      // units as per the SIMDIS user manual:
      centerXYZ_->set(
        units_.rangeUnits_.convertTo(Units::METERS, c.value("x", 0.0)),
        units_.rangeUnits_.convertTo(Units::METERS, c.value("y", 0.0)),
        units_.altitudeUnits_.convertTo(Units::METERS, c.value("z", 0.0)));
    }
  }

  if (conf.hasChild("lineprojection"))
  {
    if (simCore::caseCompare(conf.value("lineprojection"), "greatcircle") == 0)
      geoInterp_ = GEOINTERP_GREAT_CIRCLE;
    else if (simCore::caseCompare(conf.value("lineprojection"), "rhumbline") == 0)
      geoInterp_ = GEOINTERP_RHUMB_LINE;
  }

  if (conf.hasChild("verticaldatum"))
  {
    const std::string& vdatum = conf.value("verticaldatum");
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
  parseOffsetsAndTracking(conf);

  // Fill out the priority data on annotations
  if (shape == GOG_ANNOTATION)
  {
    float priority = DEFAULT_LABEL_PRIORITY;
    // Note that this if() statement will assign priority value if isValidNumber succeeds.
    if (conf.hasChild("priority") && !simCore::isValidNumber(conf.value("priority"), priority))
    {
      SIM_WARN << LC << "Invalid priority value \"" << conf.value("priority") << "\", expected numeric value.\n";
    }
    // Negative priority means to always show
    if (priority < 0.0)
      priority = std::numeric_limits<float>::max();
    style_.getOrCreateSymbol<TextSymbol>()->priority() = priority;

    // Print the priority for debugging purposes
    SIM_DEBUG << "GOG Annotation \"" << conf.value<std::string>("text", "<None>") << "\" priority: "
      << (priority == std::numeric_limits<float>::max() ? -1.f : priority) << "\n";
  }

  // name.
  name_ = conf.value("3d name");
  if (name_.empty())
    name_ = simVis::GOG::Parser::getKeywordFromShape(shape);
}

void ParserData::init()
{
  srs_ = SpatialReference::create("wgs84"); // default
  geomIsLLA_ = true;
  locatorComps_ = Locator::COMP_POSITION;
}

void ParserData::parseOffsetsAndTracking(const Config& conf)
{
  if (conf.hasValue("orient"))
  {
    locatorComps_ &= ~Locator::COMP_ORIENTATION; // reset first
    const std::string& value = conf.value("orient");
    if (value.find("c") != std::string::npos) locatorComps_ |= Locator::COMP_HEADING;
    if (value.find("p") != std::string::npos) locatorComps_ |= Locator::COMP_PITCH;
    if (value.find("r") != std::string::npos) locatorComps_ |= Locator::COMP_ROLL;
  }

  if (conf.hasValue("3d follow"))
  {
    locatorComps_ &= ~Locator::COMP_ORIENTATION; // reset first
    const std::string& value = conf.value("3d follow");
    if (value.find("c") != std::string::npos) locatorComps_ |= Locator::COMP_HEADING;
    if (value.find("p") != std::string::npos) locatorComps_ |= Locator::COMP_PITCH;
    if (value.find("r") != std::string::npos) locatorComps_ |= Locator::COMP_ROLL;
  }

  if (conf.hasValue("3d offsetalt"))
  {
    localAltOffset_ = Distance(conf.value<double>("3d offsetalt", 0.0), units_.altitudeUnits_);
  }

  if (conf.hasValue("3d offsetcourse"))
  {
    locatorComps_ |= Locator::COMP_HEADING;
    localHeadingOffset_ = Angle(conf.value<double>("3d offsetcourse", 0.0), units_.angleUnits_);
  }
  if (conf.hasValue("3d offsetpitch"))
  {
    locatorComps_ |= Locator::COMP_PITCH;
    localPitchOffset_ = Angle(conf.value<double>("3d offsetpitch", 0.0), units_.angleUnits_);
  }
  if (conf.hasValue("3d offsetroll"))
  {
    locatorComps_ |= Locator::COMP_ROLL;
    localRollOffset_ = Angle(conf.value<double>("3d offsetroll", 0.0), units_.angleUnits_);
  }

  if (conf.hasValue("heading"))
  {
    localHeadingOffset_ = Angle(conf.value<double>("heading", 0.0), units_.angleUnits_);
  }
  if (conf.hasValue("pitch"))
  {
    localPitchOffset_ = Angle(conf.value<double>("pitch", 0.0), units_.angleUnits_);
  }
  if (conf.hasValue("roll"))
  {
    localRollOffset_ = Angle(conf.value<double>("roll", 0.0), units_.angleUnits_);
  }

  // scale
  scale_->set(
    conf.value<float>("scalex", 1.0f),
    conf.value<float>("scaley", 1.0f),
    conf.value<float>("scalez", 1.0f));
}

/**
* Parses a collection of points into the specified Geometry object, and
* returns a flag indicating whether the data was lat/long/alt (absolute) or
* xyz (relative)
*
* It is expected that all angle strings have already been validated and processed into simple format by the GOG parser
*/
void ParserData::parsePoints(const Config& parent, const UnitsState& us, Geometry* geom, bool& isLLA)
{
  if (parent.hasChild("ll"))
  {
    const ConfigSet lls = parent.children("ll");
    for (ConfigSet::const_iterator i = lls.begin(); i != lls.end(); ++i)
    {
      const Config& lla = *i;
      osg::Vec3d point;
      simCore::getAngleFromDegreeString(lla.value("lon"), false, point.x());
      simCore::getAngleFromDegreeString(lla.value("lat"), false, point.y());
      point.z() = us.altitudeUnits_.convertTo(Units::METERS, lla.value("alt", 0.0));

      // normalize to -180/+180
      point.x() = simCore::angFix180(point.x());

      // Avoid adding the same point twice
      if (geom->empty() || (*geom->rbegin()) != point)
        geom->push_back(point);
    }
    isLLA = true;
  }
  else if (parent.hasChild("xy"))
  {
    const ConfigSet xys = parent.children("xy");
    for (ConfigSet::const_iterator i = xys.begin(); i != xys.end(); ++i)
    {
      const Config& xy = *i;
      osg::Vec3d point(
        us.rangeUnits_.convertTo(Units::METERS, xy.value("x", 0.0)),
        us.rangeUnits_.convertTo(Units::METERS, xy.value("y", 0.0)),
        us.altitudeUnits_.convertTo(Units::METERS, xy.value("z", 0.0)));

      // Avoid adding the same point twice
      if (geom->empty() || (*geom->rbegin()) != point)
        geom->push_back(point);
    }
    isLLA = false;
  }
}

/**
* Parses a collection of points into the specified Geometry object, and
* returns a flag indicating whether the data was lat/long/alt (absolute) or
* xyz (relative).  Line segment points must come in pairs.
*/
void ParserData::parseLineSegmentPoints(const Config& parent, const UnitsState& us, Geometry* geom, bool& isLLA)
{
  if (parent.hasChild("ll"))
  {
    const ConfigSet lls = parent.children("ll");
    for (ConfigSet::const_iterator i = lls.begin(); i != lls.end(); /* two places in the loop*/)
    {
      const Config& lla1 = *i;
      osg::Vec3d point1;
      simCore::getAngleFromDegreeString(lla1.value("lon"), false, point1.x());
      simCore::getAngleFromDegreeString(lla1.value("lat"), false, point1.y());
      point1.z() = us.altitudeUnits_.convertTo(Units::METERS, lla1.value("alt", 0.0));

      // normalize to -180/+180
      point1.x() = simCore::angFix180(point1.x());

      ++i; //<< increment
      if (i != lls.end())
      {
        const Config& lla2 = *i;
        osg::Vec3d point2;
        simCore::getAngleFromDegreeString(lla2.value("lon"), false, point2.x());
        simCore::getAngleFromDegreeString(lla2.value("lat"), false, point2.y());
        point2.z() = us.altitudeUnits_.convertTo(Units::METERS, lla2.value("alt", 0.0));

        // normalize to -180/+180
        point2.x() = simCore::angFix180(point2.x());

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
  else if (parent.hasChild("xy"))
  {
    const ConfigSet xys = parent.children("xy");
    for (ConfigSet::const_iterator i = xys.begin(); i != xys.end(); /* two places in the loop*/)
    {
      const Config& xy1 = *i;
      osg::Vec3d point1(
        us.rangeUnits_.convertTo(Units::METERS, xy1.value("x", 0.0)),
        us.rangeUnits_.convertTo(Units::METERS, xy1.value("y", 0.0)),
        us.altitudeUnits_.convertTo(Units::METERS, xy1.value("z", 0.0)));

      ++i; //<< increment
      if (i != xys.end())
      {
        const Config& xy2 = *i;
        osg::Vec3d point2(
          us.rangeUnits_.convertTo(Units::METERS, xy2.value("x", 0.0)),
          us.rangeUnits_.convertTo(Units::METERS, xy2.value("y", 0.0)),
          us.altitudeUnits_.convertTo(Units::METERS, xy2.value("z", 0.0)));

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

GeoPoint ParserData::getMapPosition() const
{
  if (refPointLLA_.isSet())
  {
    return GeoPoint(srs_.get(), *refPointLLA_, ALTMODE_ABSOLUTE);
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

    if (localAltOffset_.isSet())
    {
      local->setLocalOffset(osg::Vec3d(0, 0, localAltOffset_->as(Units::METERS)));
    }
    // Don't apply the orientation offsets to the local rotation, it will be handled later through the Locator when adding to the parent node
  }

  // name the node after the GOG :)
  node->setName(name_);
}
