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
#ifndef SIMCORE_GOG_GOGUTILS_H
#define SIMCORE_GOG_GOGUTILS_H

#include <string>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Units.h"

namespace simCore {

class UnitsRegistry;

namespace GOG
{

class ParsedShape;

/**
 * Current state of default units. This structure communicates to parsing elements
 * what Units are in effect when parsing coordinate and measurement data.
 * (utility structure - no export)
 */
struct SDKCORE_EXPORT UnitsState
{
  simCore::Units altitudeUnits_; ///< Altitude units
  simCore::Units rangeUnits_; ///< Range units
  simCore::Units timeUnits_; ///< Time units
  simCore::Units angleUnits_; ///< Angle units

  /**
   * Construct the units state
   */
  UnitsState();

  /**
   * Initialize the units state from a structured representation.
   * @param parsedShape structured data input
   * @param unitsRegistry supplies to-string conversion for units
   */
  void parse(const ParsedShape& parsedShape, const simCore::UnitsRegistry& unitsRegistry);

  /**
   * Initialize the units state from a GOG string.
   * @param unitString input string to parse
   * @param unitsRegistry supplies to-string conversion for units
   * @param units parsed output
   */
  void parse(const std::string& unitString, const simCore::UnitsRegistry& unitsRegistry, simCore::Units& units);
};


/**
 * "State" modifiers that "spill over" across GOG objects in the GOG file.
 *
 * In a GOG, certain state elements becomes active until they change, even
 * across different GOG objects. So we have to track the current state using
 * this object.
 */
struct SDKCORE_EXPORT ModifierState
{
  std::string lineColor_; ///< Line color
  std::string lineWidth_; ///< Line width
  std::string lineStyle_; ///< Line style
  std::string fillColor_; ///< File color
  std::string pointSize_; ///< Point size
  std::string altitudeMode_; ///< Altitude mode
  std::string altitudeUnits_; ///< Altitude units
  std::string rangeUnits_; ///< Range units
  std::string timeUnits_; ///< Time units
  std::string angleUnits_; ///< Angle units
  std::string verticalDatum_; ///< Vertical datum
  std::string priority_; ///< Label Priority
  std::string textOutlineColor_; ///< Text outline color
  std::string textOutlineThickness_; ///< Text outline thickness
  std::string fontName_; ///< Font filename
  std::string textSize_; ///< Text point size

  /**
   * Stores the modifier state in a structured object.
   * @param shape object in which to store the state
   */
  void apply(ParsedShape& shape);
};

} } // namespace simCore::GOG

#endif // SIMCORE_GOG_GOGUTILS_H
