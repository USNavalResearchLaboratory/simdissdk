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
#include <iomanip>
#include <set>

#include "osgEarthAnnotation/LocalGeometryNode"

#include "simNotify/Notify.h"
#include "simCore/Common/Exception.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Mgrs.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Parser.h"
#include "simVis/GOG/Utils.h"
#include "simVis/GOG/ErrorHandler.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"

using namespace simVis;
using namespace simVis::GOG;
using namespace osgEarth;
using namespace osgEarth::Symbology;


namespace
{

/** Default Specialization of ErrorHandler that prints a generic message using SIM_WARN and SIM_ERROR */
class NotifyErrorHandler : public ErrorHandler
{
  virtual void printWarning(size_t lineNumber, const std::string& warningText)
  {
    SIM_WARN << "GOG warning: " << warningText << ", line: " << lineNumber << std::endl;
  }
  virtual void printError(size_t lineNumber, const std::string& errorText)
  {
    SIM_ERROR << "GOG error: " << errorText << ", line: " << lineNumber << std::endl;
  }
};

}

//------------------------------------------------------------------------

Parser::Parser(osgEarth::MapNode* mapNode) :
mapNode_(mapNode),
registry_(mapNode)
{
  context_.errorHandler_.reset(new NotifyErrorHandler);
  initGogColors_();
}

Parser::Parser(const GOGRegistry& reg) :
mapNode_(reg.getMapNode()),
registry_(reg)
{
  context_.errorHandler_.reset(new NotifyErrorHandler);
  initGogColors_();
}

void Parser::initGogColors_()
{
  colors_["color1"] = Color::Cyan;
  colors_["color2"] = Color::Red;
  colors_["color3"] = Color::Lime;
  colors_["color4"] = Color::Blue;
  colors_["color5"] = Color::Yellow;
  colors_["color6"] = Color::Orange;
  colors_["color7"] = Color::White;

  colors_["cyan"] = Color::Cyan;
  colors_["red"] = Color::Red;
  colors_["green"] = Color::Lime;
  colors_["blue"] = Color::Blue;
  colors_["yellow"] = Color::Yellow;
  colors_["orange"] = Color::Orange;
  colors_["white"] = Color::White;
  colors_["black"] = Color::Black;
  colors_["magenta"] = Color::Magenta;
}

std::string Parser::parseGogColor_(const std::string& c, bool isHex) const
{
  if (!isHex)
  {
    Color color;
    std::map<std::string, osgEarth::Symbology::Color>::const_iterator it = colors_.find(simCore::lowerCase(c));
    if (it != colors_.end())
      color = it->second;
    else
      color = Color::White;

    return color.toHTML();
  }
  else
  {
    // hex color: SIMDIS manual 4.7.3
    return Color(c, Color::ABGR).toHTML();
  }
}

std::string Parser::parseGogGeodeticAngle_(const std::string& input) const
{
  double angle;
  if (0 == simCore::getAngleFromDegreeString(input, false, angle))
  {
    std::ostringstream angleS;
    angleS << std::setprecision(12) << angle;
    return angleS.str();
  }
  return "0.0";
}

void Parser::addOverwriteColor(const std::string& key, osgEarth::Symbology::Color color)
{
  if (key.empty())
    return;

  colors_[simCore::lowerCase(key)] = color;
}

void Parser::setReferenceLocation(const simCore::Coordinate& refCoord)
{
  if (mapNode_.valid())
  {
    GeoPoint refPoint;
    simVis::convertCoordToGeoPoint(refCoord, refPoint, mapNode_->getMapSRS());
    context_.refPoint_ = refPoint;
  }
}

void Parser::setReferenceLocation(const osgEarth::GeoPoint& refPoint)
{
  context_.refPoint_ = refPoint;
}

GogNodeInterface* Parser::createGOG(const std::vector<std::string>& lines, const GOGNodeType& nodeType, GogFollowData& followData) const
{
  std::stringstream buf;
  for (std::vector<std::string>::const_iterator i = lines.begin(); i != lines.end(); ++i)
    buf << *i << "\n";

  std::istringstream input(buf.str());

  GogNodeInterface* result = NULL;
  OverlayNodeVector output;
  std::vector<GogFollowData> followDataVec;
  if (createGOGs(input, nodeType, output, followDataVec))
  {
    result = output.size() > 0 ? output.front() : NULL;
  }
  if (!followDataVec.empty())
    followData = followDataVec.front();
  return result;
}

bool Parser::parse(std::istream& input, Config& output, std::vector<GogMetaData>& metaData) const
{
  // Set up the modifier state object with default values. The state persists
  // across the parsing of the GOG input, spanning actual objects. (e.g. if the
  // line color is set within the scope of one object, that value remains active
  // for future objects until it is set again.)
  ModifierState state;
  state.lineColor_ = parseGogColor_("red", false);

  // create a set of keywords not handled explicitly by the parser, to filter out of the metadata
  // since this data is captured in the Style
  std::set<std::string> unhandledStyleKeywords;
  unhandledStyleKeywords.insert("innerradius");

  // relative shapes will store their metadata differently, and need a metadata flag to indicate they are relative
  bool relative = false;
  // valid commands must occur within a start/end block
  bool validStartEndBlock = false;

  Config current;
  std::string line;
  GogMetaData currentMetaData;
  currentMetaData.shape = GOG_UNKNOWN;
  currentMetaData.clearSetFields();
  // reference origin settings within a start/end block
  std::string refOriginLine;
  std::string refLat;
  std::string refLon;
  std::string refAlt;
  // cache the position lines in case they need to be stored to metadata, for annotations
  std::string positionLines;
  // track line number parsed for error reporting
  size_t lineNumber = 0;

  // parse each line from the stream individually
  while (simCore::getStrippedLine(input, line))
  {
    ++lineNumber;
    StringVector tokens;
    StringTokenizer tokenizer;
    tokenizer.addDelims(" \t");
    tokenizer.keepEmpties() = false;
    // keep the quotes to protect quoted token from lower-casing below
    tokenizer.addQuotes("'\"", true);
    tokenizer.tokenize(line, tokens);

    // convert tokens to lower case (unless it's in quotes or commented)
    for (StringVector::iterator j = tokens.begin(); j != tokens.end(); ++j)
    {
      std::string& token = *j;
      if (!startsWith(token, "\"") && !startsWith(token, "#") && !startsWith(token, "//"))
      {
        token = toLower(token);
        // stop further lower case conversion on text based values
        if (token == "annotation" || token == "comment" || token == "name")
          break;
      }
    }
    // rewrite the line now that it's lowered.
    line = osgEarth::joinStrings(tokens, ' ');

    if (tokens.empty())
    {
      // skip empty line
      continue;
    }

    // determine if the command is within a valid start/end block
    // acceptable commands are: comments, start and version
    if (!validStartEndBlock &&
      (tokens[0] != "comment" && !startsWith(tokens[0], "#") && !startsWith(tokens[0], "//") && tokens[0] != "start" && tokens[0] != "version"))
    {
      std::stringstream errorText;
      errorText << "token \"" << tokens[0] << "\" detected outside of a valid start/end block";
      printError_(lineNumber, errorText.str());
      // skip command
      continue;
    }

    if (tokens[0] == "comment" || startsWith(tokens[0], "#") || startsWith(tokens[0], "//"))
    {
      // NOTE: this will only store comments within a start/end block
      currentMetaData.metadata += line + "\n";
    }
    else if (tokens[0] == "version")
    {
      // NOTE: currently there is no error checking based on version #
      currentMetaData.metadata += line + "\n";
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
      if (tokens[0] == "end" && currentMetaData.shape == GOG::GOG_UNKNOWN)
      {
        printError_(lineNumber, "end command encountered before recognized GOG shape type keyword");
        continue;
      }

      // apply all cached information to metadata when end is reached
      if (tokens[0] == "end")
      {
        updateMetaData_(state, refOriginLine, positionLines, relative, currentMetaData);
        metaData.push_back(currentMetaData);
        state.apply(current);
        output.add(current);
      }

      // clear reference origin settings for new block of commands
      refOriginLine.clear();
      refLat.clear();
      refLon.clear();
      refAlt.clear();
      positionLines.clear();
      relative = false;

      // "start" indicates a valid block, "end" indicates the block of commands are complete and subsequent commands will be invalid
      validStartEndBlock = (tokens[0] == "start");
      currentMetaData.metadata.clear();
      currentMetaData.shape = GOG_UNKNOWN;
      currentMetaData.clearSetFields();
      current = Config();
      current.set("linenumber", lineNumber);  // Save the line number in the configuration
      state = ModifierState();
      state.lineColor_ = parseGogColor_("red", false);
    }
    else if (tokens[0] == "annotation")
    {
      if (tokens.size() >= 2)
      {
        // special case: annotations. you can have multiple annotations within
        // a single start/end block.
        if (current.key() == "annotation")
        {
          updateMetaData_(state, refOriginLine, positionLines, relative, currentMetaData);
          metaData.push_back(currentMetaData);
          currentMetaData.metadata.clear();
          positionLines.clear();
          relative = false;
          currentMetaData.shape = GOG_UNKNOWN;
          currentMetaData.clearSetFields();
          state.apply(current);
          output.add(current);
          current = Config();
          // if available, recreate reference origin
          // values are needed for subsequent annotation points since meta data was cleared and a new "current" is used
          if (!refOriginLine.empty())
          {
            current.set("lat", refLat);
            current.set("lon", refLon);
            if (!refAlt.empty())
            {
              current.set("alt", refAlt);
            }
          }
        }
        currentMetaData.metadata += line + "\n";
        currentMetaData.shape = GOG_ANNOTATION;
        current.key() = "annotation";
        const std::string textToken = osgEarth::trim(line.substr(tokens[0].length() + 1));
        // Store the un-decoded text in textToken to avoid problems with trim in osgEarth code. (SIMDIS-2875)
        current.add("text", textToken);
        // add support to show annotation text in dialog
        current.add("3d name", Utils::decodeAnnotation(textToken));
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
      tokens[0] == "linesegs"
      )
    {
      currentMetaData.shape = Parser::getShapeFromKeyword(tokens[0]);
      current.key() = line;
    }
    else if (tokens[0] == "latlonaltbox")
    {
      if (tokens.size() > 5)
      {
        currentMetaData.shape = Parser::getShapeFromKeyword(tokens[0]);
        currentMetaData.metadata += line + "\n";
        current.key() = "latlonaltbox";
        current.add("n", tokens[1]);
        current.add("s", tokens[2]);
        current.add("w", tokens[3]);
        current.add("e", tokens[4]);
        current.add("minalt", tokens[5]);
        if (tokens.size() > 6)
          current.add("maxalt", tokens[6]);
      }
      else
      {
        printError_(lineNumber, "latlonaltbox command requires at least 5 arguments");
      }
    }
    // arguments
    else if (tokens[0] == "off")
    {
      current.set("draw", "false");
    }
    else if (tokens[0] == "ref" || tokens[0] == "referencepoint")
    {
      if (tokens.size() >= 3)
      {

        // cache reference origin line and values for repeated use by GOG objects within a start/end block, such as annotations
        refOriginLine = line;

        refLat = parseGogGeodeticAngle_(tokens[1]);
        current.set("lat", refLat);
        refLon = parseGogGeodeticAngle_(tokens[2]);
        current.set("lon", refLon);

        if (tokens.size() >= 4)
        {
          refAlt = tokens[3];
          current.set("alt", refAlt);
        }
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
        // need to cache xyz for annotations
        positionLines += line + "\n";
        relative = true;

        Config point("xy");
        point.set("x", tokens[1]);
        point.set("y", tokens[2]);
        if (tokens.size() >= 4)
          point.set("z", tokens[3]);
        current.add(point);
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
        // need to save lla for annotations
        positionLines += line + "\n";

        Config point("ll");
        point.set("lat", parseGogGeodeticAngle_(tokens[1]));
        point.set("lon", parseGogGeodeticAngle_(tokens[2]));
        if (tokens.size() >= 4)
          point.set("alt", tokens[3]);
        current.add(point);
      }
      else
      {
        printError_(lineNumber, "ll/lla/latlon command requires at least 2 arguments");
      }
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
          // need to save lla for annotations
          positionLines += line + "\n";

          Config point("ll");
          point.set("lat", simCore::buildString("", lat * simCore::RAD2DEG));
          point.set("lon", simCore::buildString("", lon * simCore::RAD2DEG));
          if (tokens.size() >= 3)
            point.set("alt", tokens[2]);
          current.add(point);
        }
      }
      else
      {
        printError_(lineNumber, "mgrs command requires at least 2 arguments");
      }
    }
    else if (tokens[0] == "centerxy" || tokens[0] == "centerxyz")
    {
      if (tokens.size() >= 3)
      {
        currentMetaData.metadata += line + "\n";
        Config point("centerxy");
        point.set("x", tokens[1]);
        point.set("y", tokens[2]);
        if (tokens.size() >= 4)
          point.set("z", tokens[3]);
        current.add(point);
      }
      else
      {
        printError_(lineNumber, "centerxy/centerxyz command requires at least 2 arguments");
      }
    }
    else if (tokens[0] == "centerll" || tokens[0] == "centerlla" || tokens[0] == "centerlatlon")
    {
      if (tokens.size() >= 3)
      {
        currentMetaData.metadata += line + "\n";
        Config point("centerll");
        point.set("lat", parseGogGeodeticAngle_(tokens[1]));
        point.set("lon", parseGogGeodeticAngle_(tokens[2]));
        if (tokens.size() >= 4)
          point.set("alt", tokens[3]);
        current.add(point);
      }
      else
      {
        printError_(lineNumber, "centerll/centerlla/centerlatlon command requires at least 2 arguments");
      }
    }
    // persistent state modifiers:
    else if (tokens[0] == "linecolor")
    {
      if (tokens.size() == 2)
      {
        state.lineColor_ = parseGogColor_(tokens[1], false);
        currentMetaData.setExplicitly(GOG_LINE_COLOR_SET);
      }
      else if (tokens.size() == 3)
      {
        state.lineColor_ = parseGogColor_(tokens[2], true);
        currentMetaData.setExplicitly(GOG_LINE_COLOR_SET);
      }
      else
      {
        printError_(lineNumber, "linecolor command requires at least 1 argument");
      }
    }
    else if (tokens[0] == "fillcolor")
    {
      if (tokens.size() == 2)
      {
        state.fillColor_ = parseGogColor_(tokens[1], false);
        currentMetaData.setExplicitly(GOG_FILL_COLOR_SET);
      }
      else if (tokens.size() == 3)
      {
        state.fillColor_ = parseGogColor_(tokens[2], true);
        currentMetaData.setExplicitly(GOG_FILL_COLOR_SET);
      }
      else
      {
        printError_(lineNumber, "fillcolor command requires at least 1 argument");
      }
    }
    else if (tokens[0] == "linewidth")
    {
      if (tokens.size() >= 2)
      {
        state.lineWidth_ = tokens[1];
        currentMetaData.setExplicitly(GOG_LINE_WIDTH_SET);
      }
      else
        printError_(lineNumber, "linewidth command requires 1 argument");
     }
    else if (tokens[0] == "pointsize")
    {
      if (tokens.size() >= 2)
      {
        state.pointSize_ = tokens[1];
        currentMetaData.setExplicitly(GOG_POINT_SIZE_SET);
      }
      else
        printError_(lineNumber, "pointsize command requires 1 argument");
    }
    else if (tokens[0] == "altitudemode")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        state.altitudeMode_ = tokens[1];
      }
      else
      {
        printError_(lineNumber, "altitudemode command requires 1 argument");
      }
    }
    else if (tokens[0] == "altitudeunits")
    {
      if (tokens.size() >= 2)
      {
        state.altitudeUnits_ = tokens[1];
      }
      else
      {
        printError_(lineNumber, "altitudeunits command requires 1 argument");
      }
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
      {
        currentMetaData.metadata += line + "\n";
        state.timeUnits_ = tokens[1];
      }
      else
      {
        printError_(lineNumber, "timeunits command requires 1 argument");
      }
    }
    else if (tokens[0] == "angleunits")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        state.angleUnits_ = tokens[1];
      }
      else
      {
        printError_(lineNumber, "angleunits command requires 1 argument");
      }
    }
    else if (tokens[0] == "verticaldatum" && tokens.size() >= 2)
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        state.verticalDatum_ = tokens[1];
      }
      else
      {
        printError_(lineNumber, "verticaldatum command requires 1 argument");
      }
    }
    else if (tokens[0] == "priority")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        state.priority_ = tokens[1];
      }
      else
        printError_(lineNumber, "priority command requires 1 argument");
    }
    else if (tokens[0] == "filled")
    {
      current.set(tokens[0], "true");
    }
    else if (tokens[0] == "outline")
    {
      if (tokens.size() >= 2)
      {
        current.set(tokens[0], (tokens[1] == "true" ? "true" : "false"));
        currentMetaData.setExplicitly(GOG_OUTLINE_SET);
      }
      else
      {
        printError_(lineNumber, "outline command requires 1 argument");
      }
    }
    else if (startsWith(line, "3d billboard"))
    {
      // SIMDIS user manual claims this command is a singleton, however there are examples with boolean arguments in the public
      if (tokens.size() >= 3)
        current.set("3d billboard", (tokens[2] == "true" ? "true" : "false"));
      else
        current.set("3d billboard", "true");
    }
    else if (tokens[0] == "diameter")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        double value = as<double>(tokens[1], 1.0);
        current.set("radius", Stringify() << (value*0.5));
      }
      else
      {
        printError_(lineNumber, "diameter command requires 1 argument");
      }
    }
    else if (tokens[0] == "radius")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set("radius", tokens[1]);
      }
      else
      {
        printError_(lineNumber, "radius command requires 1 argument");
      }
    }
    else if (tokens[0] == "anglestart")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set(tokens[0], tokens[1]);
      }
      else
      {
        printError_(lineNumber, "anglestart command requires 1 argument");
      }
    }
    else if (tokens[0] == "angleend")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set(tokens[0], tokens[1]);
      }
      else
      {
        printError_(lineNumber, "angleend command requires 1 argument");
      }
    }
    else if (tokens[0] == "angledeg")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set(tokens[0], tokens[1]);
      }
      else
      {
        printError_(lineNumber, "angledeg command requires 1 argument");
      }
   }
    else if (tokens[0] == "majoraxis")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set(tokens[0], tokens[1]);
      }
      else
      {
        printError_(lineNumber, "majoraxis command requires 1 argument");
      }
    }
    else if (tokens[0] == "minoraxis")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set(tokens[0], tokens[1]);
      }
      else
      {
        printError_(lineNumber, "minoraxis command requires 1 argument");
      }
    }
    else if (tokens[0] == "semimajoraxis")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        double value = as<double>(tokens[1], 1.0);
        current.set("majoraxis", Stringify() << (value*2.0));
      }
      else
      {
        printError_(lineNumber, "semimajoraxis command requires 1 argument");
      }
    }
    else if (tokens[0] == "semiminoraxis")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        double value = as<double>(tokens[1], 1.0);
        current.set("minoraxis", Stringify() << (value*2.0));
      }
      else
      {
        printError_(lineNumber, "semiminoraxis command requires 1 argument");
      }
    }
    else if (tokens[0] == "scale")
    {
      if (tokens.size() >= 4)
      {
        currentMetaData.metadata += line + "\n";
        current.set("scalex", tokens[1]);
        current.set("scaley", tokens[2]);
        current.set("scalez", tokens[3]);
      }
      else
      {
        printError_(lineNumber, "scale command requires 3 arguments");
      }
    }
    else if (tokens[0] == "orient")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set("heading", tokens[1]);
        if (tokens.size() >= 3)
        {
          current.set("pitch", tokens[2]);
          if (tokens.size() >= 4)
          {
            current.set("roll", tokens[3]);
            current.set("orient", "cpr");
          }
          else
            current.set("orient", "cp");
        }
        else
          current.set("orient", "c");
      }
      else
      {
        printError_(lineNumber, "orient command requires at least 1 argument");
      }
    }
    else if (startsWith(line, "rotate"))
    {
      currentMetaData.metadata += line + "\n";
      current.set("3d follow", "cpr");
    }
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
        // name is the only value to store in meta data
        if (startsWith(line, "3d name"))
          currentMetaData.metadata += line + "\n";
        std::string tag = Stringify() << tokens[0] << " " << tokens[1];
        current.set(tag, line.substr(tag.length() + 1));
        if (startsWith(line, "3d offsetalt"))
          currentMetaData.setExplicitly(GOG_THREE_D_OFFSET_ALT_SET);
      }
      else
      {
        printError_(lineNumber, "3d command requires at least 2 arguments");
      }
    }
    else if (startsWith(line, "extrude"))
    {
      // stored in the style, not in meta data
      if (tokens.size() >= 2)
      {
        current.set(tokens[0], tokens[1]);
        currentMetaData.setExplicitly(GOG_EXTRUDE_SET);
        if (tokens.size() >= 3)
        {
          // handle optional extrude height
          current.set("extrudeheight", tokens[2]);
        }
      }
      else
      {
        printError_(lineNumber, "extrude command requires at least 1 argument");
      }
    }
    else if (tokens[0] == "height")
    {
      if (tokens.size() >= 2)
      {
        currentMetaData.metadata += line + "\n";
        current.set(tokens[0], tokens[1]);
      }
      else
      {
        printError_(lineNumber, "height command requires 1 argument");
      }
    }
    else if (tokens[0] == "tessellate")
    {
      current.set(tokens[0], tokens[1]);
      currentMetaData.setExplicitly(GOG_TESSELLATE_SET);
    }
    else if (tokens[0] == "lineprojection")
    {
      current.set(tokens[0], tokens[1]);
      currentMetaData.setExplicitly(GOG_LINE_PROJECTION_SET);
    }
    else if (tokens[0] == "linestyle")
    {
      current.set(tokens[0], tokens[1]);
      currentMetaData.setExplicitly(GOG_LINE_STYLE_SET);
    }
    else if (tokens[0] == "depthbuffer")
    {
      current.set(tokens[0], tokens[1]);
      currentMetaData.setExplicitly(GOG_DEPTH_BUFFER_SET);
    }
    else if (tokens[0] == "fontname")
    {
      current.set(tokens[0], tokens[1]);
      currentMetaData.setExplicitly(GOG_FONT_NAME_SET);
    }
    else if (tokens[0] == "fontsize")
    {
      current.set(tokens[0], tokens[1]);
      currentMetaData.setExplicitly(GOG_FONT_SIZE_SET);
    }
    else // treat everything as a name/value pair
    {
      if (!tokens.empty())
      {
        // filter out items that are stored in the Style
        if (unhandledStyleKeywords.find(tokens[0]) == unhandledStyleKeywords.end())
        {
          currentMetaData.metadata += line + "\n";
          // NOTE: to prevent warnings for actual commands, commands should be added to if/else check
          SIM_WARN << "Unknown GOG command " << tokens[0] << " found on line " << lineNumber << std::endl;
        }
        // Store the command anyways
        current.set(tokens[0], (tokens.size() == 1) ? std::string() : tokens[1]);
      }
    }
  }

  return true;
}

void Parser::updateMetaData_(const ModifierState& state, const std::string& refOriginLine, const std::string& positionLines, bool relative, GogMetaData& currentMetaData) const
{
  // some shapes don't store position in the metadata, such as polygon, line, points, linesegs, since
  // this data can be extracted from the osg::Node directly
  bool noGeometryInMetadata = Utils::canSerializeGeometry_(currentMetaData.shape);

  if (noGeometryInMetadata)
  {
    if (relative)
    {
      // if this is a relative shape, and we aren't storing the geometry in metadata, need to flag the metadata
      if (currentMetaData.metadata.find(RelativeShapeKeyword) == std::string::npos)
        currentMetaData.metadata += RelativeShapeKeyword + "\n";
      // indicate this is a relative shape with a reference point which can be extracted from the node's geometry rather than being stored in meta data
      if (currentMetaData.metadata.find(ReferencePointKeyword) == std::string::npos)
        currentMetaData.metadata += ReferencePointKeyword + "\n";
    }
  }
  // add reference point to meta data for relative shapes that store their geometry in metadata
  else if (!refOriginLine.empty())
    currentMetaData.metadata += refOriginLine + "\n";

  // store altitude units in metadata for shapes with geometry stored in metadata
  if (!noGeometryInMetadata && !state.altitudeUnits_.value().empty())
    currentMetaData.metadata += "altitudeunits " + state.altitudeUnits_.value() + "\n";

  // store range units in meta data for shapes with geometry stored in metadata
  if (!noGeometryInMetadata && !state.rangeUnits_.value().empty())
    currentMetaData.metadata += "rangeunits " + state.rangeUnits_.value() + "\n";

  // add position lines to metadata for annotations
  if (currentMetaData.shape == GOG_ANNOTATION && !positionLines.empty())
      currentMetaData.metadata += positionLines;
}

bool Parser::createGOGs_(const Config& conf, const GOGNodeType& nodeType, const std::vector<GogMetaData>& metaData, OverlayNodeVector& output, std::vector<GogFollowData>& followData) const
{
  // add exception handling prior to passing data to renderer
  SAFETRYBEGIN;
  const ConfigSet& objects = conf.children();

  size_t index = 0;
  for (ConfigSet::const_iterator i = objects.begin(); i != objects.end(); ++i)
  {
    const Config& conf = *i;

    GogFollowData follow;
    // make sure the lists are parallel, assert if they are not
    assert(index < metaData.size());
    GogNodeInterface* node = registry_.createGOG(conf, nodeType, style_, context_, metaData[index], follow);

    if (node)
    {
      // update draw
      node->setDrawState(conf.value<bool>("draw", true));
      output.push_back(node);
      followData.push_back(follow);

      // turn off lighting
      if (node->osgNode())
        simVis::setLighting(node->osgNode()->getOrCreateStateSet(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
    }
    index++;
  }
  return true;
  // provide exception notification, if something went awry
  SAFETRYEND("creating GOG");
  return false;
}

bool Parser::createGOGs(std::istream& input, const GOGNodeType& nodeType, OverlayNodeVector& output, std::vector<GogFollowData>& followData) const
{
  // first, parse from GOG into Config
  Config conf;
  std::vector<GogMetaData> metaData;
  if (!parse(input, conf, metaData))
    return false;

  // then parse from Config into Annotation.
  return createGOGs_(conf, nodeType, metaData, output, followData);
}

GogShape Parser::getShapeFromKeyword(const std::string& keyword)
{
  if (keyword == "annotation")
    return GOG_ANNOTATION;
  if (keyword == "circle")
    return GOG_CIRCLE;
  if (keyword == "ellipse")
    return GOG_ELLIPSE;
  if (keyword == "arc")
    return GOG_ARC;
  if (keyword == "cylinder")
    return GOG_CYLINDER;
  if (keyword == "hemisphere")
    return GOG_HEMISPHERE;
  if (keyword == "sphere")
    return GOG_SPHERE;
  if (keyword =="ellipsoid")
    return GOG_ELLIPSOID;
  if (keyword == "points")
    return GOG_POINTS;
  if (keyword == "line")
    return GOG_LINE;
  if (keyword == "poly")
    return GOG_POLYGON;
  if (keyword =="polygon")
    return GOG_POLYGON;
  if (keyword == "linesegs")
    return GOG_LINESEGS;
  if (keyword == "latlonaltbox")
    return GOG_LATLONALTBOX;
  return GOG_UNKNOWN;
}

std::string Parser::getKeywordFromShape(GogShape shape)
{
  switch (shape)
  {
  case GOG_ANNOTATION:
    return "annotation";
  case GOG_CIRCLE:
    return "circle";
  case GOG_ELLIPSE:
    return "ellipse";
  case GOG_ELLIPSOID:
    return "ellipsoid";
  case GOG_ARC:
    return "arc";
  case GOG_CYLINDER:
    return "cylinder";
  case GOG_HEMISPHERE:
    return "hemisphere";
  case GOG_SPHERE:
    return "sphere";
  case GOG_POINTS:
    return "points";
  case GOG_LINE:
    return "line";
  case GOG_POLYGON:
    return "polygon";
  case GOG_LINESEGS:
    return "linesegs";
  case GOG_LATLONALTBOX:
    return "latlonaltbox";
  case GOG_UNKNOWN:
    return "";
  }
  return "";
}

bool Parser::loadGOGs(std::istream& input, const GOGNodeType& nodeType, OverlayNodeVector& output, std::vector<GogFollowData>& followData) const
{
  return createGOGs(input, nodeType, output, followData);
}

void Parser::printError_(size_t lineNumber, const std::string& errorText) const
{
  // Assertion failure means Null Object pattern failed
  assert(context_.errorHandler_ != NULL);
  if (context_.errorHandler_)
    context_.errorHandler_->printError(lineNumber, errorText);
}

void Parser::setErrorHandler(std::shared_ptr<ErrorHandler> errorHandler)
{
  if (!errorHandler)
    context_.errorHandler_.reset(new NotifyErrorHandler);
  else
    context_.errorHandler_ = errorHandler;
}
