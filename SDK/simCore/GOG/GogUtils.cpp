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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
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
  // defaults
  altitudeUnits_ = simCore::Units::FEET;
  rangeUnits_ = simCore::Units::YARDS;
  timeUnits_ = simCore::Units::SECONDS;
  angleUnits_ = simCore::Units::DEGREES;
}

void UnitsState::parse(const ParsedShape& parsedShape, const simCore::UnitsRegistry& unitsRegistry)
{
  if (parsedShape.hasValue(ShapeParameter::GOG_ANGLEUNITS))
    parse(parsedShape.stringValue(ShapeParameter::GOG_ANGLEUNITS), unitsRegistry, angleUnits_);
  if (parsedShape.hasValue(ShapeParameter::GOG_ALTITUDEUNITS))
    parse(parsedShape.stringValue(ShapeParameter::GOG_ALTITUDEUNITS), unitsRegistry, altitudeUnits_);
  if (parsedShape.hasValue(ShapeParameter::GOG_RANGEUNITS))
    parse(parsedShape.stringValue(ShapeParameter::GOG_RANGEUNITS), unitsRegistry, rangeUnits_);
  if (parsedShape.hasValue(ShapeParameter::GOG_TIMEUNITS))
    parse(parsedShape.stringValue(ShapeParameter::GOG_TIMEUNITS), unitsRegistry, timeUnits_);
}

void UnitsState::parse(const std::string& s, const simCore::UnitsRegistry& unitsRegistry, simCore::Units& units)
{
  if (s == "secs")
    units = simCore::Units::SECONDS;
  else if (s == "mins")
    units = simCore::Units::MINUTES;
  else if (s == "hrs")
    units = simCore::Units::HOURS;
  else if (s == "sm")
    units = simCore::Units::MILES;
  if (unitsRegistry.unitsByAbbreviation(s, units) == 0)
    return;
  unitsRegistry.unitsByName(s, units);
}

void ModifierState::apply(ParsedShape& shape)
{
  if (!lineColor_.empty()) shape.set(ShapeParameter::GOG_LINECOLOR, lineColor_);
  if (!lineWidth_.empty()) shape.set(ShapeParameter::GOG_LINEWIDTH, lineWidth_);
  if (!lineStyle_.empty()) shape.set(ShapeParameter::GOG_LINESTYLE, lineStyle_);
  if (!fillColor_.empty()) shape.set(ShapeParameter::GOG_FILLCOLOR, fillColor_);
  if (!pointSize_.empty()) shape.set(ShapeParameter::GOG_POINTSIZE, pointSize_);
  if (!altitudeMode_.empty()) shape.set(ShapeParameter::GOG_ALTITUDEMODE, altitudeMode_);
  if (!altitudeUnits_.empty()) shape.set(ShapeParameter::GOG_ALTITUDEUNITS, altitudeUnits_);
  if (!rangeUnits_.empty()) shape.set(ShapeParameter::GOG_RANGEUNITS, rangeUnits_);
  if (!timeUnits_.empty()) shape.set(ShapeParameter::GOG_TIMEUNITS, timeUnits_);
  if (!angleUnits_.empty()) shape.set(ShapeParameter::GOG_ANGLEUNITS, angleUnits_);
  if (!verticalDatum_.empty()) shape.set(ShapeParameter::GOG_VERTICALDATUM, verticalDatum_);
  if (!priority_.empty()) shape.set(ShapeParameter::GOG_PRIORITY, priority_);
  if (!textOutlineColor_.empty()) shape.set(ShapeParameter::GOG_TEXTOUTLINECOLOR, textOutlineColor_);
  if (!textOutlineThickness_.empty()) shape.set(ShapeParameter::GOG_TEXTOUTLINETHICKNESS, textOutlineThickness_);
  if (!fontName_.empty()) shape.set(ShapeParameter::GOG_FONTNAME, fontName_);
  if (!textSize_.empty()) shape.set(ShapeParameter::GOG_TEXTSIZE, textSize_);
}

}}

