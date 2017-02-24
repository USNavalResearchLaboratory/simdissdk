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
#include <fstream>
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/GogToGeoFence.h"
#include "simCore/Calc/Units.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/ValidNumber.h"

namespace simCore
{

GogToGeoFence::GogToGeoFence()
{
}

GogToGeoFence::~GogToGeoFence()
{
  // no-op
}

int GogToGeoFence::parse(std::istream& is)
{
  // Container for ll or lla coordinates to be made into fence
  Vec3String coordinates;

  // Keep track of line number for error reporting
  int lineNumber = 0;
  std::string line;
  bool start = false;
  bool poly = false;
  bool off = false;
  std::string name = "";

  // For every line
  while (simCore::getStrippedLine(is, line))
  {
    ++lineNumber;
    std::vector<std::string> tokens;
    simCore::quoteCommentTokenizer(line, tokens); // Tokenize the line

    if (tokens.empty())
      continue;

    const std::string keyword = simCore::lowerCase(tokens[0]); // Make the keyword lowercase

    // We can safely ignore empty lines and lines beginning with these keywords
    if (keyword == "annotation" ||
        keyword == "comment" ||
        keyword == "depthbuffer" ||
        keyword == "extrude" ||
        keyword == "fillcolor" ||
        keyword == "filled" ||
        keyword == "linecolor" ||
        keyword == "linestyle" ||
        keyword == "linewidth" ||
        keyword == "outline" ||
        keyword == "ref" ||
        keyword == "referencepoint" ||
        keyword == "tessellate" ||
        keyword == "version")
      continue;

    // Shapes other than poly are not accepted in this parser
    if (keyword == "arc" ||
        keyword == "circle" ||
        keyword == "ellipse" ||
        keyword == "line")
    {
      SIM_ERROR << "Shape \"" << keyword << "\" not accepted (Line #" << lineNumber << "). Only poly shapes are accepted. Stopping...\n";
      return 1;
    }

    // Check for the end of a shape
    if (keyword == "end")
    {
      if (parseEndKeyword_(lineNumber, start, poly, off, name, coordinates) != 0)
        return 1;
    }

    // Check for the start of a shape
    else if (keyword == "start")
    {
      if (parseStartKeyword_(lineNumber, start) != 0)
        return 1;
    }

    // Check for poly keyword
    else if (keyword == "poly" || keyword == "polygon")
    {
      if (parsePolyKeyword_(lineNumber, start, poly) != 0)
        return 1;
    }

    // Check for off keyword
    else if (keyword == "off")
      off = true;

    // If we've started a shape and it's a poly
    else if (start && poly)
    {
      if (parseShape_(tokens, lineNumber, start, poly, off, name, coordinates) != 0)
        return 1;
    }

    // Other GOG keywords aren't accepted
    else
    {
      SIM_ERROR << "Keyword \"" << tokens[0] << "\" not accepted! (Line #" << lineNumber << "). Stopping...\n";
      return 1;
    }
  }

  return 0;
}

int GogToGeoFence::parseStartKeyword_(int lineNumber, bool& start) const
{
  // Make sure we're not already in the middle of a shape
  if (start)
  {
    SIM_ERROR << "GOG syntax error! Additional \"start\" keyword found before \"end\" (Line #" << lineNumber << "). Stopping...\n";
    return 1;
  }
  start = true;
  return 0;
}

int GogToGeoFence::parsePolyKeyword_(int lineNumber, bool& start, bool& poly) const
{
  // Make sure start was already found
  if (start)
  {
    poly = true;
    return 0;
  }
  else
  {
    SIM_ERROR << "GOG syntax error! Need \"start\" keyword before \"poly\" (Line #" << lineNumber << "). Stopping...\n";
    return 1;
  }
}

int GogToGeoFence::parseEndKeyword_(int lineNumber, bool& start, bool& poly, bool& off, std::string& name, simCore::Vec3String& coordinates)
{
  if (!start && !poly)
  {
    SIM_ERROR << "GOG syntax error! \"end\" keyword found before \"start\" (Line #" << lineNumber << "). Stopping...\n";
    return 1;
  }

  // Don't create a fence if the off keyword was found in this shape
  if (!off)
  {
    // Save this shape's coordinates vector
    coordinatesVec_.push_back(coordinates);

    // Generate a simCore::GeoFence for this shape and save it
    std::shared_ptr<simCore::GeoFence> fence(new simCore::GeoFence(coordinates, simCore::COORD_SYS_LLA));

    // Check that newly created GeoFence is convex
    if (!fence->valid())
    {
      SIM_ERROR << "Fence \"" << (name == "" ? "no name" : name) << "\" is concave. This shape will be drawn but will not act as an exclusion zone.\n";
    }
    fences_.push_back(fence);
  }

  // Clear coordinates for the next poly
  coordinates.clear();

  // Clear 3d name
  name = "";

  // Clear flags
  start = false;
  poly = false;
  off = false;

  return 0;
}

int GogToGeoFence::parseShape_(const std::vector<std::string>& tokens, int lineNumber, bool& start, bool& poly, bool& off, std::string& name, simCore::Vec3String& coordinates) const
{
  // Any valid line at this point needs at least 3 tokens
  if (tokens.size() < 3)
  {
    SIM_ERROR << "Line #" << lineNumber << " is invalid, need a keyword and at least two arguments. Stopping...\n";
    return 1;
  }

  const std::string keyword = simCore::lowerCase(tokens[0]); // Make the keyword lowercase

  // check for 3d name keyword
  if (keyword == "3d" && simCore::lowerCase(tokens[1]) == "name")
  {
    name = tokens[2];
    return 0;
  }

  // ll, lla, latlon
  else if ((keyword == "ll" || keyword == "latlon" || keyword == "lla") && tokens.size() >= 3)
  {
    if (parseLatLonAlt_(tokens, lineNumber, coordinates) == 0)
      return 0;

    return 1;
  }

  // Unaccepted keyword found
  SIM_ERROR << "Keyword \"" << keyword << "\" not accepted! (Line #" << lineNumber << ") Stopping...\n";
  return 1;
}

int GogToGeoFence::parseLatLonAlt_(const std::vector<std::string>& tokens, int lineNumber, simCore::Vec3String& coordinates) const
{
  double lat;
  double lon;

  // Make sure lat and lon are valid numbers
  if (!simCore::isValidNumber(tokens[1], lat) || !simCore::isValidNumber(tokens[2], lon))
  {
    SIM_ERROR << "Invalid latitude or longitude value at line #" << lineNumber << ". Stopping...\n";
    return 1;
  }

  // Convert to radians
  lat *= simCore::DEG2RAD;
  lon *= simCore::DEG2RAD;

  // Check for altitude coordinate
  if (tokens.size() > 3)
  {
    double alt = 0.0;
    if (simCore::isValidNumber(tokens[3], alt))
    {
      // Token 4 is a valid number, so it must be an altitude value. Convert from feet to meters and add to coordinates vector
      alt = Units::FEET.convertTo(Units::METERS, alt);
      coordinates.push_back(simCore::Vec3(lat, lon, alt));
      return 0;
    }

    else
    {
      SIM_ERROR << "Invalid altitude value \""<< tokens[3] << "\" at line #" << lineNumber << ". Stopping...\n";
      return 1;
    }
  }

  // Add to coordinates vector
  coordinates.push_back(simCore::Vec3(lat, lon, 0.0));

  return 0;
}

void GogToGeoFence::getFences(simCore::GogToGeoFence::GeoFenceVec& fences) const
{
  fences = fences_;
}

void GogToGeoFence::getCoordinatesVec(std::vector<simCore::Vec3String>& vec) const
{
  vec = coordinatesVec_;
}

void GogToGeoFence::clear()
{
  coordinatesVec_.clear();
  fences_.clear();
}

}