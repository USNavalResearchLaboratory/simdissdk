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
#include <cassert>
#include "simCore/String/Format.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/GOG/ParsedShape.h"

namespace simCore { namespace GOG {

ParsedShape::ParsedShape()
  : shape_(ShapeType::UNKNOWN),
    pointType_(UNKNOWN),
    lineNumber_(0)
{
}

void ParsedShape::reset()
{
  shape_ = ShapeType::UNKNOWN;
  stringParams_.clear();
  positionParams_.clear();
  points_.clear();
  pointType_ = UNKNOWN;
  lineNumber_ = 0;
  filename_.clear();
  comments_.clear();
}

void ParsedShape::setLineNumber(size_t lineNumber)
{
  lineNumber_ = lineNumber;
}

size_t ParsedShape::lineNumber() const
{
  return lineNumber_;
}

void ParsedShape::setFilename(const std::string& filename)
{
  filename_ = filename;
}

std::string ParsedShape::filename() const
{
  return filename_;
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
  return getBoolFromString(i->second);
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

void ParsedShape::setShape(ShapeType shape)
{
  shape_ = shape;
}

ShapeType ParsedShape::shape() const
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

void ParsedShape::addComment(const std::string& comment)
{
  comments_.push_back(comment);
}

const std::vector<std::string>& ParsedShape::comments() const
{
  return comments_;
}

bool ParsedShape::getBoolFromString(const std::string& boolStr)
{
  const std::string& temp = simCore::lowerCase(boolStr);
  return stringIsTrueToken(temp);
}

}}
