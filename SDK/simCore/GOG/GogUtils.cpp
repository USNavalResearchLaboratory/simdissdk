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
#include "simCore/GOG/ParsedShape.h"
#include "simCore/GOG/GogUtils.h"

namespace simCore { namespace GOG {

UnitsState::UnitsState()
{
}

const simCore::Units& UnitsState::altitudeUnits() const
{
  return altitudeUnits_.value_or(simCore::Units::FEET);
}

void UnitsState::setAltitudeUnits(const simCore::Units& units)
{
  altitudeUnits_ = units;
}

bool UnitsState::hasAltitudeUnits() const
{
  return altitudeUnits_.has_value();
}

const simCore::Units& UnitsState::angleUnits() const
{
  return angleUnits_.value_or(simCore::Units::DEGREES);
}

void UnitsState::setAngleUnits(const simCore::Units& units)
{
  angleUnits_ = units;
}

bool UnitsState::hasAngleUnits() const
{
  return angleUnits_.has_value();
}

const simCore::Units& UnitsState::rangeUnits() const
{
  return rangeUnits_.value_or(simCore::Units::YARDS);
}

void UnitsState::setRangeUnits(const simCore::Units& units)
{
  rangeUnits_ = units;
}

bool UnitsState::hasRangeUnits() const
{
  return rangeUnits_.has_value();
}

void UnitsState::parse(const ParsedShape& parsedShape, const simCore::UnitsRegistry& unitsRegistry)
{
  if (parsedShape.hasValue(ShapeParameter::ANGLEUNITS))
    parse(parsedShape.stringValue(ShapeParameter::ANGLEUNITS), unitsRegistry, angleUnits_);
  if (parsedShape.hasValue(ShapeParameter::ALTITUDEUNITS))
    parse(parsedShape.stringValue(ShapeParameter::ALTITUDEUNITS), unitsRegistry, altitudeUnits_);
  if (parsedShape.hasValue(ShapeParameter::RANGEUNITS))
    parse(parsedShape.stringValue(ShapeParameter::RANGEUNITS), unitsRegistry, rangeUnits_);
}

void UnitsState::parse(const std::string& unitString, const simCore::UnitsRegistry& unitsRegistry, Optional<simCore::Units>& units)
{
  if (unitString == "secs")
    units = simCore::Units::SECONDS;
  else if (unitString == "mins")
    units = simCore::Units::MINUTES;
  else if (unitString == "hrs")
    units = simCore::Units::HOURS;
  else if (unitString == "sm")
    units = simCore::Units::MILES;
  else if (unitString == "degree")
    units = simCore::Units::DEGREES;
  else
  {
    simCore::Units unitsObject;
    if (unitsRegistry.unitsByAbbreviation(unitString, unitsObject) != 0)
      unitsRegistry.unitsByName(unitString, unitsObject);
    units = unitsObject;
  }
}

void ModifierState::apply(ParsedShape& shape)
{
  if (!lineColor_.empty()) shape.set(ShapeParameter::LINECOLOR, lineColor_);
  if (!lineWidth_.empty()) shape.set(ShapeParameter::LINEWIDTH, lineWidth_);
  if (!lineStyle_.empty()) shape.set(ShapeParameter::LINESTYLE, lineStyle_);
  if (!fillColor_.empty()) shape.set(ShapeParameter::FILLCOLOR, fillColor_);
  if (!pointSize_.empty()) shape.set(ShapeParameter::POINTSIZE, pointSize_);
  if (!altitudeMode_.empty()) shape.set(ShapeParameter::ALTITUDEMODE, altitudeMode_);
  if (!altitudeUnits_.empty()) shape.set(ShapeParameter::ALTITUDEUNITS, altitudeUnits_);
  if (!rangeUnits_.empty()) shape.set(ShapeParameter::RANGEUNITS, rangeUnits_);
  if (!angleUnits_.empty()) shape.set(ShapeParameter::ANGLEUNITS, angleUnits_);
  if (!verticalDatum_.empty()) shape.set(ShapeParameter::VERTICALDATUM, verticalDatum_);
  if (!priority_.empty()) shape.set(ShapeParameter::PRIORITY, priority_);
  if (!textOutlineColor_.empty()) shape.set(ShapeParameter::TEXTOUTLINECOLOR, textOutlineColor_);
  if (!textOutlineThickness_.empty()) shape.set(ShapeParameter::TEXTOUTLINETHICKNESS, textOutlineThickness_);
  if (!fontName_.empty()) shape.set(ShapeParameter::FONTNAME, fontName_);
  if (!textSize_.empty()) shape.set(ShapeParameter::TEXTSIZE, textSize_);
}

}}

