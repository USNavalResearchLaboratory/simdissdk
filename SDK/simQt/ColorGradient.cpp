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
#include <algorithm>
#include <cassert>
#include "simCore/Calc/Interpolation.h"
#include "simQt/QtConversion.h"
#include "simQt/ColorGradient.h"

namespace simQt {

const osg::Vec4 CC_BLACK(0.f, 0.f, 0.f, 1.f);
const osg::Vec4 CC_WHITE(1.f, 1.f, 1.f, 1.f);
const osg::Vec4 CC_BLUE(0.f, 0.f, 1.f, 1.f);
const osg::Vec4 CC_CYAN(0.f, 1.f, 1.f, 1.f);
const osg::Vec4 CC_GREEN(0.f, 1.f, 0.f, 1.f);
const osg::Vec4 CC_YELLOW(1.f, 1.f, 0.f, 1.f);
const osg::Vec4 CC_RED(1.f, 0.f, 0.f, 1.f);
const osg::Vec4 CC_ORANGE(1.f, 0.5, 0.f, 1.f);
const osg::Vec4 CC_MAGENTA(1.f, 0.f, 1.f, 1.f);
const osg::Vec4 CC_GREY_753(0.753f, 0.753f, 0.753f, 1.f);

ColorGradient::ColorGradient()
  : function_(new osg::TransferFunction1D)
{
  controlColors_ = {
    { 0.00f, CC_BLACK },
    { 1.00f, CC_RED },

    { 0.00f, CC_BLUE },
    { 0.25f, CC_CYAN },
    { 0.50f, CC_GREEN },
    { 0.75f, CC_YELLOW },
    { 1.00f, CC_RED },
  };
  updateTransferFunc_();
}

ColorGradient::ColorGradient(const ColorGradient& rhs)
  : function_(new osg::TransferFunction1D),
    discrete_(rhs.discrete_),
    controlColors_(rhs.controlColors_)
{
  updateTransferFunc_();
}

ColorGradient& ColorGradient::operator=(const ColorGradient& rhs)
{
  if (this == &rhs)
    return *this;
  discrete_ = rhs.discrete_;
  controlColors_ = rhs.controlColors_;
  updateTransferFunc_();
  return *this;
}

ColorGradient::~ColorGradient()
{
}

ColorGradient ColorGradient::newDefaultGradient()
{
  return ColorGradient();
}

ColorGradient ColorGradient::newDarkGradient()
{
  ColorGradient rv;
  rv.controlColors_ = {
    { 0.0f, CC_BLACK },
    { 1.0f, CC_BLACK },

    { 0.2f, CC_BLUE },
    { 0.4f, CC_CYAN },
    { 0.5f, CC_GREEN },
    { 0.6f, CC_YELLOW },
    { 0.8f, CC_RED },
  };
  rv.updateTransferFunc_();
  return rv;
}

ColorGradient ColorGradient::newGreyscaleGradient()
{
  ColorGradient rv;
  rv.controlColors_ = {
    { 0.0f, CC_BLACK },
    { 1.0f, CC_WHITE },

    { 0.0f, CC_BLACK },
    { 1.0f, CC_WHITE },
  };
  rv.updateTransferFunc_();
  return rv;
}

ColorGradient ColorGradient::newDopplerGradient()
{
  ColorGradient rv;
  rv.controlColors_ = {
    { 0.0f, CC_GREY_753 },
    { 1.0f, CC_WHITE },

    { 0.00170775f, CC_GREY_753 },
    { 0.126281f, osg::Vec4(0.f, 0.878f, 1.f, 1.f) }, // cyan
    { 0.250854f, osg::Vec4(0.f, 0.941f, 0.f, 1.f) }, // green
    { 0.375427f, osg::Vec4(0.f, 0.584f, 0.f, 1.f) }, // olive green
    { 0.50f, osg::Vec4(0.f, 0.333f, 0.f, 1.f) }, // dark green
    { 0.626838f, CC_YELLOW },
    { 0.749146f, CC_ORANGE },
    { 0.875984f, CC_RED },
    { 0.998292f, CC_MAGENTA },
    { 1.0f, CC_WHITE },
  };
  rv.updateTransferFunc_();
  return rv;
}

QColor ColorGradient::colorAt(float zeroToOne) const
{
  return simQt::getQtColorFromOsg(osgColorAt(zeroToOne));
}

osg::Vec4 ColorGradient::osgColorAt(float zeroToOne) const
{
  if (!discrete_)
    return function_->getColor(zeroToOne);
  const auto& map = function_->getColorMap();
  auto iter = map.upper_bound(zeroToOne);
  if (iter != map.begin())
    --iter;
  return iter->second;
}

size_t ColorGradient::addControlColor(float zeroToOne, const QColor& color)
{
  return addControlColor(zeroToOne, simQt::getOsgColorFromQt(color));
}

size_t ColorGradient::addControlColor(float zeroToOne, const osg::Vec4& color)
{
  controlColors_.emplace_back(std::make_pair(simCore::clamp(zeroToOne, 0.f, 1.f), color));
  updateTransferFunc_();
  return controlColors_.size() - 1;
}

int ColorGradient::setControlColor(size_t index, float zeroToOne, const QColor& color)
{
  return setControlColor(index, zeroToOne, simQt::getOsgColorFromQt(color));
}

int ColorGradient::setControlColor(size_t index, float zeroToOne, const osg::Vec4& color)
{
  if (index >= controlColors_.size())
    return 1;

  // Bound the zero-to-one
  if (index == 0)
    zeroToOne = 0.f;
  else if (index == 1)
    zeroToOne = 1.f;
  else
    zeroToOne = simCore::clamp(zeroToOne, 0.f, 1.f);

  controlColors_[index] = { zeroToOne, color };
  updateTransferFunc_();
  return 0;
}

int ColorGradient::removeControlColor(size_t index)
{
  if (index < 2 || index >= controlColors_.size())
    return 1;

  controlColors_.erase(controlColors_.begin() + index);
  updateTransferFunc_();
  return 0;
}

void ColorGradient::clearControlColors()
{
  controlColors_ = {
    { 0.f, CC_WHITE },
    { 1.f, CC_WHITE },
  };
  updateTransferFunc_();
}

QColor ColorGradient::controlColor(size_t index) const
{
  if (index >= controlColors_.size())
    return QColor::fromRgba(0);
  return simQt::getQtColorFromOsg(controlColors_[index].second);
}

osg::Vec4 ColorGradient::osgControlColor(size_t index) const
{
  return simQt::getOsgColorFromQt(controlColor(index));
}

float ColorGradient::controlColorPct(size_t index) const
{
  if (index >= controlColors_.size())
    return -1.f;
  return controlColors_[index].first;
}

size_t ColorGradient::numControlColors() const
{
  return controlColors_.size();
}

void ColorGradient::importColorMap(const std::map<float, QColor>& colors)
{
  if (colors.empty())
  {
    clearControlColors();
    return;
  }

  // Add 0th and 1st entry at 0% and 100%
  auto iter = colors.upper_bound(0.f);
  if (iter != colors.begin())
    --iter;
  controlColors_ = { { 0.f, simQt::getOsgColorFromQt(iter->second) } };
  iter = colors.upper_bound(1.f);
  if (iter != colors.begin())
    --iter;
  controlColors_.emplace_back(std::make_pair(1.f, simQt::getOsgColorFromQt(iter->second)));

  // Add all points between 0 and 1 as control points. this allows for compression later if desired
  for (const auto& pctColor : colors)
  {
    if (simCore::isBetween(pctColor.first, 0.f, 1.f))
      controlColors_.emplace_back(std::make_pair(pctColor.first, simQt::getOsgColorFromQt(pctColor.second)));
  }
  updateTransferFunc_();
}

void ColorGradient::importColorVector(const std::vector<std::pair<float, QColor> >& colorVec)
{
  // Assume nothing about the input vector. It might have 0% or 100% items, or not; it might
  // have values outside the range [0,1] that need to be ignored. We'll gather two maps, one
  // that has original colors for extracting the 0,1 in order to determine the bounds, and
  // another that overwrites previous values so that we can get the other control points.
  std::map<float, osg::Vec4> noOverwriteMap;
  std::map<float, osg::Vec4> overwriteMap;
  for (const auto& pctColor : colorVec)
  {
    if (!simCore::isBetween(pctColor.first, 0.f, 1.f))
      continue;
    const osg::Vec4& osgColor = simQt::getOsgColorFromQt(pctColor.second);
    overwriteMap[pctColor.first] = osgColor;
    // std::map::emplace will not overwrite existing entries
    noOverwriteMap.emplace(std::make_pair(pctColor.first, osgColor));
  }

  // Avoid noop
  if (noOverwriteMap.empty())
  {
    clearControlColors();
    return;
  }

  // Extract the 0% color and 100% color
  controlColors_.clear();
  controlColors_.emplace_back(std::make_pair(0.f, noOverwriteMap.begin()->second));
  controlColors_.emplace_back(std::make_pair(1.f, noOverwriteMap.rbegin()->second));

  // Add all other control colors in order
  for (const auto& pctColor : overwriteMap)
    controlColors_.emplace_back(pctColor);

  updateTransferFunc_();
}

std::map<float, osg::Vec4> ColorGradient::effectiveColorMap() const
{
  return function_->getColorMap();
}

QColor ColorGradient::interpolate(const QColor& lowColor, const QColor& highColor, float low, float val, float high)
{
  const float factor = simCore::getFactor(low, val, high);
  QColor rv;
  rv.setRedF(lowColor.redF() + (highColor.redF() - lowColor.redF()) * factor);
  rv.setGreenF(lowColor.greenF() + (highColor.greenF() - lowColor.greenF()) * factor);
  rv.setBlueF(lowColor.blueF() + (highColor.blueF() - lowColor.blueF()) * factor);
  rv.setAlphaF(lowColor.alphaF() + (highColor.alphaF() - lowColor.alphaF()) * factor);
  return rv;
}

void ColorGradient::setDiscrete(bool discrete)
{
  discrete_ = discrete;
}

bool ColorGradient::discrete() const
{
  return discrete_;
}

ColorGradient ColorGradient::compress(float lowPercent, float highPercent) const
{
  // Must always have at least 2 control colors
  assert(controlColors_.size() >= 2);

  ColorGradient rv;
  rv.controlColors_ = {
    controlColors_[0],
    controlColors_[1]
  };

  // Loop through our control colors, adding them to the new gradient, reprojected
  for (size_t k = 2; k < controlColors_.size(); ++k)
  {
    const float newPct = simCore::linearInterpolate(lowPercent, highPercent,
      0.0, controlColors_[k].first, 1.0);
    rv.controlColors_.emplace_back(std::make_pair(newPct, controlColors_[k].second));
  }

  // If low is greater than high, then swap the colors on the 0% and 100% too
  if (lowPercent > highPercent)
    std::swap(rv.controlColors_[0].second, rv.controlColors_[1].second);
  return rv;
}

void ColorGradient::updateTransferFunc_()
{
  std::map<float, osg::Vec4> effectiveColors;
  for (const auto& ccPair : controlColors_)
    effectiveColors[ccPair.first] = ccPair.second;
  function_->setColorMap(effectiveColors);
}

bool ColorGradient::operator==(const ColorGradient& rhs) const
{
  // No need to test the transfer function, since it is updated whenever control colors update
  return discrete_ == rhs.discrete_ &&
    controlColors_ == rhs.controlColors_;
}

bool ColorGradient::operator!=(const ColorGradient& rhs) const
{
  // No need to test the transfer function, since it is updated whenever control colors update
  return discrete_ != rhs.discrete_ ||
    controlColors_ != rhs.controlColors_;
}

}
