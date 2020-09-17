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
#ifndef SIMVIS_GOG_PARSEDSHAPE_H
#define SIMVIS_GOG_PARSEDSHAPE_H

#include <map>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"

namespace simVis { namespace GOG {

/**
 * Represents an entry that can contain an xy, xyz, ll, or lla point.
 * The provided context (e.g. "centerxy" vs "centerll") is expected to
 * be sufficient to differentiate between XY or LL point content.
 */
struct /* HEADER-ONLY */ PositionStrings
{
public:
  /** Construct the position without any values. */
  PositionStrings()
  {
  }

  /** Construct the position with an XY or LL point. */
  PositionStrings(const std::string& xVal, const std::string& yVal)
    : x(xVal),
      y(yVal)
  {
  }

  /** Construct the position with an XYZ or LLA point. */
  PositionStrings(const std::string& xVal, const std::string& yVal, const std::string& zVal)
    : x(xVal),
      y(yVal),
      z(zVal)
  {
  }

  /** X or Latitude value */
  std::string x;
  /** Y or Longitude value */
  std::string y;
  /** Z or Altitude value, if provided */
  std::string z;
};

/**
 * List of every possible parameter that can be stored in the ParsedShape parameter
 * maps.  Almost all of these values map either to a keyword in the GOG specification,
 * or to a parameter from a keyword in the specification.
 *
 * Not every keyword is directly represented here, because the GOG specification
 * provides a lot of duplication for keywords.  For example, "centerll", "centerlatlon",
 " and "centerlla" all mean the same thing; "diameter" is simply "radius" value times
 * two.  In cases where the keywords can be condensed, they are condensed.  The
 * following is a list of exceptions:
 *
 *  - start, end, comment, and version are not represented in this data structure.
 *  - Only annotation and latlonaltbox include parameters.
 *  - GOG_CENTERLL covers centerll, centerlla, centerlatlon.
 *  - GOG_CENTERXY covers centerxy, centerxyz.
 *  - GOG_REF_LAT, GOG_REF_LON, and GOG_REF_ALT are parameters for ref and referencepoint.
 *  - "diameter" modifier is represented as GOG_RADIUS (times two)
 *  - "rotate" modifier is represented as GOG_3D_FOLLOW
 *  - "semimajoraxis" modifier is represented as GOG_MAJORAXIS (times two)
 *  - "semiminoraxis" modifier is represented as GOG_MINORAXIS (times two)
 *  - GOG_ABSOLUTE is a flag set "true" when the GOG has ll, lla, or latlon points, and
 *      does not have a direct relationship to any single command.
 */
enum ShapeParameter
{
  // GOG Structure Commands
  GOG_DRAW = 0, // Maps to "off"

  // GOG Type Commands
  GOG_LLABOX_E = 20, // LatLonAltBox
  GOG_LLABOX_MAXALT, // LatLonAltBox
  GOG_LLABOX_MINALT, // LatLonAltBox
  GOG_LLABOX_N, // LatLonAltBox
  GOG_LLABOX_S, // LatLonAltBox
  GOG_LLABOX_W, // LatLonAltBox
  GOG_TEXT, // Annotation

  // GOG Position Commands
  GOG_CENTERLL = 40,
  GOG_CENTERXY,
  GOG_REF_LAT, // Reference LLA
  GOG_REF_LON, // Reference LLA
  GOG_REF_ALT, // Reference LLA
  GOG_CENTERLL2,
  GOG_CENTERXY2,

  // GOG Unit Commands
  GOG_ALTITUDEUNITS = 50,
  GOG_ANGLEUNITS,
  GOG_RANGEUNITS,
  GOG_TIMEUNITS,
  GOG_VERTICALDATUM,

  // GOG Modifier Commands
  GOG_ALTITUDEMODE = 100,
  GOG_ANGLEDEG, // Used by Arc, Cylinder
  GOG_ANGLEEND, // (Deprecated) Used by Arc, Cylinder
  GOG_ANGLESTART, // Used by Arc, Cylinder
  GOG_DEPTHBUFFER,
  // "diameter" maps to radius
  GOG_EXTRUDE,
  GOG_EXTRUDE_HEIGHT, // parameter for GOG_EXTRUDE
  GOG_FILLCOLOR,
  GOG_FILLED,
  GOG_FONTNAME,
  GOG_FONTSIZE,
  GOG_HEIGHT, // Used by Cylinder, Ellipsoid
  GOG_INNERRADIUS, // Used by Arc; currently unsupported
  GOG_LINECOLOR,
  GOG_LINEPROJECTION,
  GOG_LINESTYLE,
  GOG_LINEWIDTH,
  GOG_MAJORAXIS, // Used by Arc, Cylinder, Ellipse, Ellipsoid
  GOG_MINORAXIS, // Used by Arc, Cylinder, Ellipse, Ellipsoid
  GOG_ORIENT,
  GOG_ORIENT_HEADING, // Parameter to GOG_ORIENT
  GOG_ORIENT_PITCH, // Parameter to GOG_ORIENT
  GOG_ORIENT_ROLL, // Parameter to GOG_ORIENT
  GOG_OUTLINE,
  GOG_POINTSIZE,
  GOG_PRIORITY,
  GOG_RADIUS, // Used by Arc, Circle, Cylinder, Ellipse, Ellipsoid, Hemisphere, Sphere
  // "rotate" command is mapped to 3D Follow cpr
  GOG_SCALEX,
  GOG_SCALEY,
  GOG_SCALEZ,
  GOG_TESSELLATE,
  GOG_3D_BILLBOARD,
  GOG_3D_FOLLOW,
  GOG_3D_NAME,
  GOG_3D_OFFSETALT,
  GOG_3D_OFFSETCOURSE,
  GOG_3D_OFFSETPITCH,
  GOG_3D_OFFSETROLL,
  GOG_TEXTOUTLINECOLOR,
  GOG_TEXTOUTLINETHICKNESS,
  GOG_ICON,
  GOG_LLABOX_ROT, // rotation used by KML ground overlay latlonbox

  // Set to non-empty when GOG is absolute
  GOG_ABSOLUTE = 200,
};

/**
 * In-memory representation of a single parsed GOG shape.  Positions and
 * values are all stored as string representations, using normalized
 * parameter names (e.g. "centerll" and "centerlla" GOG commands both store
 * in "centerll" key).  This is the output of the stream parser, using
 * simVis::GOG::Parser::parse().
 */
class SDKVIS_EXPORT ParsedShape
{
public:
  /** A single shape may store points in LLA or XYZ mode, but not both */
  enum PointType
  {
    UNKNOWN,
    LLA,
    XYZ
  };

  /** Construct a new Parsed Shape */
  ParsedShape();

  /** Clear all internal structures and reset back to new. */
  void reset();

  /** Sets the line number for the GOG object.  May be 0 for none. */
  void setLineNumber(size_t lineNumber);
  /** Retrieves the stored line number. */
  size_t lineNumber() const;

  /** Sets the name of the shape, e.g. "line" or "annotation" */
  void setShape(const std::string& key);
  /** Retrieve the type of shape, e.g. "line" or "annotation". */
  std::string shape() const;

  /** Saves a configuration string, such as "linewidth" or "fillcolor" */
  void set(ShapeParameter key, const std::string& value);
  /** Saves a configuration position, such as "centerll" */
  void set(ShapeParameter key, const PositionStrings& pos);

  /** Retrieves a value from set(ShapeParameter, std::string) */
  std::string stringValue(ShapeParameter key, const std::string& defaultValue="") const;
  /** Retrieves a boolean value from the parameter map, converting from string, returning default value if needed */
  bool boolValue(ShapeParameter key, bool defaultValue) const;
  /** Retrieves a boolean value from the parameter map, converting from string, returning default value if needed */
  double doubleValue(ShapeParameter key, double defaultValue) const;
  /** Retrieves a value from set(ShapeParameter, PositionStrings) */
  PositionStrings positionValue(ShapeParameter key) const;

  /** Returns true if the given key is present in string or PositionStrings */
  bool hasValue(ShapeParameter key) const;

  /** Appends a position to the list of points.  Indicates whether position is ll or xy. 0 on success. */
  int append(PointType pointType, const PositionStrings& pos);
  /** Retrieves the points vector. */
  const std::vector<PositionStrings>& positions() const;

  /** Returns the type of points stored in the object: LLA, XYZ, or Unknown */
  PointType pointType() const;

private:
  std::string shape_;
  std::map<ShapeParameter, std::string> stringParams_;
  std::map<ShapeParameter, PositionStrings> positionParams_;
  std::vector<PositionStrings> points_;
  PointType pointType_;
  size_t lineNumber_;
};

}}

#endif /* SIMVIS_GOG_PARSEDSHAPE_H */
