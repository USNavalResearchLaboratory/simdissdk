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
#include <cassert>
#include "simCore/String/Format.h"
#include "simCore/String/ValidNumber.h"
#include "simVis/GOG/ParsedShape.h"

namespace simVis { namespace GOG {

ParsedShape::ParsedShape()
  : pointType_(UNKNOWN),
    lineNumber_(0)
{
}

void ParsedShape::reset()
{
  shape_.clear();
  stringParams_.clear();
  positionParams_.clear();
  points_.clear();
  pointType_ = UNKNOWN;
  lineNumber_ = 0;
}

void ParsedShape::setLineNumber(size_t lineNumber)
{
  lineNumber_ = lineNumber;
}

size_t ParsedShape::lineNumber() const
{
  return lineNumber_;
}

void ParsedShape::set(ShapeParameter key, const std::string& value)
{
  stringParams_[key] = value;
}

void ParsedShape::set(ShapeParameter key, const PositionStrings& pos)
{
  positionParams_[key] = pos;
}

std::string ParsedShape::stringValue(ShapeParameter key, const std::string& defaultValue) const
{
  auto i = stringParams_.find(key);
  if (i != stringParams_.end())
    return i->second;
  return defaultValue;
}

bool ParsedShape::boolValue(ShapeParameter key, bool defaultValue) const
{
  auto i = stringParams_.find(key);
  if (i == stringParams_.end())
    return defaultValue;
  const std::string& temp = simCore::lowerCase(i->second);
  if (temp == "true" || temp == "yes" || temp == "on" || temp == "1")
    return true;
  if (temp == "false" || temp == "no" || temp == "off" || temp == "0")
    return false;
  return defaultValue;
}

double ParsedShape::doubleValue(ShapeParameter key, double defaultValue) const
{
  auto i = stringParams_.find(key);
  if (i == stringParams_.end())
    return defaultValue;
  double rv = 0.;
  if (simCore::isValidNumber(i->second, rv))
    return rv;
  return defaultValue;
}

PositionStrings ParsedShape::positionValue(ShapeParameter key) const
{
  auto i = positionParams_.find(key);
  if (i != positionParams_.end())
    return i->second;
  return PositionStrings();
}

bool ParsedShape::hasValue(ShapeParameter key) const
{
  return (stringParams_.find(key) != stringParams_.end()) ||
    (positionParams_.find(key) != positionParams_.end());
}

void ParsedShape::setShape(const std::string& key)
{
  shape_ = key;
}

std::string ParsedShape::shape() const
{
  return shape_;
}

int ParsedShape::append(PointType pointType, const PositionStrings& pos)
{
  if (pointType == UNKNOWN)
  {
    // Avoid appending unknown points, dev error
    assert(0);
    return 1;
  }
  if (pointType_ == UNKNOWN)
    pointType_ = pointType;
  else if (pointType_ != pointType)
  {
    // Cannot mix and match "ll" and "xy" in same GOG
    assert(0);
    return 1;
  }
  points_.push_back(pos);
  return 0;
}

const std::vector<PositionStrings>& ParsedShape::positions() const
{
  return points_;
}

ParsedShape::PointType ParsedShape::pointType() const
{
  return pointType_;
}

}}
