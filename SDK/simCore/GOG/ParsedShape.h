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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_GOG_PARSEDSHAPE_H
#define SIMCORE_GOG_PARSEDSHAPE_H

#include <map>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/GOG/GogUtils.h"
#include "simCore/GOG/GogShape.h"

namespace simCore { namespace GOG {

/**
 * Represents an entry that can contain an xy, xyz, ll, or lla point.
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
 *  - CENTERLL covers centerll, centerlla, centerlatlon.
 *  - CENTERXY covers centerxy, centerxyz.
 *  - REF_LLA covers ref and referencepoint.
 *  - "diameter" modifier is represented as RADIUS (times two)
 *  - "rotate" modifier is represented as FOLLOW
 *  - "semimajoraxis" modifier is represented as MAJORAXIS (times two)
 *  - "semiminoraxis" modifier is represented as MINORAXIS (times two)
 *  - ABSOLUTE is a flag set "true" when the GOG has ll, lla, or latlon points, and
 *      does not have a direct relationship to any single command.
 */
enum class ShapeParameter
{
  // GOG Structure Commands
  DRAW = 0, // Maps to "off"

  // GOG Type Commands
  LLABOX_E = 20, // LatLonAltBox
  LLABOX_MAXALT, // LatLonAltBox
  LLABOX_MINALT, // LatLonAltBox
  LLABOX_N, // LatLonAltBox
  LLABOX_S, // LatLonAltBox
  LLABOX_W, // LatLonAltBox
  TEXT, // Annotation

  // GOG Position Commands
  CENTERLL = 40,
  CENTERXY,
  REF_LLA,
  CENTERLL2,
  CENTERXY2,

  // GOG Unit Commands
  ALTITUDEUNITS = 50,
  ANGLEUNITS,
  RANGEUNITS,
  TIMEUNITS,
  VERTICALDATUM,

  // GOG Modifier Commands
  ALTITUDEMODE = 100,
  ANGLEDEG, // Used by Arc, Cylinder
  ANGLEEND, // (Deprecated) Used by Arc, Cylinder
  ANGLESTART, // Used by Arc, Cylinder
  DEPTHBUFFER,
  // "diameter" maps to radius
  EXTRUDE_HEIGHT, // parameter for ALTITUDEMODE extrude
  FILLCOLOR,
  FILLED,
  FONTNAME,
  TEXTSIZE,
  HEIGHT, // Used by Cylinder, Ellipsoid
  INNERRADIUS, // Used by Arc
  LINECOLOR,
  LINEPROJECTION,
  LINESTYLE,
  LINEWIDTH,
  MAJORAXIS, // Used by Arc, Cylinder, Ellipse, Ellipsoid
  MINORAXIS, // Used by Arc, Cylinder, Ellipse, Ellipsoid
  ORIENT,
  OUTLINE,
  POINTSIZE,
  PRIORITY,
  RADIUS, // Used by Arc, Circle, Cylinder, Ellipse, Ellipsoid, Hemisphere, Sphere
  // "rotate" command is mapped to 3D Follow cpr
  SCALEX,
  SCALEY,
  SCALEZ,
  TESSELLATE,
  BILLBOARD,
  FOLLOW,
  NAME,
  OFFSETALT,
  OFFSETYAW,
  OFFSETPITCH,
  OFFSETROLL,
  TEXTOUTLINECOLOR,
  TEXTOUTLINETHICKNESS,
  IMAGE,
  LLABOX_ROT, // rotation used by KML ground overlay latlonbox
  TIME_START,
  TIME_END,
  OPACITY, // Used by ImageOverlay; opacity of the image file; other shapes use full RGBA values

  // Set to non-empty when GOG points are absolute (lla), rather than relative (xyz)
  ABSOLUTE_POINTS = 200,
};

/**
 * In-memory representation of a single parsed GOG shape.  Positions and
 * values are all stored as string representations, using normalized
 * parameter names (e.g. "centerll" and "centerlla" GOG commands both store
 * in "centerll" key).  This is used by the Parser as an intermediate stage
*  in the parsing process.
 */
class SDKCORE_EXPORT ParsedShape
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

  /** Sets the filename for the GOG object. May be empty for provisional or other locally created GOGs */
  void setFilename(const std::string& filename);
  /** Retrieve the object's filename */
  std::string filename() const;

  /** Sets the name of the shape, e.g. "line" or "annotation" */
  void setShape(ShapeType shape);
  /** Retrieve the type of shape, e.g. "line" or "annotation". */
  ShapeType shape() const;

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

  /** Add a comment associated with the shape */
  void addComment(const std::string& comment);
  /** Get the comments associated with this shape */
  const std::vector<std::string>& comments() const;

  /** Convert the GOG format boolean string to a bool */
  static bool getBoolFromString(const std::string& boolStr);

private:
  ShapeType shape_;
  std::map<ShapeParameter, std::string> stringParams_;
  std::map<ShapeParameter, PositionStrings> positionParams_;
  std::vector<PositionStrings> points_;
  PointType pointType_;
  size_t lineNumber_;
  std::string filename_;
  std::vector<std::string> comments_;
};

}}

#endif /* SIMCORE_GOG_PARSEDSHAPE_H */
