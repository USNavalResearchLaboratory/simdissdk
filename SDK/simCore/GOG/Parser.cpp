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
#include <iomanip>
#include <set>

#include "simNotify/Notify.h"
#include "simCore/Common/Exception.h"
#include "simCore/Common/Optional.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
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
}

Parser::~Parser()
{}

void Parser::initGogColors_()
{
  // GOG hex colors are AABBGGRR
  colors_["color1"] = "0xff0000ff"; // Cyan
  colors_["color2"] = "0xff0000ff"; // Red
  colors_["color3"] = "Oxff00ff00"; // Lime
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

std::string Parser::parseGogColor_(const std::string& color, bool isHex) const
{
  if (!isHex)
  {
    auto it = colors_.find(simCore::lowerCase(color));
    if (it != colors_.end())
      return it->second;
    return "0xffffffff";
  }

  // append prefix if necessary
  std::string prefix = "0x";
  if (startsWith(color, prefix))
    prefix = "";

  return prefix + color;
}

void Parser::addOverwriteColor(const std::string& key, const std::string& color)
{
  if (key.empty())
    return;

  // append prefix if necessary
  std::string prefix = "0x";
  if (startsWith(color, prefix))
    prefix = "";

  colors_[simCore::lowerCase(key)] = prefix + color;
}

void Parser::setUnitsRegistry(const simCore::UnitsRegistry* registry)
{
  units_ = registry;
}

void Parser::parse(std::istream& input, std::vector<GogShapePtr>& output) const
{
  // Set up the modifier state object with default values. The state persists
  // across the parsing of the GOG input for annotations, spanning actual objects.
  // (e.g. if the line color is set within the scope of one annotation, that value
  // remains active for future annotations until it is set again.)
  ModifierState state;

  // create a set of keywords not handled explicitly by the parser
  std::set<std::string> unhandledKeywords;
  // not supported
  unhandledKeywords.insert("innerradius");
  // billboard is OBE, since all annotations are always billboarded
  unhandledKeywords.insert("3d billboard");
  // no checks on version
  unhandledKeywords.insert("version");

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
    for (std::string token : tokens)
    {
      if (token[0] != '"' && token[0] != '#' && token[0] !=  '/')
      {
        token = simCore::lowerCase(token);
        // stop further lower case conversion on text based values
        if (token == "annotation" || token == "comment" || token == "name")
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
      printError_(lineNumber, errorText.str());
      // skip command
      continue;
    }

    if (isComment_(tokens[0]))
    {
      // NOTE: this will only store comments within a start/end block
      current.addComment(line);

      // process special KML icon comment keywords
      if (tokens.size () > 2 && tokens[1] == "kml_icon")
        current.set(ShapeParameter::ICON, tokens[2]);
      if (tokens.size() > 1 && tokens[1] == "kml_groundoverlay")
        current.setShape(GogShape::ShapeType::IMAGEOVERLAY);
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
        printError_(lineNumber, "nested start command not allowed");
        continue;
      }
      if (!validStartEndBlock && tokens[0] == "end")
      {
        printError_(lineNumber, "end command encountered before start");
        continue;
      }
      if (tokens[0] == "end" && current.shape() == GogShape::ShapeType::UNKNOWN)
      {
        printError_(lineNumber, "end command encountered before recognized GOG shape type keyword");
        continue;
      }

      // apply all cached information to metadata when end is reached, only if shape is valid
      if (tokens[0] == "end" && !invalidShape)
      {
        if (current.pointType() == ParsedShape::LLA)
          current.set(ShapeParameter::ABSOLUTE_POINTS, "1");
        state.apply(current);
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
        if (current.shape() == GogShape::ShapeType::ANNOTATION)
        {
          state.apply(current);
          GogShapePtr gog = getShape_(current);
          if (gog)
            output.push_back(gog);
          current.reset();
          // if available, recreate reference origin
          // values are needed for subsequent annotation points since meta data was cleared and a new "current" is used
          if (refLla.has_value())
            current.set(ShapeParameter::CENTERXY, refLla.value_or(PositionStrings()));
        }
        if (current.shape() != GogShape::ShapeType::UNKNOWN)
        {
          SIM_WARN << "Multiple shape keywords found in single start/end block\n";
          invalidShape = true;
        }
        current.setShape(GogShape::ShapeType::ANNOTATION);
        std::string textToken = simCore::StringUtils::trim(line.substr(tokens[0].length() + 1));
        // Store the un-decoded text in textToken
        current.set(ShapeParameter::TEXT, textToken);
        // clean up text for the shape name
        textToken = simCore::StringUtils::substitute(textToken, "_", " ");
        textToken = simCore::StringUtils::substitute(textToken, "\\n", "\n");
        current.set(ShapeParameter::NAME, textToken);
      }
      else
      {
        printError_(lineNumber, "annotation command requires at least 1 argument");
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
      if (current.shape() != GogShape::ShapeType::UNKNOWN)
      {
        SIM_WARN << "Multiple shape keywords found in single start/end block\n";
        invalidShape = true;
      }
      current.setShape(GogShape::stringToShapeType(tokens[0]));
    }
    else if (tokens[0] == "latlonaltbox")
    {
      if (tokens.size() > 5)
      {
        if (current.shape() != GogShape::ShapeType::UNKNOWN)
        {
          SIM_WARN << "Multiple shape keywords found in single start/end block\n";
          invalidShape = true;
        }
        current.setShape(GogShape::ShapeType::LATLONALTBOX);
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
        printError_(lineNumber, "latlonaltbox command requires at least 5 arguments");
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
        printError_(lineNumber, "ref/referencepoint command requires at least 2 arguments");
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
        printError_(lineNumber, "xy/xyz command requires at least 2 arguments");
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
        printError_(lineNumber, "ll/lla/latlon command requires at least 2 arguments");
    }
    else if (tokens[0] == "mgrs")
    {
      if (tokens.size() >= 2)
      {
        double lat;
        double lon;
        if (simCore::Mgrs::convertMgrsToGeodetic(tokens[1], lat, lon) != 0)
          printError_(lineNumber, "Unable to convert MGRS coordinate to lat/lon");
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
        printError_(lineNumber, "mgrs command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerxy" || tokens[0] == "centerxyz")
    {
      if (tokens.size() >= 3)
      {
        if (tokens.size() >= 4)
          current.set(ShapeParameter::CENTERXY, PositionStrings(tokens[1], tokens[2], tokens[3]));
        else
          current.set(ShapeParameter::CENTERXY, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(lineNumber, "centerxy/centerxyz command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerxy2")
    {
      if (tokens.size() >= 3)
        current.set(ShapeParameter::CENTERXY2, PositionStrings(tokens[1], tokens[2]));
      else
        printError_(lineNumber, "centerxy2 command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerll" || tokens[0] == "centerlla" || tokens[0] == "centerlatlon")
    {
      if (tokens.size() >= 3)
      {
        if (tokens.size() >= 4)
          current.set(ShapeParameter::CENTERLL, PositionStrings(tokens[1], tokens[2], tokens[3]));
        else
          current.set(ShapeParameter::CENTERLL, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(lineNumber, "centerll/centerlla/centerlatlon command requires at least 2 arguments");
    }
    else if (tokens[0] == "centerll2" || tokens[0] == "centerlatlon2")
    {
      if (tokens.size() >= 3)
      {
        // note centerll2 only supports lat and lon, altitude for shape must be derived from first center point
        current.set(ShapeParameter::CENTERLL2, PositionStrings(tokens[1], tokens[2]));
      }
      else
        printError_(lineNumber, "centerll2 command requires at least 2 arguments");
    }
    // persistent state modifiers:
    else if (tokens[0] == "linecolor")
    {
      if (tokens.size() == 2)
        state.lineColor_ = parseGogColor_(tokens[1], false);
      else if (tokens.size() == 3)
        state.lineColor_ = parseGogColor_(tokens[2], true);
      else
        printError_(lineNumber, "linecolor command requires at least 1 argument");
    }
    else if (tokens[0] == "fillcolor")
    {
      if (tokens.size() == 2)
        state.fillColor_ = parseGogColor_(tokens[1], false);
      else if (tokens.size() == 3)
        state.fillColor_ = parseGogColor_(tokens[2], true);
      else
        printError_(lineNumber, "fillcolor command requires at least 1 argument");
    }
    else if (tokens[0] == "linewidth")
    {
      if (tokens.size() >= 2)
        state.lineWidth_ = tokens[1];
      else
        printError_(lineNumber, "linewidth command requires 1 argument");
     }
    else if (tokens[0] == "pointsize")
    {
      if (tokens.size() >= 2)
        state.pointSize_ = tokens[1];
      else
        printError_(lineNumber, "pointsize command requires 1 argument");
    }
    else if (tokens[0] == "altitudemode")
    {
      if (tokens.size() >= 2)
        state.altitudeMode_ = tokens[1];
      else
        printError_(lineNumber, "altitudemode command requires 1 argument");
    }
    else if (tokens[0] == "altitudeunits")
    {
      if (tokens.size() >= 2)
        state.altitudeUnits_ = tokens[1];
      else
        printError_(lineNumber, "altitudeunits command requires 1 argument");
    }
    else if (tokens[0] == "rangeunits")
    {
      if (tokens.size() >= 2)
        state.rangeUnits_ = tokens[1];
      else
        printError_(lineNumber, "rangeunits command requires 1 argument");
    }
    else if (tokens[0] == "timeunits")
    {
      if (tokens.size() >= 2)
        state.timeUnits_ = tokens[1];
      else
        printError_(lineNumber, "timeunits command requires 1 argument");
    }
    else if (tokens[0] == "angleunits")
    {
      if (tokens.size() >= 2)
        state.angleUnits_ = tokens[1];
      else
        printError_(lineNumber, "angleunits command requires 1 argument");
    }
    else if (tokens[0] == "verticaldatum")
    {
      if (tokens.size() >= 2)
        state.verticalDatum_ = tokens[1];
      else
        printError_(lineNumber, "verticaldatum command requires 1 argument");
    }
    else if (tokens[0] == "priority")
    {
      if (tokens.size() >= 2)
        state.priority_ = tokens[1];
      else
        printError_(lineNumber, "priority command requires 1 argument");
    }
    else if (tokens[0] == "filled")
    {
      current.set(ShapeParameter::FILLED, "true");
    }
    else if (tokens[0] == "outline")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::OUTLINE, (tokens[1] == "true" ? "true" : "false"));
      else
        printError_(lineNumber, "outline command requires 1 argument");
    }
    else if (tokens[0] == "textoutlinecolor")
    {
      if (tokens.size() == 2)
        state.textOutlineColor_ = parseGogColor_(tokens[1], false);
      else if (tokens.size() == 3)
        state.textOutlineColor_ = parseGogColor_(tokens[2], true);
      else
        printError_(lineNumber, "textoutlinecolor command requires at least 1 argument");
    }
    else if (tokens[0] == "textoutlinethickness")
    {
      if (tokens.size() >= 2)
        state.textOutlineThickness_ = tokens[1];
      else
        printError_(lineNumber, "textoutlinethickness command requires 1 argument");
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
        printError_(lineNumber, "diameter command requires 1 argument");
    }
    else if (tokens[0] == "radius")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::RADIUS, tokens[1]);
      else
        printError_(lineNumber, "radius command requires 1 argument");
    }
    else if (tokens[0] == "anglestart")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::ANGLESTART, tokens[1]);
      else
        printError_(lineNumber, "anglestart command requires 1 argument");
    }
    else if (tokens[0] == "angleend")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::ANGLEEND, tokens[1]);
      else
        printError_(lineNumber, "angleend command requires 1 argument");
    }
    else if (tokens[0] == "angledeg")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::ANGLEDEG, tokens[1]);
      else
        printError_(lineNumber, "angledeg command requires 1 argument");
   }
    else if (tokens[0] == "majoraxis")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::MAJORAXIS, tokens[1]);
      else
        printError_(lineNumber, "majoraxis command requires 1 argument");
    }
    else if (tokens[0] == "minoraxis")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::MINORAXIS, tokens[1]);
      else
        printError_(lineNumber, "minoraxis command requires 1 argument");
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
        printError_(lineNumber, "semimajoraxis command requires 1 argument");
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
        printError_(lineNumber, "semiminoraxis command requires 1 argument");
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
        printError_(lineNumber, "scale command requires 3 arguments");
    }
    else if (tokens[0] == "orient")
    {
      if (tokens.size() >= 2)
      {
        current.set(ShapeParameter::ORIENT_HEADING, tokens[1]);
        if (tokens.size() >= 3)
        {
          current.set(ShapeParameter::ORIENT_PITCH, tokens[2]);
          if (tokens.size() >= 4)
          {
            current.set(ShapeParameter::ORIENT_ROLL, tokens[3]);
            current.set(ShapeParameter::ORIENT, "cpr"); // c=heading(course), p=pitch, r=roll
          }
          else
            current.set(ShapeParameter::ORIENT, "cp"); // c=heading(course), p=pitch, r=roll
        }
        else
          current.set(ShapeParameter::ORIENT, "c");
      }
      else
        printError_(lineNumber, "orient command requires at least 1 argument");
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
        else if (tokens[1] == "offsetcourse")
          current.set(ShapeParameter::OFFSETCOURSE, restOfLine);
        else if (tokens[1] == "offsetpitch")
          current.set(ShapeParameter::OFFSETPITCH, restOfLine);
        else if (tokens[1] == "offsetroll")
          current.set(ShapeParameter::OFFSETROLL, restOfLine);
        else if (tokens[1] == "follow")
          current.set(ShapeParameter::FOLLOW, restOfLine);
      }
      else
        printError_(lineNumber, "3d command requires at least 2 arguments");
    }
    else if (startsWith(line, "extrude"))
    {
      // stored in the style, not in meta data
      if (tokens.size() >= 2)
      {
        current.set(ShapeParameter::EXTRUDE, tokens[1]);
        if (tokens.size() >= 3)
        {
          // handle optional extrude height
          current.set(ShapeParameter::EXTRUDE_HEIGHT, tokens[2]);
        }
      }
      else
        printError_(lineNumber, "extrude command requires at least 1 argument");
    }
    else if (tokens[0] == "height")
    {
      if (tokens.size() >= 2)
        current.set(ShapeParameter::HEIGHT, tokens[1]);
      else
        printError_(lineNumber, "height command requires 1 argument");
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
    else // treat everything as a name/value pair
    {
      if (!tokens.empty())
      {
        // filter out items that are explicitly unhandled
        if (unhandledKeywords.find(tokens[0]) == unhandledKeywords.end())
          printError_(lineNumber, "Found unknown GOG command " + tokens[0]);
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
  bool relative = !parsed.boolValue(ShapeParameter::ABSOLUTE_POINTS, true);
  std::string name = parsed.stringValue(ShapeParameter::NAME);
  if (name.empty())
    name = GogShape::shapeTypeToString(parsed.shape());
  switch (parsed.shape())
  {
  case GogShape::ShapeType::ANNOTATION:
  {
    simCore::Vec3 position;
    bool hasPosition;
    // verify shape has minimum required fields
    if (relative)
    {
      // relative annotation uses the xyz keyword for the center position, not the centerxy, so check for a position
      const std::vector<PositionStrings>& positions = parsed.positions();
      hasPosition = (!positions.empty() && getPosition_(positions.front(), relative, units, position) == 0);
    }
    else
      hasPosition = (parsed.hasValue(ShapeParameter::CENTERLL) && getPosition_(parsed.positionValue(ShapeParameter::CENTERLL), relative, units, position) == 0);

    if (!hasPosition)
    {
      printError_(parsed.lineNumber(), "Annotation " + name + " did not have a position, cannot create shape");
      break;
    }
    if (!parsed.hasValue(ShapeParameter::TEXT))
    {
      printError_(parsed.lineNumber(), "Annotation " + name + " missing text, cannot create shape");
      break;
    }

    Annotation* anno = new Annotation(relative);
    anno->setPosition(position);
    anno->setText(parsed.stringValue(ShapeParameter::TEXT));
    if (parsed.hasValue(ShapeParameter::FONTNAME))
      anno->setFontName(parsed.stringValue(ShapeParameter::FONTNAME));
    if (parsed.hasValue(ShapeParameter::TEXTSIZE))
    {
      int textSize = 0;
      if (simCore::isValidNumber(parsed.stringValue(ShapeParameter::TEXTSIZE), textSize))
        anno->setTextSize(textSize);
      else
        printError_(parsed.lineNumber(), "Invalid fontsize: " + parsed.stringValue(ShapeParameter::TEXTSIZE) + " for " + name);
    }
    if (parsed.hasValue(ShapeParameter::LINECOLOR))
    {
      GogShape::Color color;
      if (getColor_(parsed, ShapeParameter::LINECOLOR, name, "linecolor", color) == 0)
        anno->setTextColor(color);
    }
    if (parsed.hasValue(ShapeParameter::TEXTOUTLINETHICKNESS))
    {
      std::string thicknessStr = parsed.stringValue(ShapeParameter::TEXTOUTLINETHICKNESS);
      Annotation::OutlineThickness thickness = Annotation::OutlineThickness::NONE;
      bool valid = true;
      if (thicknessStr == "thick")
        thickness = Annotation::OutlineThickness::THICK;
      else if (thicknessStr == "thin")
        thickness = Annotation::OutlineThickness::THIN;
      else if (thicknessStr != "none")
      {
        valid = false;
        printError_(parsed.lineNumber(), "Invalid textoutlinethickness: " + thicknessStr + " for " + name);
      }
      if (valid)
        anno->setOutlineThickness(thickness);
    }
    if (parsed.hasValue(ShapeParameter::TEXTOUTLINECOLOR))
    {
      GogShape::Color color;
      if (getColor_(parsed, ShapeParameter::TEXTOUTLINECOLOR, name, "textoutlinecolor", color) == 0)
        anno->setOutlineColor(color);
    }
    rv.reset(anno);
    break;
  }
  case GogShape::ShapeType::CIRCLE:
  {
    std::unique_ptr<Circle> circle(new Circle(relative));
    if (parseCircular_(parsed, relative, name, units, circle.get()) == 0)
      rv.reset(circle.release());
    break;
  }
  case GogShape::ShapeType::LINE:
  {
    std::unique_ptr<Line> line(new Line(relative));
    if (parsePointBased_(parsed, relative, name, units, 2, line.get()) == 0)
      rv.reset(line.release());
    break;
  }
  case GogShape::ShapeType::LINESEGS:
  {
    std::unique_ptr<LineSegs> line(new LineSegs(relative));
    if (parsePointBased_(parsed, relative, name, units, 2, line.get()) == 0)
      rv.reset(line.release());
    break;
  }
  case GogShape::ShapeType::POLYGON:
  {
    std::unique_ptr<Polygon> poly(new Polygon(relative));
    if (parsePointBased_(parsed, relative, name, units, 3, poly.get()) == 0)
      rv.reset(poly.release());
  }
  break;
  case GogShape::ShapeType::SPHERE:
  {
    std::unique_ptr<Sphere> sphere(new Sphere(relative));
    if (parseCircular_(parsed, relative, name, units, sphere.get()) == 0)
      rv.reset(sphere.release());
  }
  break;
  case GogShape::ShapeType::HEMISPHERE:
  {
    std::unique_ptr<Hemisphere> hemi(new Hemisphere(relative));
    if (parseCircular_(parsed, relative, name, units, hemi.get()) == 0)
      rv.reset(hemi.release());
    break;
  }
  case GogShape::ShapeType::ORBIT:
  {
    std::unique_ptr<Orbit> orbit(new Orbit(relative));
    if (parseCircular_(parsed, relative, name, units, orbit.get()) == 0)
    {
      simCore::Vec3 center2;
      ShapeParameter param = (relative ? ShapeParameter::CENTERXY2 : ShapeParameter::CENTERLL2);
      // verify orbit has required center2 field
      if (parsed.hasValue(param) && getPosition_(parsed.positionValue(param), relative, units, center2) == 0)
      {
        orbit->setCenterPosition2(center2);
        rv.reset(orbit.release());
      }
      else
        printError_(parsed.lineNumber(), "orbit " + name + " missing or invalid center point 2, cannot create shape");
    }
    break;
  }
  case GogShape::ShapeType::CONE:
  {
    std::unique_ptr<Cone> cone(new Cone(relative));
    if (parseCircular_(parsed, relative, name, units, cone.get()) == 0)
    {
      parseCircularHeightOptional_(parsed, name, units, cone.get());
      rv.reset(cone.release());
    }
    break;
  }
  // TODO: other shapes
  case GogShape::ShapeType::ARC:
  case GogShape::ShapeType::CYLINDER:
  case GogShape::ShapeType::ELLIPSE:
  case GogShape::ShapeType::ELLIPSOID:
  case GogShape::ShapeType::IMAGEOVERLAY:
  case GogShape::ShapeType::LATLONALTBOX:
  case GogShape::ShapeType::POINTS:
  case GogShape::ShapeType::UNKNOWN:
    break;
  }
  if (!rv)
    return rv;

  // TODO: general options

  return rv;
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
    GogShape::Color color;
    if (getColor_(parsed, ShapeParameter::LINECOLOR, name, "linecolor", color) == 0)
      shape->setLineColor(color);
  }
  if (parsed.hasValue(ShapeParameter::LINESTYLE))
  {
    std::string styleStr = parsed.stringValue(ShapeParameter::LINESTYLE);
    bool valid = true;
    FillableShape::LineStyle style = FillableShape::LineStyle::SOLID;
    if (styleStr == "dashed")
      style = FillableShape::LineStyle::DASHED;
    else if (styleStr == "dotted")
      style = FillableShape::LineStyle::DOTTED;
    else if (styleStr != "solid")
    {
      printError_(parsed.lineNumber(), "Invalid linestyle: " + styleStr + " for " + name);
      valid = false;
    }
    if (valid)
      shape->setLineStyle(style);
  }
  if (parsed.hasValue(ShapeParameter::LINEWIDTH))
  {
    int lineWidth = 0;
    std::string lineWidthStr = parsed.stringValue(ShapeParameter::LINEWIDTH);
    if (simCore::isValidNumber(lineWidthStr, lineWidth))
      shape->setLineWidth(lineWidth);
    else
      printError_(parsed.lineNumber(), "Invalid linewidth: " + lineWidthStr + " for " + name);
  }
  if (parsed.hasValue(ShapeParameter::FILLED))
    shape->setFilled(parsed.boolValue(ShapeParameter::FILLED, true));
  if (parsed.hasValue(ShapeParameter::FILLCOLOR))
  {
    GogShape::Color color;
    if (getColor_(parsed, ShapeParameter::FILLCOLOR, name, "fillcolor", color) == 0)
      shape->setFillColor(color);
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
    printError_(parsed.lineNumber(), shapeTypeName + " " + name + " has no points, cannot create shape");
    return 1;
  }
  else if (positions.size() < minimumNumPoints)
  {
    printError_(parsed.lineNumber(), shapeTypeName + " " + name + " has less than the required number of points, cannot create shape");
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
    printError_(parsed.lineNumber(), shapeTypeName + " " + name + " has no valid points, cannot create shape");
    return 1;
  }
  else if (shape->points().size() < minimumNumPoints)
  {
    printError_(parsed.lineNumber(), shapeTypeName + " " + name + " has less than the required number of valid points, cannot create shape");
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
  // if tessellate is set, default to RHUMBLINE unless LINEPROJECTION specifies otherwise
  PointBasedShape::TessellationStyle style = PointBasedShape::TessellationStyle::RHUMBLINE;
  if (parsed.hasValue(ShapeParameter::LINEPROJECTION))
  {
    std::string projectionStr = parsed.stringValue(ShapeParameter::LINEPROJECTION);
    if (projectionStr == "greatcircle")
      style = PointBasedShape::TessellationStyle::GREAT_CIRCLE;
  }
  shape->setTesssellation(style);
}

int Parser::parseCircular_(const ParsedShape& parsed, bool relative, const std::string& name, const UnitsState& units, CircularShape* shape) const
{
  simCore::Vec3 position;
  ShapeParameter param = (relative ? ShapeParameter::CENTERXY : ShapeParameter::CENTERLL);
  // verify shape has minimum required fields
  if (!parsed.hasValue(param) || getPosition_(parsed.positionValue(param), relative, units, position) != 0)
  {
    printError_(parsed.lineNumber(), GogShape::shapeTypeToString(shape->shapeType()) + " " + name + " missing or invalid center point, cannot create shape");
    return 1;
  }
  shape->setCenterPosition(position);
  parseCircularOptional_(parsed, name, units, shape);
  return 0;
}

void Parser::parseCircularOptional_(const ParsedShape& parsed, const std::string& name, const UnitsState& units, CircularShape* shape) const
{
  if (!shape)
  {
    assert(0); // should not be called with NULL
    return;
  }
  parseFillable_(parsed, name, shape);
  if (!parsed.hasValue(ShapeParameter::RADIUS))
    return;
  double radius = 0.;
  std::string radiusStr = parsed.stringValue(ShapeParameter::RADIUS);
  if (simCore::isValidNumber(radiusStr, radius))
    shape->setRadius(units.rangeUnits_.convertTo(simCore::Units::METERS, radius));
  else
    printError_(parsed.lineNumber(), "Invalid radius: " + radiusStr + " for " + name);
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
  std::string heightStr = parsed.stringValue(ShapeParameter::HEIGHT);
  if (simCore::isValidNumber(heightStr, height))
    shape->setHeight(units.rangeUnits_.convertTo(simCore::Units::METERS, height));
  else
    printError_(parsed.lineNumber(), "Invalid height: " + heightStr + " for " + name);
}

int Parser::getColor_(const ParsedShape& parsed, ShapeParameter param, const std::string& shapeName, const std::string& fieldName, GogShape::Color& color) const
{
  std::string colorStr = parsed.stringValue(param);
  uint32_t abgr;
  if (!simCore::isValidHexNumber(colorStr, abgr))
  {
    printError_(parsed.lineNumber(), "Invalid " + fieldName + ": " + colorStr + " for " + shapeName);
    return 1;
  }
  color = GogShape::Color(abgr & 0xff, (abgr >> 8) & 0xff, (abgr >> 16) & 0xff, (abgr >> 24) & 0xff);
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
    x = units.rangeUnits_.convertTo(simCore::Units::METERS, x);
    y = units.rangeUnits_.convertTo(simCore::Units::METERS, y);
    z = units.altitudeUnits_.convertTo(simCore::Units::METERS, z);
    position.set(x, y, z);
  }
  else
  {
    double altitude = 0.;
    simCore::isValidNumber(pos.z, altitude);
    // convert altitude units
    altitude = units.altitudeUnits_.convertTo(simCore::Units::METERS, altitude);
    double lat = 0.;
    double lon = 0.;
    if (simCore::getAngleFromDegreeString(pos.x, true, lat) == 0 && simCore::getAngleFromDegreeString(pos.y, true, lon) == 0)
      position.set(lat, lon, altitude);
    else
      return 1;
  }
  return 0;
}

void Parser::printError_(size_t lineNumber, const std::string& errorText) const
{
  SIM_ERROR << "GOG error: " << errorText << ", line: " << lineNumber << std::endl;
}


} }
