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
  void set(const std::string& key, const std::string& value);
  /** Saves a configuration position, such as "centerll" */
  void set(const std::string& key, const PositionStrings& pos);

  /** Retrieves a value from set(std::string, std::string) */
  std::string stringValue(const std::string& key, const std::string& defaultValue="") const;
  /** Retrieves a boolean value from the parameter map, converting from string, returning default value if needed */
  bool boolValue(const std::string& key, bool defaultValue) const;
  /** Retrieves a boolean value from the parameter map, converting from string, returning default value if needed */
  double doubleValue(const std::string& key, double defaultValue) const;
  /** Retrieves a value from set(std::string, PositionStrings) */
  PositionStrings positionValue(const std::string& key) const;

  /** Returns true if the given key is present in string or PositionStrings */
  bool hasValue(const std::string& key) const;

  /** Appends a position to the list of points.  Indicates whether position is ll or xy. 0 on success. */
  int append(PointType pointType, const PositionStrings& pos);
  /** Retrieves the points vector. */
  const std::vector<PositionStrings>& positions() const;

  /** Returns the type of points stored in the object: LLA, XYZ, or Unknown */
  PointType pointType() const;

private:
  std::string shape_;
  std::map<std::string, std::string> stringParams_;
  std::map<std::string, PositionStrings> positionParams_;
  std::vector<PositionStrings> points_;
  PointType pointType_;
  size_t lineNumber_;
};

}}

#endif /* SIMVIS_GOG_PARSEDSHAPE_H */
