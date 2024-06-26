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
#ifndef SIMCORE_GOG_GOGUTILS_H
#define SIMCORE_GOG_GOGUTILS_H

#include <optional>
#include <string>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Units.h"

namespace simCore {

class UnitsRegistry;

namespace GOG
{

class ParsedShape;

/// Recommended GOG serialization precision (number of total digits, not decimal places)
inline constexpr int GOG_PRECISION = 12;

/** Generic reusable GOG-related methods */
class SDKCORE_EXPORT GogUtils
{
public:
  /** Converts an annotation string to a displayable string, de-encoding newlines and underscores */
  static std::string decodeAnnotation(const std::string& anno);

  /**
   * Processes a URL e.g. from imagefile or annotations. Removes quotes and attempts to dereference
   * file protocol ("file://") links to raw filenames.
   */
  static std::string processUrl(const std::string& addr);
};

/**
 * Current state of default units. This object communicates to parsing elements
 * what Units are in effect when parsing coordinate and measurement data.
 */
class SDKCORE_EXPORT UnitsState
{
public:
  /// Construct the units state
  UnitsState();
  virtual ~UnitsState() {}

  /// Get the current altitude units, returns default units if not set
  const simCore::Units& altitudeUnits() const;
  /// Set the current altitude units
  void setAltitudeUnits(const simCore::Units& units);
  /// Returns true if altitude units has been set
  bool hasAltitudeUnits() const;

  /// Get the current angle units, returns default units if not set
  const simCore::Units& angleUnits() const;
  /// Set the current angle units
  void setAngleUnits(const simCore::Units& units);
  /// Returns true if angle units has been set
  bool hasAngleUnits() const;

  /// Get the current range units, returns default units if not set
  const simCore::Units& rangeUnits() const;
  /// Set the current range units
  void setRangeUnits(const simCore::Units& units);
  /// Returns true if range units has been set
  bool hasRangeUnits() const;

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
  void parse(const std::string& unitString, const simCore::UnitsRegistry& unitsRegistry, std::optional<simCore::Units>& units);

private:
  std::optional<simCore::Units> altitudeUnits_; ///< Altitude units
  std::optional<simCore::Units> rangeUnits_; ///< Range units
  std::optional<simCore::Units> angleUnits_; ///< Angle units
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
