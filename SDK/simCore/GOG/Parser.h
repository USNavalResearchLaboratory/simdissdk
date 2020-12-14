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
#ifndef SIMCORE_GOG_PARSER_H
#define SIMCORE_GOG_PARSER_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/GOG/GogShape.h"
#include "simCore/GOG/ParsedShape.h"

namespace simCore {

class UnitsRegistry;

namespace GOG
{
class UnitsState;

/**
 * Parses GOG files (streams).
 *
 * The GOG Parser will read a GOG file or stream, and encode it into a vector
 * of GogShape objects.  This is an in-memory representation of the GOG shape
 * data for the input stream.
 */
class SDKCORE_EXPORT Parser
{
public:

  /// Constructs a GOG parser.
  Parser();

  /// Virtual destructor
  virtual ~Parser();

  /** Changes the Units Registry for unit conversions. */
  void setUnitsRegistry(const simCore::UnitsRegistry* registry);

  /**
  * Add or overwrite a color key with a new color
  * @param[in ] key   GOG key like color1, color2, red, black,...
  * @param[in ] color The color to use for the given key in GOG hex string format, 0xAABBGGRR
  */
  void addOverwriteColor(const std::string& key, const std::string& colorHex);

  /**
   * Parses an input GOG stream into a vector of GogShapes
   * @param[in ] input GOG input data
   * @param[out] output Vector that will contain a GogShape object for each shape in the input stream.
   */
  void parse(std::istream& input, std::vector<GogShapePtr>& output) const;

private:
  /// Get a GogShape for the specified parsed shape, returns an empty ptr if could not convert
  GogShapePtr getShape_(const ParsedShape& parsed) const;
  /// Parses the optional field for an OutlinedShape
  void parseOutlined_(const ParsedShape& parsed, OutlinedShape* shape) const;
  /// Parses the fields for a FillableShape, which are all optional, calls parseOutlined_()
  void parseFillable_(const ParsedShape& parsed, const std::string& name, FillableShape* shape) const;
  /// Parse the required fields for the specified PointBasedShape, calls parsePointBasedOptional_(); returns 0 on success, non-zero otherwise
  int parsePointBased_(const ParsedShape& parsed, bool relative, const std::string& name, const UnitsState& units, size_t minimumNumPoints, PointBasedShape* shape) const;
  /// Parses the optional fields for a PointBasedShape, calls parseFillable_()
  void parsePointBasedOptional_(const ParsedShape& parsed, const std::string& name, PointBasedShape* shape) const;
  /// Parses the optional fields a CircularShape, calls parseFillable_()
  void parseCircularOptional_(const ParsedShape& parsed, bool relative, const std::string& name, const UnitsState& units, CircularShape* shape) const;
  /// Parses the optional height field for a CircularHeightShape
  void parseCircularHeightOptional_(const ParsedShape& parsed, const std::string& name, const UnitsState& units, CircularHeightShape* shape) const;
  /// Parses the optional fields for an EllipticalShape
  void parseEllipticalOptional_(const ParsedShape& parsed, const std::string& name, const UnitsState& units, EllipticalShape* shape) const;

  /// Return true if the specified token is a comment
  bool isComment_(const std::string& token) const;
  /// Get the color value from the specified parameter in the parsed shape; returns 0 on success, non-zero otherwise
  int getColor_(const ParsedShape& parsed, ShapeParameter param, const std::string& shapeName, const std::string& fieldName, Color& color) const;
  // Get the positions from the specified PositionStrings, applying unit conversions if necessary; returns 0 on success, non-zero otherwise
  int getPosition_(const PositionStrings& pos, bool relative, const UnitsState& units, simCore::Vec3& position) const;
  /// Validate that the specified string converts to a double properly, print error on failure; return 0 on success, non-zero otherwise
  int validateDouble_(const std::string& valueStr, const std::string& paramName, const std::string& name, size_t lineNumber, double& value) const;

  /// Initialize the default GOG colors
  void initGogColors_();
  /// Converts an GOG color string into GOG hex string format (0xAABBGGRR)
  std::string parseGogColor_(const std::string& c, bool isHex) const;

  /// Prints any GOG parsing error to simNotify
  void printError_(size_t lineNumber, const std::string& errorText) const;

private:
  const simCore::UnitsRegistry* units_; ///< registry for unit conversions
  std::map<std::string, std::string> colors_; ///< maps GOG color keywords to GOG hex string format (0xAABBGGRR), e.g. "white", "color1"
};

} } // namespace simCore::GOG

#endif // SIMCORE_GOG_PARSER_H
