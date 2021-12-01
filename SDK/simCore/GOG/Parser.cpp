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
#include <iomanip>

#include "simNotify/Notify.h"
#include "simCore/Common/Exception.h"
#include "simCore/Common/Optional.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Time/String.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Mgrs.h"
#include "simCore/Calc/Units.h"
#include "simCore/GOG/GogUtils.h"
#include "simCore/GOG/Parser.h"

namespace
{

/// Returns true if the specified token starts with the specified start string
bool startsWith(const std::string& token, const std::string& start)
{
  return token.substr(0, start.size()) == start;
}

}

//------------------------------------------------------------------------

namespace simCore { namespace GOG {

Parser::Parser()
  : units_(nullptr)
{
  initGogColors_();

  // no checks on version
  unhandledKeywords_.insert("version");
  // not supported
  unhandledKeywords_.insert("timeunits");
}

Parser::~Parser()
{}

void Parser::initGogColors_()
{
  // GOG hex colors are AABBGGRR
  colors_["color1"] = "0xffffff00"; // Cyan
  colors_["color2"] = "0xff0000ff"; // Red
  colors_["color3"] = "0xff00ff00"; // Lime
  colors_["color4"] = "0xffff0000"; // Blue
  colors_["color5"] = "0xff00ffff"; // Yellow
  colors_["color6"] = "0xff00a5ff"; // Orange
  colors_["color7"] = "0xffffffff"; // White

  colors_["cyan"] = "0xffffff00";  // Cyan
  colors_["red"] = "0xff0000ff"; // Red
  colors_["green"] = "0xff00ff00"; // Lime
  colors_["blue"] = "0xffff0000";  // Blue
  colors_["yellow"] = "0xff00ffff"; // Yellow
  colors_["orange"] = "0xff00a5ff"; // Orange
  colors_["white"] = "0xffffffff"; // White
  colors_["black"] = "0xff000000"; // Black
  colors_["magenta"] = "0xffc000c0"; // Magenta
}

std::string Parser::parseGogColor_(const std::string& color) const
{
  auto it = colors_.find(simCore::lowerCase(color));
  if (it != colors_.end())
    return it->second;
  return "0xff0000ff";
}

void Parser::addOverwriteColor(const std::string& key, const std::string& color)
{
  if (key.empty())
    return;

  // append prefix if necessary
  std::string prefix = "0x";
  if (startsWith(color, prefix))
    prefix = "";

  colors_[simCore::lowerCase(key)] = simCore::lowerCase(prefix + color);
}

void Parser::setUnitsRegistry(const simCore::UnitsRegistry* registry)
{
  units_ = registry;
}

void Parser::parse(std::istream& input, const std::string& filename, std::vector<GogShapePtr>& output) const
{
  // Set up the modifier state object with default values. The state persists
  // across the parsing of the GOG input for annotations, spanning actual objects.
  // (e.g. if the line color is set within the scope of one annotation, that value
  // remains active for future annotations until it is set again.)
  ModifierState state;

  // valid commands must occur within a start/end block
  bool validStartEndBlock = false;
  bool invalidShape = false;

  ParsedShape current;
  std::string line;
  // reference origin settings within a start/end block
  simCore::Optional<PositionStrings> refLla;

  // track line number parsed for error reporting
  size_t lineNumber = 0;

  std::vector<std::string> tokens;
  while (simCore::getStrippedLine(input, line))
  {
    ++lineNumber;
    simCore::quoteTokenizer(tokens, line);

    // convert tokens to lower case (unless it's in quotes or commented)
    for (std::string& token : tokens)
    {
      if (token[0] != '"' && token[0] != '#' && token[0] !=  '/')
      {
        token = simCore::lowerCase(token);
        // stop further lower case conversion on text based values
        if (token == "annotation" || token == "comment" || token == "name" || token == "starttime" || token == "endtime")
          break;
      }
    }
    // rewrite the line now that it's lowered.
    line = simCore::join(tokens, " ");

    if (tokens.empty())
    {
      // skip empty line
      continue;
    }

    // determine if the command is within a valid start/end block
    // acceptable commands are: comments, start and version
    if (!validStartEndBlock && !isComment_(tokens[0]) && tokens[0] != "start" && tokens[0] != "version")
    {
      std::stringstream errorText;
      errorText << "token \"" << tokens[0] << "\" detected outside of a valid start/end block";
      printError_(filename, lineNumber, errorText.str());
      // skip command
      continue;
    }

    if (isComment_(tokens[0]))
    {
      // NOTE: this will only store comments within a start/end block
      current.addComment(line);

      // process deprecated KML icon comment keywords
      if (tokens.size () > 2 && tokens[1] == "kml_icon")
        current.set(ShapeParameter::IMAGE, tokens[2]);
      if (tokens.size() > 1 && tokens[1] == "kml_groundoverlay")
        current.setShape(ShapeType::IMAGEOVERLAY);
      if (tokens.size() > 1 && tokens[1] == "kml_latlonbox")
      {
        if (tokens.size() > 6)
        {
          current.set(ShapeParameter::LLABOX_N, tokens[2]);
          current.set(ShapeParameter::LLABOX_S, tokens[3]);
          current.set(ShapeParameter::LLABOX_E, tokens[4]);
          current.set(ShapeParameter::LLABOX_W, tokens[5]);
          current.set(ShapeParameter::LLABOX_ROT, tokens[6]);
        }
      }
    }
    else if (tokens[0] == "start" || tokens[0] == "end")
    {
      if (validStartEndBlock && tokens[0] == "start")
      {
        printError_(filename, lineNumber, "nested start command not allowed");
        continue;
      }
      if (!validStartEndBlock && tokens[0] == "end")
      {
        printError_(filename, lineNumber, "end command encountered before start");
        continue;
      }
      if (tokens[0] == "end" && current.shape() == ShapeType::UNKNOWN)
      {
        printError_(filename, lineNumber, "end command encountered before recognized GOG shape type keyword");
        continue;
      }

      // apply all cached information to shape when end is reached, only if shape is valid
      if (tokens[0] == "end" && !invalidShape)
      {
        // set the relative state based on point type if it hasn't already been specified
        if (!current.hasValue(ShapeParameter::ABSOLUTE_POINTS) && current.pointType() == ParsedShape::LLA)
          current.set(ShapeParameter::ABSOLUTE_POINTS, "1");
        state.apply(current);
        current.setFilename(filename);
        GogShapePtr gog = getShape_(current);
        if (gog)
          output.push_back(gog);
      }

      // clear reference origin settings for new block of commands
      refLla.reset();
      invalidShape = false;

      // "start" indicates a valid block, "end" indicates the block of commands are complete and subsequent commands will be invalid
      validStartEndBlock = (tokens[0] == "start");
      current.reset();
      current.setLineNumber(lineNumber);
      state = ModifierState();
    }
    else if (tokens[0] == "annotation")
    {
      if (tokens.size() >= 2)
      {
        // special case: annotations. you can have multiple annotations within
        // a single start/end block.
        if (current.shape() == ShapeType::ANNOTATION)
        {
          // set the relative state based on point type if it hasn't already been specified
          if (!current.hasValue(ShapeParameter::ABSOLUTE_POINTS) && current.pointType() == ParsedShape::LLA)
            current.set(ShapeParameter::ABSOLUTE_POINTS, "1");
          state.apply(current);
          current.setFilename(filename);
          GogShapePtr gog = getShape_(current);
          if (gog)
            output.push_back(gog);
          current.reset();
          // if available, recreate reference origin
          // values are needed for subsequent annotation points since a new "current" is used
          if (refLla.has_value())
            current.set(ShapeParameter::REF_LLA, refLla.value_or(PositionStrings()));
        }
        if (current.shape() != ShapeType::UNKNOWN)
        {
          SIM_WARN << "Multiple shape keywords found in single start/end block, " << filename << " line: " << lineNumber << "\n";
          invalidShape = true;
        }
        current.setShape(ShapeType::ANNOTATION);
        std::string textToken = simCore::StringUtils::trim(line.substr(tokens[0].length() + 1));
        // clean up text
        textToken = simCore::StringUtils::substitute(textToken, "_", " ");
        textToken = simCore::StringUtils::substitute(textToken, "\\n", "\n");
        current.set(ShapeParameter::TEXT, textToken);
        current.set(ShapeParameter::NAME, textToken);
        invalidShape = false;
      }
      else
      {
        printError_(filename, lineNumber, "annotation command requires at least 1 argument");
        // shape is recognized, but invalid, so set the shape type correctly
        current.setShape(ShapeType::ANNOTATION);
        invalidShape = true;
      }
    }
    // object types
    else if (
      tokens[0] == "circle"        ||
      tokens[0] == "ellipse"       ||
      tokens[0] == "arc"           ||
      tokens[0] == "cylinder"      ||
      tokens[0] == "hemisphere"    ||
      tokens[0] == "sphere"        ||
      tokens[0] == "ellipsoid"     ||
      tokens[0] == "points"        ||
      tokens[0] == "line"          ||
      tokens[0] == "poly"          ||
      tokens[0] == "polygon"       ||
      tokens[0] == "linesegs"      ||
      tokens[0] == "cone"          ||
      tokens[0] == "orbit"
      )
    {
      if (current.shape() != ShapeType::UNKNOWN)
      {
        SIM_WARN << "Multiple shape keywords found in single start/end block, " << filename << " line: " << lineNumber << "\n";
        invalidShape = true;
      }
      current.setShape(GogShape::stringToShapeType(tokens[0]));
    }
    else if (tokens[0] == "latlonaltbox")
    {
      if (tokens.size() > 5)
      {
        if (current.shape() != ShapeType::UNKNOWN)
        {
          SIM_WARN << "Multiple shape keywords found in single start/end block, " << filename << " line: " << lineNumber << "\n";
          invalidShape = true;
        }
        current.setShape(ShapeType::LATLONALTBOX);
        current.set(ShapeParameter::LLABOX_N, tokens[1]);
        current.set(ShapeParameter::LLABOX_S, tokens[2]);
        current.set(ShapeParameter::LLABOX_W, tokens[3]);
        current.set(ShapeParameter::LLABOX_E, tokens[4]);
        current.set(ShapeParameter::LLABOX_MINALT, tokens[5]);
        if (tokens.size() > 6)
          current.set(ShapeParameter::LLABOX_MAXALT, tokens[6]);
      }
      else
      {
        printError_(filename, lineNumber, "latlonaltbox command requires at least 5 arguments");
      }
    }
    else if (tokens[0] == "imageoverlay")
    {
      if (tokens.size() > 4)
      {
        if (current.shape() != ShapeType::UNKNOWN)
        {
          SIM_WARN << "Multiple shape keywords found in single start/end block, " << filename << " line: " << lineNumber << "\n";
          invalidShape = true;
        }
        current.setShape(ShapeType::IMAGEOVERLAY);
        current.set(ShapeParameter::LLABOX_N, tokens[1]);
        current.set(ShapeParameter::LLABOX_S, tokens[2]);
        current.set(ShapeParameter::LLABOX_W, tokens[3]);
        current.set(ShapeParameter::LLABOX_E, tokens[4]);
        if (tokens.size() > 5)
          current.set(ShapeParameter::LLABOX_ROT, tokens[5]);
      }
      else
      {
        printError_(filename, lineNumber, "imageoverlay command requires at least 4 arguments");
      }
    }
    // arguments
    else if (tokens[0] == "off")
    {
      current.set(ShapeParameter::DRAW, "false");
    }
    else if (tokens[0] == "ref" || tokens[0] == "referencepoint")
    {
      if (tokens.size() >= 3)
      {
        // cache reference origin line and values for repeated use by GOG objects within a start/end block, such as annotations
        if (tokens.size() >= 4)
          refLla = PositionStrings(tokens[1], tokens[2], tokens[3]);
        else
          refLla = PositionStrings(tokens[1], tokens[2]);
        current.set(ShapeParameter::REF_LLA, refLla.value_or(PositionStrings()));
      }
      else
      {
        printError_(filename, lineNumber, "ref/referencepoint command requires at least 2 arguments");
      }
    }
    // geometric data
    else if (tokens[0] == "xy" || tokens[0] == "xyz")
    {
      if (tokens.size() >= 3)
      {
        if (tokens.size() >= 4)
          current.append(ParsedShape::XYZ, PositionStrings(tokens[1], tokens[2], tokens[3]));
        else
          current.append(ParsedShape::XYZ, PositionStrings(tokens[1], tokens[2]));
      }
      else
      {
        printError_(filename, lineNumber, "xy/xyz command requires at least 2 arguments");
      }
    }
    else if (tokens[0] == "ll" || tokens[0] == "lla" || tokens[0] == "latlon")
    {
      if (tokens.size() >= 3)
      {
        if (tokens.size() >= 4)
          current.append(ParsedShape::LLA, PositionStrings(tokens[1], tokens[2], tokens[3]));
        else
          current.append(ParsedShape::LLA, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(filename, lineNumber, "ll/lla/latlon command requires at least 2 arguments");
    }
    else if (tokens[0] == "mgrs")
    {
      if (tokens.size() >= 2)
      {
        double lat;
        double lon;
        if (simCore::Mgrs::convertMgrsToGeodetic(tokens[1], lat, lon) != 0)
          printError_(filename, lineNumber, "Unable to convert MGRS coordinate to lat/lon");
        else
        {
          const std::string& latString = simCore::buildString("", lat * simCore::RAD2DEG);
          const std::string& lonString = simCore::buildString("", lon * simCore::RAD2DEG);
          if (tokens.size() >= 3)
            current.append(ParsedShape::LLA, PositionStrings(latString, lonString, tokens[2]));
          else
            current.append(ParsedShape::LLA, PositionStrings(latString, lonString));
        }
      }
      else
        printError_(filename, lineNumber, "mgrs command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerxy" || tokens[0] == "centerxyz")
    {
      if (tokens.size() >= 3)
      {
        current.set(ShapeParameter::ABSOLUTE_POINTS, "0");
        if (tokens.size() >= 4)
          current.set(ShapeParameter::CENTERXY, PositionStrings(tokens[1], tokens[2], tokens[3]));
        else
          current.set(ShapeParameter::CENTERXY, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(filename, lineNumber, "centerxy/centerxyz command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerxy2")
    {
      if (tokens.size() >= 3)
      {
        current.set(ShapeParameter::ABSOLUTE_POINTS, "0");
        current.set(ShapeParameter::CENTERXY2, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(filename, lineNumber, "centerxy2 command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerll" || tokens[0] == "centerlla" || tokens[0] == "centerlatlon")
    {
      if (tokens.size() >= 3)
      {
        current.set(ShapeParameter::ABSOLUTE_POINTS, "1");
        if (tokens.size() >= 4)
          current.set(ShapeParameter::CENTERLL, PositionStrings(tokens[1], tokens[2], tokens[3]));
        else
          current.set(ShapeParameter::CENTERLL, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(filename, lineNumber, "centerll/centerlla/centerlatlon command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerll2" || tokens[0] == "centerlatlon2")
    {
      if (tokens.size() >= 3)
      {
        current.set(ShapeParameter::ABSOLUTE_POINTS, "1");
        // note centerll2 only supports lat and lon, altitude for shape must be derived from first center point
        current.set(ShapeParameter::CENTERLL2, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(filename, lineNumber, "centerll2 command requires at least 2 arguments");
    }
    // persistent state modifiers:
    else if (tokens[0] == "linecolor")
    {
      if (tokens.size() == 2)
        state.lineColor_ = parseGogColor_(tokens[1]);
      else if (tokens.size() == 3)
        state.lineColor_ = tokens[2];
      else
        printError_(filename, lineNumber, "linecolor command requires at least 1 argument");
    }
    else if (tokens[0] == "fillcolor")
    {
      if (tokens.size() == 2)
        state.fillColor_ = parseGogColor_(tokens[1]);
      else if (tokens.size() == 3)
        state.fillColor_ = tokens[2];
      else
        printError_(filename, lineNumber, "fillcolor command requires at least 1 argument");
    }
    else if (tokens[0] == "linewidth")
    {
      if (tokens.size() >= 2)
        state.lineWidth_ = tokens[1];
      else
        printError_(filename, lineNumber, "linewidth command requires 1 argument");
     }
    else if (tokens[0] == "pointsize")
    {
      if (tokens.size() >= 2)
        state.pointSize_ = tokens[1];
      else
        printError_(filename, lineNumber, "pointsize command requires 1 argument");
    }
    else if (tokens[0] == "altitudemode")
    {
      if (tokens.size() >= 2)
        state.altitudeMode_ = tokens[1];
      else
        printError_(filename, lineNumber, "altitudemode command requires 1 argument");
    }
    else if (tokens[0] == "altitudeunits")
    {
      if (tokens.size() >= 2)
      {
        std::string restOfLine = simCore::StringUtils::trim(line.substr(tokens[0].size() + 1));
        state.altitudeUnits_ = restOfLine;
      }
      else
        printError_(filename, lineNumber, "altitudeunits command requires 1 argument");
    }
    else if (tokens[0] == "rangeunits")
    {
      if (tokens.size() >= 2)
      {
        std::string restOfLine = simCore::StringUtils::trim(line.substr(tokens[0].size() + 1));
        state.rangeUnits_ = restOfLine;
      }
      else
        printError_(filename, lineNumber, "rangeunits command requires 1 argument");
    }
    else if (tokens[0] == "angleunits")
    {
      if (tokens.size() >= 2)
      {
        std::string restOfLine = simCore::StringUtils::trim(line.substr(tokens[0].size() + 1));
        state.angleUnits_ = restOfLine;
      }
      else
        printError_(filename, lineNumber, "angleunits command requires 1 argument");
    }
    else if (tokens[0] == "verticaldatum")
    {
      if (tokens.size() >= 2)
        state.verticalDatum_ = tokens[1];
      else
        printError_(filename, lineNumber, "verticaldatum command requires 1 argument");
    }
    else if (tokens[0] == "priority")
    {
      if (tokens.size() >= 2)
        state.priority_ = tokens[1];
      else
        printError_(filename, lineNumber, "priority command requires 1 argument");
    }
    else if (tokens[0] == "filled")
    {
      current.set(ShapeParameter::FILLED, "true");
    }
    else if (tokens[0] == "outline")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::OUTLINE, tokens[1]);
      else
        printError_(filename, lineNumber, "outline command requires 1 argument");
    }
    else if (tokens[0] == "textoutlinecolor")
    {
      if (tokens.size() == 2)
        state.textOutlineColor_ = parseGogColor_(tokens[1]);
      else if (tokens.size() == 3)
        state.textOutlineColor_ = tokens[2];
      else
        printError_(filename, lineNumber, "textoutlinecolor command requires at least 1 argument");
    }
    else if (tokens[0] == "textoutlinethickness")
    {
      if (tokens.size() >= 2)
        state.textOutlineThickness_ = tokens[1];
      else
        printError_(filename, lineNumber, "textoutlinethickness command requires 1 argument");
    }
    else if (tokens[0] == "diameter")
    {
      if (tokens.size() >= 2)
      {
        double value = 1.0;
        if (simCore::isValidNumber(tokens[1], value))
        {
          std::ostringstream os;
          os << (value * 0.5);
          current.set(ShapeParameter::RADIUS, os.str());
        }
      }
      else
        printError_(filename, lineNumber, "diameter command requires 1 argument");
    }
    else if (tokens[0] == "radius")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::RADIUS, tokens[1]);
      else
        printError_(filename, lineNumber, "radius command requires 1 argument");
    }
    else if (tokens[0] == "innerradius")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::INNERRADIUS, tokens[1]);
      else
        printError_(filename, lineNumber, "innerradius command requires 1 argument");
    }
    else if (tokens[0] == "anglestart")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::ANGLESTART, tokens[1]);
      else
        printError_(filename, lineNumber, "anglestart command requires 1 argument");
    }
    else if (tokens[0] == "angleend")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::ANGLEEND, tokens[1]);
      else
        printError_(filename, lineNumber, "angleend command requires 1 argument");
    }
    else if (tokens[0] == "angledeg")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::ANGLEDEG, tokens[1]);
      else
        printError_(filename, lineNumber, "angledeg command requires 1 argument");
   }
    else if (tokens[0] == "majoraxis")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::MAJORAXIS, tokens[1]);
      else
        printError_(filename, lineNumber, "majoraxis command requires 1 argument");
    }
    else if (tokens[0] == "minoraxis")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::MINORAXIS, tokens[1]);
      else
        printError_(filename, lineNumber, "minoraxis command requires 1 argument");
    }
    else if (tokens[0] == "semimajoraxis")
    {
      if (tokens.size() >= 2)
      {
        double value = 1.0;
        if (simCore::isValidNumber(tokens[1], value))
        {
          std::ostringstream os;
          os << (value * 2.0);
          current.set(ShapeParameter::MAJORAXIS, os.str());
        }
      }
      else
        printError_(filename, lineNumber, "semimajoraxis command requires 1 argument");
    }
    else if (tokens[0] == "semiminoraxis")
    {
      if (tokens.size() >= 2)
      {
        double value = 1.0;
        if (simCore::isValidNumber(tokens[1], value))
        {
          std::ostringstream os;
          os << (value * 2.0);
          current.set(ShapeParameter::MINORAXIS, os.str());
        }
      }
      else
        printError_(filename, lineNumber, "semiminoraxis command requires 1 argument");
    }
    else if (tokens[0] == "scale")
    {
      if (tokens.size() >= 4)
      {
        current.set(ShapeParameter::SCALEX, tokens[1]);
        current.set(ShapeParameter::SCALEY, tokens[2]);
        current.set(ShapeParameter::SCALEZ, tokens[3]);
      }
      else
        printError_(filename, lineNumber, "scale command requires 3 arguments");
    }
    else if (tokens[0] == "orient")
    {
      if (tokens.size() >= 2)
      {
        current.set(ShapeParameter::OFFSETYAW, tokens[1]);
        if (tokens.size() >= 3)
        {
          current.set(ShapeParameter::OFFSETPITCH, tokens[2]);
          if (tokens.size() >= 4)
          {
            current.set(ShapeParameter::OFFSETROLL, tokens[3]);
            current.set(ShapeParameter::FOLLOW, "cpr"); // c=heading(course), p=pitch, r=roll
          }
          else
            current.set(ShapeParameter::FOLLOW, "cp"); // c=heading(course), p=pitch, r=roll
        }
        else
          current.set(ShapeParameter::FOLLOW, "c");
      }
      else
        printError_(filename, lineNumber, "orient command requires at least 1 argument");
    }
    else if (startsWith(line, "rotate"))
      current.set(ShapeParameter::FOLLOW, "cpr"); // c=heading(course), p=pitch, r=roll
    else if (
      startsWith(line, "3d name") ||
      startsWith(line, "3d offsetalt") ||
      startsWith(line, "3d offsetcourse") ||
      startsWith(line, "3d offsetpitch") ||
      startsWith(line, "3d offsetroll") ||
      startsWith(line, "3d follow"))
    {
      if (tokens.size() >= 3)
      {
        const std::string tag = tokens[0] + " " + tokens[1];
        const std::string restOfLine = line.substr(tag.length() + 1);

        if (tokens[1] == "name")
          current.set(ShapeParameter::NAME, restOfLine);
        else if (tokens[1] == "offsetalt")
          current.set(ShapeParameter::OFFSETALT, restOfLine);
        else if (tokens[1] == "offsetcourse") // original terminology was mistaken, they used course when they meant heading/yaw
          current.set(ShapeParameter::OFFSETYAW, restOfLine);
        else if (tokens[1] == "offsetpitch")
          current.set(ShapeParameter::OFFSETPITCH, restOfLine);
        else if (tokens[1] == "offsetroll")
          current.set(ShapeParameter::OFFSETROLL, restOfLine);
        else if (tokens[1] == "follow")
          current.set(ShapeParameter::FOLLOW, restOfLine);
      }
      else
        printError_(filename, lineNumber, "3d command requires at least 2 arguments: " + line);
    }
    else if (startsWith(line, "extrude"))
    {
      if (tokens.size() >= 2)
      {
        // extrusion is an altitude mode
        if (ParsedShape::getBoolFromString(tokens[1]))
          current.set(ShapeParameter::ALTITUDEMODE, "extrude");
        if (tokens.size() >= 3)
        {
          // handle optional extrude height
          current.set(ShapeParameter::EXTRUDE_HEIGHT, tokens[2]);
        }
      }
      else
        printError_(filename, lineNumber, "extrude command requires at least 1 argument");
    }
    else if (tokens[0] == "height")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::HEIGHT, tokens[1]);
      else
        printError_(filename, lineNumber, "height command requires 1 argument");
    }
    else if (tokens[0] == "tessellate")
      current.set(ShapeParameter::TESSELLATE, tokens[1]);
    else if (tokens[0] == "lineprojection")
      current.set(ShapeParameter::LINEPROJECTION, tokens[1]);
    else if (tokens[0] == "linestyle")
      current.set(ShapeParameter::LINESTYLE, tokens[1]);
    else if (tokens[0] == "depthbuffer")
      current.set(ShapeParameter::DEPTHBUFFER, tokens[1]);
    else if (tokens[0] == "fontname")
    {
      state.fontName_ = tokens[1];
      current.set(ShapeParameter::FONTNAME, tokens[1]);
    }
    else if (tokens[0] == "fontsize")
    {
      state.textSize_ = tokens[1];
      current.set(ShapeParameter::TEXTSIZE, tokens[1]);
    }
    else if (tokens[0] == "starttime")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::TIME_START, tokens[1]);
    }
    else if (tokens[0] == "endtime")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::TIME_END, tokens[1]);
    }
    // 3d billboard is OBE, since all annotations are always billboarded
    else if (startsWith(line, "3d billboard"))
      continue;
    else if (tokens[0] == "imagefile")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::IMAGE, tokens[1]);
    }
    else // treat everything as a name/value pair
    {
      if (!tokens.empty())
      {
        // filter out items that are explicitly unhandled
        if (unhandledKeywords_.find(tokens[0]) == unhandledKeywords_.end())
          printError_(filename, lineNumber, "Found unknown GOG command " + line);
      }
    }
  }
}

bool Parser::isComment_(const std::string& token) const
{
  return token == "comment" || token[0] == '#' || token[0] == '/';
}

GogShapePtr Parser::getShape_(const ParsedShape& parsed) const
{
  GogShapePtr rv;
  UnitsState units;
  if (units_)
    units.parse(parsed, *units_);
  else
  {
    simCore::UnitsRegistry registry;
    registry.registerDefaultUnits();
    units.parse(parsed, registry);
  }
  // default to absolute if not otherwise specified
  bool relative = !parsed.boolValue(ShapeParameter::ABSOLUTE_POINTS, false);
  std::string name = parsed.stringValue(ShapeParameter::NAME);
  switch (parsed.shape())
  {
  case ShapeType::ANNOTATION:
  {
    // annotation requires text
    if (!parsed.hasValue(ShapeParameter::TEXT))
    {
      printError_(parsed.filename(), parsed.lineNumber(), "Annotation " + name + " missing text, cannot create shape");
      break;
    }

    simCore::Vec3 position;
    bool hasPosition;
    // get position, annotation supports multiple ways to define center: centerlla or lla, centerxyz or xyz
    if (relative)
    {
      const std::vector<PositionStrings>& positions = parsed.positions();
      hasPosition = (!positions.empty() && getPosition_(positions.front(), relative, units, position) == 0);
      if (!hasPosition)
        hasPosition = (parsed.hasValue(ShapeParameter::CENTERXY) && getPosition_(parsed.positionValue(ShapeParameter::CENTERXY), relative, units, position) == 0);
    }
    else
    {
      const std::vector<PositionStrings>& positions = parsed.positions();
      hasPosition = (!positions.empty() && getPosition_(positions.front(), relative, units, position) == 0);
      if (!hasPosition)
        hasPosition = (parsed.hasValue(ShapeParameter::CENTERLL) && getPosition_(parsed.positionValue(ShapeParameter::CENTERLL), relative, units, position) == 0);
    }

    Annotation* anno = new Annotation(relative);
    if (hasPosition)
      anno->setPosition(position);
    anno->setText(parsed.stringValue(ShapeParameter::TEXT));
    if (parsed.hasValue(ShapeParameter::FONTNAME))
      anno->setFontName(parsed.stringValue(ShapeParameter::FONTNAME));
    if (parsed.hasValue(ShapeParameter::TEXTSIZE))
    {
      // support double input by user and round to int
      double textSize = 0;
      if (simCore::isValidNumber(parsed.stringValue(ShapeParameter::TEXTSIZE), textSize))
        anno->setTextSize(static_cast<int>(simCore::round(textSize)));
      else
        printError_(parsed.filename(), parsed.lineNumber(), "Invalid fontsize: " + parsed.stringValue(ShapeParameter::TEXTSIZE) + " for " + name);
    }
    if (parsed.hasValue(ShapeParameter::LINECOLOR))
    {
      Color color;
      if (getColor_(parsed, ShapeParameter::LINECOLOR, name, "linecolor", color) == 0)
        anno->setTextColor(color);
    }
    if (parsed.hasValue(ShapeParameter::TEXTOUTLINETHICKNESS))
    {
      std::string thicknessStr = parsed.stringValue(ShapeParameter::TEXTOUTLINETHICKNESS);
      OutlineThickness thickness = OutlineThickness::NONE;
      bool valid = true;
      if (thicknessStr == "thick")
        thickness = OutlineThickness::THICK;
      else if (thicknessStr == "thin")
        thickness = OutlineThickness::THIN;
      else if (thicknessStr != "none")
      {
        valid = false;
        printError_(parsed.filename(), parsed.lineNumber(), "Invalid textoutlinethickness: " + thicknessStr + " for " + name);
      }
      if (valid)
        anno->setOutlineThickness(thickness);
    }
    if (parsed.hasValue(ShapeParameter::TEXTOUTLINECOLOR))
    {
      Color color;
      if (getColor_(parsed, ShapeParameter::TEXTOUTLINECOLOR, name, "textoutlinecolor", color) == 0)
        anno->setOutlineColor(color);
    }
    if (parsed.hasValue(ShapeParameter::IMAGE))
      anno->setImageFile(parsed.stringValue(ShapeParameter::IMAGE));
    if (parsed.hasValue(ShapeParameter::PRIORITY))
    {
      double priority = 0.;
      if (validateDouble_(parsed.stringValue(ShapeParameter::PRIORITY), "priority", name, parsed, priority) == 0)
        anno->setPriority(priority);
    }
    rv.reset(anno);
    break;
  }
  case ShapeType::CIRCLE:
  {
    std::unique_ptr<Circle> circle(new Circle(relative));
    parseCircularOptional_(parsed, relative, name, units, circle.get());
    rv.reset(circle.release());
    break;
  }
  case ShapeType::LINE:
  {
    std::unique_ptr<Line> line(new Line(relative));
    if (parsePointBased_(parsed, relative, name, units, 2, line.get()) == 0)
      rv.reset(line.release());
    break;
  }
  case ShapeType::LINESEGS:
  {
    std::unique_ptr<LineSegs> line(new LineSegs(relative));
    if (parsePointBased_(parsed, relative, name, units, 2, line.get()) == 0)
      rv.reset(line.release());
    break;
  }
  case ShapeType::POLYGON:
  {
    std::unique_ptr<Polygon> poly(new Polygon(relative));
    if (parsePointBased_(parsed, relative, name, units, 3, poly.get()) == 0)
      rv.reset(poly.release());
  }
  break;
  case ShapeType::SPHERE:
  {
    std::unique_ptr<Sphere> sphere(new Sphere(relative));
    parseCircularOptional_(parsed, relative, name, units, sphere.get());
    rv.reset(sphere.release());
  }
  break;
  case ShapeType::HEMISPHERE:
  {
    std::unique_ptr<Hemisphere> hemi(new Hemisphere(relative));
    parseCircularOptional_(parsed, relative, name, units, hemi.get());
    rv.reset(hemi.release());
    break;
  }
  case ShapeType::ORBIT:
  {
    std::unique_ptr<Orbit> orbit(new Orbit(relative));
    parseCircularOptional_(parsed, relative, name, units, orbit.get());
    bool hasPoints = false;
    simCore::Vec3 center1;
    // orbit requires both center positions
    if (orbit->getCenterPosition(center1) == 0)
    {
      simCore::Vec3 center2;
      ShapeParameter param = (relative ? ShapeParameter::CENTERXY2 : ShapeParameter::CENTERLL2);
      // verify orbit has required center2 field
      if (parsed.hasValue(param) && getPosition_(parsed.positionValue(param), relative, units, center2) == 0)
      {
        hasPoints = true;
        orbit->setCenterPosition2(center2);
        rv.reset(orbit.release());
      }
    }
    if (!hasPoints)
      printError_(parsed.filename(), parsed.lineNumber(), "orbit " + name + " missing or invalid center points, cannot create shape");
    break;
  }
  case ShapeType::CONE:
  {
    std::unique_ptr<Cone> cone(new Cone(relative));
    parseCircularOptional_(parsed, relative, name, units, cone.get());
    parseCircularHeightOptional_(parsed, name, units, cone.get());
    rv.reset(cone.release());
    break;
  }
  case ShapeType::ELLIPSOID:
  {
    std::unique_ptr<Ellipsoid> ellipsoid(new Ellipsoid(relative));
    parseCircularOptional_(parsed, relative, name, units, ellipsoid.get());
    parseCircularHeightOptional_(parsed, name, units, ellipsoid.get());
    if (parsed.hasValue(ShapeParameter::MAJORAXIS))
    {
      double majorAxis = 0.;
      if (validateDouble_(parsed.stringValue(ShapeParameter::MAJORAXIS), "majoraxis", name, parsed, majorAxis) == 0)
        ellipsoid->setMajorAxis(units.rangeUnits().convertTo(simCore::Units::METERS, majorAxis));
      double minorAxis = 0.;
      if (validateDouble_(parsed.stringValue(ShapeParameter::MINORAXIS), "minoraxis", name, parsed, minorAxis) == 0)
        ellipsoid->setMinorAxis(units.rangeUnits().convertTo(simCore::Units::METERS, minorAxis));
    }
    rv.reset(ellipsoid.release());

    break;
  }
  case ShapeType::POINTS:
  {
    // verify shape has some points
    const std::vector<PositionStrings>& positions = parsed.positions();
    if (positions.empty())
    {
      printError_(parsed.filename(), parsed.lineNumber(), "point " + (name.empty() ? "" : name + " ") + "has no points, cannot create shape");
      break;
    }
    std::unique_ptr<Points> points(new Points(relative));
    for (PositionStrings pos : positions)
    {
      simCore::Vec3 position;
      if (getPosition_(pos, relative, units, position) == 0)
        points->addPoint(position);
    }
    if (points->points().empty())
    {
      printError_(parsed.filename(), parsed.lineNumber(), "point " + (name.empty() ? "" : name + " ") + "has no valid points, cannot create shape");
      break;
    }
    parseOutlined_(parsed, points.get());
    if (parsed.hasValue(ShapeParameter::POINTSIZE))
    {
      // support double input by user and round to int
      double pointSize = 0;
      std::string pointSizeStr = parsed.stringValue(ShapeParameter::POINTSIZE);
      if (simCore::isValidNumber(pointSizeStr, pointSize))
        points->setPointSize(static_cast<int>(simCore::round(pointSize)));
      else
        printError_(parsed.filename(), parsed.lineNumber(), "Invalid pointsize: " + pointSizeStr + (name.empty() ? "" : " for " + name));
    }
    if (parsed.hasValue(ShapeParameter::LINECOLOR))
    {
      Color color;
      if (getColor_(parsed, ShapeParameter::LINECOLOR, name, "linecolor", color) == 0)
        points->setColor(color);
    }
    rv.reset(points.release());
    break;
  }
  case ShapeType::ARC:
  {
    std::unique_ptr<Arc> arc(new Arc(relative));
    parseCircularOptional_(parsed, relative, name, units, arc.get());
    parseEllipticalOptional_(parsed, name, units, arc.get());
    if (parsed.hasValue(ShapeParameter::INNERRADIUS))
    {
      double innerRadius = 0.;
      if (validateDouble_(parsed.stringValue(ShapeParameter::INNERRADIUS), "innerradius", name, parsed, innerRadius) == 0)
      {
        if (innerRadius >= 0.)
          arc->setInnerRadius(units.rangeUnits().convertTo(simCore::Units::METERS, innerRadius));
        else
          printError_(parsed.filename(), parsed.lineNumber(), "innerradius must be non-negative " + (name.empty() ? "" : " for " + name));
      }
    }
    rv.reset(arc.release());
    break;
  }
  case ShapeType::CYLINDER:
  {
    std::unique_ptr<Cylinder> cyl(new Cylinder(relative));
    parseCircularOptional_(parsed, relative, name, units, cyl.get());
    parseEllipticalOptional_(parsed, name, units, cyl.get());
    if (parsed.hasValue(ShapeParameter::HEIGHT))
    {
      double height = 0.;
      if (validateDouble_(parsed.stringValue(ShapeParameter::HEIGHT), "height", name, parsed, height) == 0)
        cyl->setHeight(units.altitudeUnits().convertTo(simCore::Units::METERS, height));
    }
    rv.reset(cyl.release());
    break;
  }
  case ShapeType::ELLIPSE:
  {
    std::unique_ptr<Ellipse> ellipse(new Ellipse(relative));
    parseCircularOptional_(parsed, relative, name, units, ellipse.get());
    parseEllipticalOptional_(parsed, name, units, ellipse.get());
    rv.reset(ellipse.release());
    break;
  }
  case ShapeType::LATLONALTBOX:
  {
    if (parsed.hasValue(ShapeParameter::LLABOX_N) && parsed.hasValue(ShapeParameter::LLABOX_S)
      && parsed.hasValue(ShapeParameter::LLABOX_E) && parsed.hasValue(ShapeParameter::LLABOX_W)
      && parsed.hasValue(ShapeParameter::LLABOX_MINALT))
    {
      std::unique_ptr<LatLonAltBox> llab(new LatLonAltBox());
      int validValues = 0;
      double corner = 0.;
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_N), true, corner) == 0)
      {
        llab->setNorth(corner);
        validValues++;
      }
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_S), true, corner) == 0)
      {
        llab->setSouth(corner);
        validValues++;
      }
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_E), true, corner) == 0)
      {
        llab->setEast(corner);
        validValues++;
      }
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_W), true, corner) == 0)
      {
        llab->setWest(corner);
        validValues++;
      }
      double altitude = 0.;
      if (simCore::isValidNumber(parsed.stringValue(ShapeParameter::LLABOX_MINALT), altitude))
      {
        llab->setAltitude(units.altitudeUnits().convertTo(simCore::Units::METERS, altitude));
        validValues++;
      }
      if (validValues == 5)
      {
        parseFillable_(parsed, name, llab.get());
        if (parsed.hasValue(ShapeParameter::LLABOX_MAXALT))
        {
          double maxAlt = 0.;
          if (simCore::isValidNumber(parsed.stringValue(ShapeParameter::LLABOX_MAXALT), maxAlt))
            llab->setHeight(units.altitudeUnits().convertTo(simCore::Units::METERS, maxAlt - altitude));
        }
        rv.reset(llab.release());
      }
      else
        printError_(parsed.filename(), parsed.lineNumber(), "latlonaltbox " + name + " had invalid values, cannot create shape");
    }
    break;
  }
  case ShapeType::IMAGEOVERLAY:
  {
    if (parsed.hasValue(ShapeParameter::LLABOX_N) && parsed.hasValue(ShapeParameter::LLABOX_S)
      && parsed.hasValue(ShapeParameter::LLABOX_E) && parsed.hasValue(ShapeParameter::LLABOX_W)
      && parsed.hasValue(ShapeParameter::IMAGE))
    {
      std::unique_ptr<ImageOverlay> imageOverlay(new ImageOverlay());
      int validValues = 0;
      double corner = 0.;
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_N), true, corner) == 0)
      {
        imageOverlay->setNorth(corner);
        validValues++;
      }
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_S), true, corner) == 0)
      {
        imageOverlay->setSouth(corner);
        validValues++;
      }
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_E), true, corner) == 0)
      {
        imageOverlay->setEast(corner);
        validValues++;
      }
      if (simCore::getAngleFromDegreeString(parsed.stringValue(ShapeParameter::LLABOX_W), true, corner) == 0)
      {
        imageOverlay->setWest(corner);
        validValues++;
      }
      if (validValues == 4)
      {
        imageOverlay->setImageFile(parsed.stringValue(ShapeParameter::IMAGE));
        if (parsed.hasValue(ShapeParameter::LLABOX_ROT))
        {
          double rotation = 0.;
          if (simCore::isValidNumber(parsed.stringValue(ShapeParameter::LLABOX_ROT), rotation))
            imageOverlay->setRotation(simCore::angFix2PI(rotation * simCore::DEG2RAD));
        }
        rv.reset(imageOverlay.release());
      }
      else
        printError_(parsed.filename(), parsed.lineNumber(), "imageoverlay " + name + " had invalid values, cannot create shape");
    }
    break;
  }
  case ShapeType::UNKNOWN:
    break;
  }
  if (!rv)
    return rv;

  // fill in GogShape fields

  if (parsed.hasValue(ShapeParameter::NAME))
    rv->setName(parsed.stringValue(ShapeParameter::NAME));

  if (parsed.hasValue(ShapeParameter::DRAW))
    rv->setDrawn(parsed.boolValue(ShapeParameter::DRAW, true));

  if (parsed.hasValue(ShapeParameter::DEPTHBUFFER))
    rv->setDepthBufferActive(parsed.boolValue(ShapeParameter::DEPTHBUFFER, false));

  if (parsed.hasValue(ShapeParameter::OFFSETALT))
  {
    double altOffset = 0.;
    if (validateDouble_(parsed.stringValue(ShapeParameter::OFFSETALT), "offsetalt", name, parsed, altOffset) == 0)
      rv->setAltitudeOffset(units.altitudeUnits().convertTo(simCore::Units::METERS, altOffset));
  }

  if (parsed.hasValue(ShapeParameter::ALTITUDEMODE))
  {
    std::string altModeStr = parsed.stringValue(ShapeParameter::ALTITUDEMODE);
    AltitudeMode mode = AltitudeMode::NONE;
    if (altModeStr == "relativetoground")
      mode = AltitudeMode::RELATIVE_TO_GROUND;
    else if (altModeStr == "clamptoground")
      mode = AltitudeMode::CLAMP_TO_GROUND;
    else if (altModeStr == "extrude")
      mode = AltitudeMode::EXTRUDE;
    rv->setAltitudeMode(mode);
  }

  if (parsed.hasValue(ShapeParameter::EXTRUDE_HEIGHT))
  {
    double height = 0.;
    if (validateDouble_(parsed.stringValue(ShapeParameter::EXTRUDE_HEIGHT), "extrude height", name, parsed, height) == 0)
      rv->setExtrudeHeight(units.altitudeUnits().convertTo(simCore::Units::METERS, height));
  }

  if (parsed.hasValue(ShapeParameter::REF_LLA))
  {
    PositionStrings pos = parsed.positionValue(ShapeParameter::REF_LLA);
    double alt = 0.;
    simCore::isValidNumber(pos.z, alt);
    // convert altitude units
    alt = units.altitudeUnits().convertTo(simCore::Units::METERS, alt);
    double lat = 0.;
    double lon = 0.;
    if (simCore::getAngleFromDegreeString(pos.x, true, lat) == 0 && simCore::getAngleFromDegreeString(pos.y, true, lon) == 0)
      rv->setReferencePosition(simCore::Vec3(lat, lon, alt));
    else
      printError_(parsed.filename(), parsed.lineNumber(), "Invalid referencepoint: " + parsed.stringValue(ShapeParameter::REF_LLA) + " for " + name);

  }
  // if SCALEX exists, so should the others
  if (parsed.hasValue(ShapeParameter::SCALEX))
  {
    // parsing error, should not have only one of the scale components set
    if (parsed.hasValue(ShapeParameter::SCALEY) && parsed.hasValue(ShapeParameter::SCALEZ))
      printError_(parsed.filename(), parsed.lineNumber(), "Invalid scale: scalex, scaley, and scalez must be used together to take effect");
    double scaleX = 1.;
    double scaleY = 1.;
    double scaleZ = 1.;
    bool validX = (validateDouble_(parsed.stringValue(ShapeParameter::SCALEX), "scale x", name, parsed, scaleX) == 0);
    bool validY = (validateDouble_(parsed.stringValue(ShapeParameter::SCALEY), "scale y", name, parsed, scaleY) == 0);
    bool validZ = (validateDouble_(parsed.stringValue(ShapeParameter::SCALEZ), "scale z", name, parsed, scaleZ) == 0);
    // only need one valid value, using scale default of 1 otherwise
    if (validX || validY || validZ)
      rv->setScale(simCore::Vec3(scaleX, scaleY, scaleZ));
  }

  if (parsed.hasValue(ShapeParameter::FOLLOW))
  {
    std::string followComponents = parsed.stringValue(ShapeParameter::FOLLOW);
    if (followComponents.find("c") != std::string::npos)
      rv->setFollowYaw(true);
    if (followComponents.find("p") != std::string::npos)
      rv->setFollowPitch(true);
    if (followComponents.find("r") != std::string::npos)
      rv->setFollowRoll(true);
  }

  if (parsed.hasValue(ShapeParameter::OFFSETYAW))
  {
    double yawOffset = 0.;
    // note that original GOG terminology was mistaken, they used course when they meant heading/yaw
    if (validateDouble_(parsed.stringValue(ShapeParameter::OFFSETYAW), "offsetcourse", name, parsed, yawOffset) == 0)
      rv->setYawOffset(simCore::angFix2PI(units.angleUnits().convertTo(simCore::Units::RADIANS, yawOffset)));
  }

  if (parsed.hasValue(ShapeParameter::OFFSETPITCH))
  {
    double pitchOffset = 0.;
    if (validateDouble_(parsed.stringValue(ShapeParameter::OFFSETPITCH), "offsetpitch", name, parsed, pitchOffset) == 0)
      rv->setPitchOffset(simCore::angFix2PI(units.angleUnits().convertTo(simCore::Units::RADIANS, pitchOffset)));
  }

  if (parsed.hasValue(ShapeParameter::OFFSETROLL))
  {
    double rollOffset = 0.;
    if (validateDouble_(parsed.stringValue(ShapeParameter::OFFSETROLL), "offsetroll", name, parsed, rollOffset) == 0)
      rv->setRollOffset(simCore::angFix2PI(units.angleUnits().convertTo(simCore::Units::RADIANS, rollOffset)));
  }

  if (parsed.hasValue(ShapeParameter::VERTICALDATUM))
  {
    std::string vdatum = parsed.stringValue(ShapeParameter::VERTICALDATUM);
    // verify the vertical datum is a known valid string
    if (vdatum == "egm1984" || vdatum == "egm84" || vdatum == "egm1996" || vdatum == "egm96" || vdatum == "egm2008" || vdatum == "egm08" || vdatum == "wgs84")
      rv->setVerticalDatum(vdatum);
  }

  bool hasStart = parsed.hasValue(ShapeParameter::TIME_START);
  bool hasEnd = parsed.hasValue(ShapeParameter::TIME_END);
  if (hasStart || hasEnd)
  {
    simCore::TimeFormatterRegistry formatter(false, false);
    formatter.registerCustomFormatter(std::shared_ptr<TimeFormatter>(new simCore::Iso8601TimeFormatter));
    formatter.registerCustomFormatter(std::shared_ptr<TimeFormatter>(new simCore::DtgTimeFormatter));
    formatter.registerCustomFormatter(std::shared_ptr<TimeFormatter>(new simCore::MonthDayTimeFormatter));
    formatter.registerCustomFormatter(std::shared_ptr<TimeFormatter>(new simCore::OrdinalTimeFormatter));

    bool validStart = false;
    simCore::TimeStamp startTime;
    if (hasStart)
    {
      validStart = (formatter.fromString(parsed.stringValue(ShapeParameter::TIME_START), startTime, 1970) == 0);
      if (!validStart)
        printError_(parsed.filename(), parsed.lineNumber(), "Invalid start time" + (name.empty() ? "" : " for " + name) + ": \"" + parsed.stringValue(ShapeParameter::TIME_START) + "\"");
    }

    bool validEnd = false;
    simCore::TimeStamp endTime;
    if (hasEnd)
    {
      validEnd = (formatter.fromString(parsed.stringValue(ShapeParameter::TIME_END), endTime, 1970) == 0);
      if (!validEnd)
        printError_(parsed.filename(), parsed.lineNumber(), "Invalid end time" + (name.empty() ? "" : " for " + name) + ": \"" + parsed.stringValue(ShapeParameter::TIME_END) + "\"");
    }

    if (validStart || validEnd)
    {
      // If both start and end are defined, check that start is before end.  If only one is defined, set it without any further checks
      if (validStart && validEnd)
      {
        if (startTime <= endTime)
        {
          rv->setStartTime(startTime);
          rv->setEndTime(endTime);
        }
        else
        {
          // disable times on this gog; will always display
          printError_(parsed.filename(), parsed.lineNumber(), "Invalid start and end times" + (name.empty() ? "" : " for " + name) + ": start time must be before end time");
        }
      }

      else if (validStart)
        rv->setStartTime(startTime);
      else if (validEnd)
        rv->setEndTime(endTime);
    }
  }

  for (std::string comment : parsed.comments())
  {
    rv->addComment(comment);
  }
  rv->setLineNumber(parsed.lineNumber());
  rv->setOriginalUnits(units);
  return rv;
}

int Parser::validateDouble_(const std::string& valueStr, const std::string& paramName, const std::string& name, const ParsedShape& parsed, double& value) const
{
  if (simCore::isValidNumber(valueStr, value))
    return 0;
  printError_(parsed.filename(), parsed.lineNumber(), "Invalid " + paramName + ": " + valueStr + (name.empty() ? "" : " for " + name));
  return 1;
}

void Parser::parseOutlined_(const ParsedShape& parsed, OutlinedShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return;
  }
  if (parsed.hasValue(ShapeParameter::OUTLINE))
    shape->setOutlined(parsed.boolValue(ShapeParameter::OUTLINE, true));
}

void Parser::parseFillable_(const ParsedShape& parsed, const std::string& name, FillableShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return;
  }
  parseOutlined_(parsed, shape);
  if (parsed.hasValue(ShapeParameter::LINECOLOR))
  {
    Color color;
    if (getColor_(parsed, ShapeParameter::LINECOLOR, name, "linecolor", color) == 0)
      shape->setLineColor(color);
    else
      printError_(parsed.filename(), parsed.lineNumber(), "Invalid linecolor: " + parsed.stringValue(ShapeParameter::LINECOLOR) + (name.empty() ? "" : " for " + name));
  }
  if (parsed.hasValue(ShapeParameter::LINESTYLE))
  {
    std::string styleStr = parsed.stringValue(ShapeParameter::LINESTYLE);
    bool valid = true;
    LineStyle style = LineStyle::SOLID;
    if (styleStr == "dashed" || styleStr == "dash")
      style = LineStyle::DASHED;
    else if (styleStr == "dotted" || styleStr == "dot")
      style = LineStyle::DOTTED;
    else if (styleStr != "solid")
    {
      printError_(parsed.filename(), parsed.lineNumber(), "Invalid linestyle: " + styleStr + (name.empty() ? "" : " for " + name));
      valid = false;
    }
    if (valid)
      shape->setLineStyle(style);
  }
  if (parsed.hasValue(ShapeParameter::LINEWIDTH))
  {
    // support double input by user and round to int
    double lineWidth = 0;
    std::string lineWidthStr = parsed.stringValue(ShapeParameter::LINEWIDTH);
    if (simCore::isValidNumber(lineWidthStr, lineWidth))
      shape->setLineWidth(static_cast<int>(simCore::round(lineWidth)));
    else
    {
      std::string lowerLineWidth = simCore::lowerCase(lineWidthStr);
      if (lowerLineWidth == "thin")
        shape->setLineWidth(1);
      else if (lowerLineWidth == "med" || lowerLineWidth == "medium")
        shape->setLineWidth(2);
      else if (lowerLineWidth == "thick")
        shape->setLineWidth(4);
      else
        printError_(parsed.filename(), parsed.lineNumber(), "Invalid linewidth: " + lineWidthStr + (name.empty() ? "" : " for " + name));
    }
  }
  if (parsed.hasValue(ShapeParameter::FILLED))
    shape->setFilled(parsed.boolValue(ShapeParameter::FILLED, true));
  if (parsed.hasValue(ShapeParameter::FILLCOLOR))
  {
    Color color;
    if (getColor_(parsed, ShapeParameter::FILLCOLOR, name, "fillcolor", color) == 0)
      shape->setFillColor(color);
    else
      printError_(parsed.filename(), parsed.lineNumber(), "Invalid fillcolor: " + parsed.stringValue(ShapeParameter::LINECOLOR) + (name.empty() ? "" : " for " + name));
  }
}

int Parser::parsePointBased_(const ParsedShape& parsed, bool relative, const std::string& name, const UnitsState& units, size_t minimumNumPoints, PointBasedShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return 1;
  }
  std::string shapeTypeName = GogShape::shapeTypeToString(shape->shapeType());
  const std::vector<PositionStrings>& positions = parsed.positions();
  if (positions.empty())
  {
    printError_(parsed.filename(), parsed.lineNumber(), shapeTypeName + (name.empty() ? "" : " " + name) + " has no points, cannot create shape");
    return 1;
  }
  else if (positions.size() < minimumNumPoints)
  {
    printError_(parsed.filename(), parsed.lineNumber(), shapeTypeName + (name.empty() ? "" : " " + name) + " has less than the required number of points, cannot create shape");
    return 1;
  }
  for (PositionStrings pos : positions)
  {
    simCore::Vec3 position;
    if (getPosition_(pos, relative, units, position) == 0)
      shape->addPoint(position);
  }
  if (shape->points().empty())
  {
    printError_(parsed.filename(), parsed.lineNumber(), shapeTypeName + (name.empty() ? "" : " " + name) + " has no valid points, cannot create shape");
    return 1;
  }
  else if (shape->points().size() < minimumNumPoints)
  {
    printError_(parsed.filename(), parsed.lineNumber(), shapeTypeName + (name.empty() ? "" : " " + name) + " has less than the required number of valid points, cannot create shape");
    return 1;
  }
  parsePointBasedOptional_(parsed, name, shape);
  return 0;
}

void Parser::parsePointBasedOptional_(const ParsedShape& parsed, const std::string& name, PointBasedShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return;
  }
  parseFillable_(parsed, name, shape);
  if (!parsed.hasValue(ShapeParameter::TESSELLATE))
    return;
  TessellationStyle style = TessellationStyle::NONE;
  // set style to none if tessellate is set to false
  if (!parsed.boolValue(ShapeParameter::TESSELLATE, false))
    shape->setTesssellation(style);
  else
  {
    // if tessellate is set, default to RHUMBLINE unless LINEPROJECTION specifies otherwise
    style = TessellationStyle::RHUMBLINE;
    if (parsed.hasValue(ShapeParameter::LINEPROJECTION))
    {
      if (parsed.stringValue(ShapeParameter::LINEPROJECTION) == "greatcircle")
        style = TessellationStyle::GREAT_CIRCLE;
    }
    shape->setTesssellation(style);
  }
}

void Parser::parseCircularOptional_(const ParsedShape& parsed, bool relative, const std::string& name, const UnitsState& units, CircularShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return;
  }
  parseFillable_(parsed, name, shape);

  simCore::Vec3 position;
  ShapeParameter param = (relative ? ShapeParameter::CENTERXY : ShapeParameter::CENTERLL);
  bool foundPosition = false;
  if (parsed.hasValue(param))
  {
    if (getPosition_(parsed.positionValue(param), relative, units, position) == 0)
      shape->setCenterPosition(position);
    else
      printError_(parsed.filename(), parsed.lineNumber(), GogShape::shapeTypeToString(shape->shapeType()) + (name.empty() ? "" : " " + name) + " invalid center point");
  }

  if (!parsed.hasValue(ShapeParameter::RADIUS))
    return;
  double radius = 0.;
  if (validateDouble_(parsed.stringValue(ShapeParameter::RADIUS), "radius", name, parsed, radius) == 0)
    shape->setRadius(units.rangeUnits().convertTo(simCore::Units::METERS, radius));
}

void Parser::parseCircularHeightOptional_(const ParsedShape& parsed, const std::string& name, const UnitsState& units, CircularHeightShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return;
  }
  if (!parsed.hasValue(ShapeParameter::HEIGHT))
    return;

  double height = 0.;
  if (validateDouble_(parsed.stringValue(ShapeParameter::HEIGHT), "height", name, parsed, height) == 0)
    shape->setHeight(units.altitudeUnits().convertTo(simCore::Units::METERS, height));
}

void Parser::parseEllipticalOptional_(const ParsedShape& parsed, const std::string& name, const UnitsState& units, EllipticalShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return;
  }
  bool hasAngleStart = false;
  double angleStart = 0;
  if (parsed.hasValue(ShapeParameter::ANGLESTART))
  {
    if (validateDouble_(parsed.stringValue(ShapeParameter::ANGLESTART), "anglestart", name, parsed, angleStart) == 0)
    {
      angleStart = simCore::angFix2PI(units.angleUnits().convertTo(simCore::Units::RADIANS, angleStart));
      shape->setAngleStart(angleStart);
      hasAngleStart = true;
    }
  }
  // only bother with the angledeg and angleend if anglestart exists
  if (hasAngleStart)
  {
    if (parsed.hasValue(ShapeParameter::ANGLEDEG))
    {
      double angleSweep = 0.;
      if (validateDouble_(parsed.stringValue(ShapeParameter::ANGLEDEG), "angledeg", name, parsed, angleSweep) == 0)
      {
        if (angleSweep != 0.)
          shape->setAngleSweep(units.angleUnits().convertTo(simCore::Units::RADIANS, angleSweep));
        else
          printError_(parsed.filename(), parsed.lineNumber(), (name.empty() ? "" : "for " + name + " ") + "angledeg cannot be 0");
      }
    }
    if (parsed.hasValue(ShapeParameter::ANGLEEND))
    {
      double angleEnd = 0.;
      if (validateDouble_(parsed.stringValue(ShapeParameter::ANGLEEND), "angleend", name, parsed, angleEnd) == 0)
      {
        // convert to sweep, cannot cross 0 with angleend
        angleEnd = simCore::angFix2PI(units.angleUnits().convertTo(simCore::Units::RADIANS, angleEnd));
        if (angleEnd != angleStart)
          shape->setAngleSweep(angleEnd - angleStart);
        else
          printError_(parsed.filename(), parsed.lineNumber(), (name.empty() ? "" : "for " + name + " ") + "angleend cannot be the same as anglestart");
      }
    }
  }
  if (parsed.hasValue(ShapeParameter::MAJORAXIS))
  {
    double majorAxis = 0.;
    if (validateDouble_( parsed.stringValue(ShapeParameter::MAJORAXIS), "majoraxis", name, parsed, majorAxis) == 0)
      shape->setMajorAxis(units.rangeUnits().convertTo(simCore::Units::METERS, majorAxis));
  }
  if (parsed.hasValue(ShapeParameter::MINORAXIS))
  {
    double minorAxis = 0.;
    if (validateDouble_(parsed.stringValue(ShapeParameter::MINORAXIS), "minoraxis", name, parsed, minorAxis) == 0)
      shape->setMinorAxis(units.rangeUnits().convertTo(simCore::Units::METERS, minorAxis));
  }
}

int Parser::getColor_(const ParsedShape& parsed, ShapeParameter param, const std::string& shapeName, const std::string& fieldName, Color& color) const
{
  std::string colorStr = parsed.stringValue(param);
  uint32_t abgr;
  // try hex formatted string
  if (!simCore::isValidHexNumber(colorStr, abgr))
  {
    // try simple unsigned int string
    if (!simCore::isValidNumber(colorStr, abgr))
    {
      printError_(parsed.filename(), parsed.lineNumber(), "Invalid " + fieldName + ": " + colorStr + (shapeName.empty() ? "" : " for " + shapeName));
      return 1;
    }
  }
  // color value AABBGGRR
  color = Color(abgr & 0xff, (abgr >> 8) & 0xff, (abgr >> 16) & 0xff, (abgr >> 24) & 0xff);
  return 0;
}

int Parser::getPosition_(const PositionStrings & pos, bool relative, const UnitsState& units, simCore::Vec3 & position) const
{
  // require lat and lon, altitude is optional
  if (pos.x.empty() || pos.y.empty())
    return 1;
  if (relative)
  {
    double x = 0.;
    double y = 0.;
    double z = 0.;
    if (!simCore::isValidNumber(pos.x, x) || !simCore::isValidNumber(pos.y, y))
      return 1;
    simCore::isValidNumber(pos.z, z);

    // convert units
    x = units.rangeUnits().convertTo(simCore::Units::METERS, x);
    y = units.rangeUnits().convertTo(simCore::Units::METERS, y);
    z = units.altitudeUnits().convertTo(simCore::Units::METERS, z);
    position.set(x, y, z);
  }
  else
  {
    double altitude = 0.;
    simCore::isValidNumber(pos.z, altitude);
    // convert altitude units
    altitude = units.altitudeUnits().convertTo(simCore::Units::METERS, altitude);
    double lat = 0.;
    double lon = 0.;
    if (simCore::getAngleFromDegreeString(pos.x, true, lat) == 0 && simCore::getAngleFromDegreeString(pos.y, true, lon) == 0)
      position.set(lat, lon, altitude);
    else
      return 1;
  }
  return 0;
}

void Parser::printError_(const std::string& filename, size_t lineNumber, const std::string& errorText) const
{
  SIM_ERROR << "GOG: " << errorText << ", " << (!filename.empty() ? filename + " " : "") <<  "line: " << lineNumber << std::endl;
}


} }
