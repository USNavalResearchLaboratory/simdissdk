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
#include <algorithm>
#include <cassert>
#include "simCore/Calc/Interpolation.h"
#include "simQt/QtConversion.h"
#include "simQt/ColorGradient.h"

namespace simQt {

ColorGradient::ColorGradient()
  : function_(new osg::TransferFunction1D)
{
  std::map<float, osg::Vec4> colors;
  colors[0.00f] = osg::Vec4(0.f, 0.f, 1.f, 1.f); // blue
  colors[0.25f] = osg::Vec4(0.f, 1.f, 1.f, 1.f); // cyan
  colors[0.50f] = osg::Vec4(0.f, 1.f, 0.f, 1.f); // green
  colors[0.75f] = osg::Vec4(1.f, 1.f, 0.f, 1.f); // yellow
  colors[1.00f] = osg::Vec4(1.f, 0.f, 0.f, 1.f); // red
  function_->setColorMap(colors);
}

#ifdef OLD_SIMQT_COLORGRADIENT_API
ColorGradient::ColorGradient(const std::map<float, QColor>& colors)
  : function_(new osg::TransferFunction1D)
{
  // Start with a default "empty" gradient
  clearColors();
  // setColors will fix values outside [0,1]
  setColors(colors);
}

ColorGradient::ColorGradient(const std::map<float, osg::Vec4>& colors)
  : function_(new osg::TransferFunction1D)
{
  // Start with a default "empty" gradient
  clearColors();
  // setColors will fix values outside [0,1]
  setColors(colors);
}
#endif

ColorGradient::ColorGradient(const ColorGradient& rhs)
  : function_(new osg::TransferFunction1D),
    discrete_(rhs.discrete_)
{
  function_->setColorMap(rhs.function_->getColorMap());
}

ColorGradient& ColorGradient::operator=(const ColorGradient& rhs)
{
  if (this == &rhs)
    return *this;
  discrete_ = rhs.discrete_;
  function_->setColorMap(rhs.function_->getColorMap());
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

  std::map<float, osg::Vec4> colors;
  colors[0.0f] = osg::Vec4(0.f, 0.f, 0.f, 1.f); // black
  colors[0.2f] = osg::Vec4(0.f, 0.f, 1.f, 1.f); // blue
  colors[0.4f] = osg::Vec4(0.f, 1.f, 1.f, 1.f); // cyan
  colors[0.5f] = osg::Vec4(0.f, 1.f, 0.f, 1.f); // green
  colors[0.6f] = osg::Vec4(1.f, 1.f, 0.f, 1.f); // yellow
  colors[0.8f] = osg::Vec4(1.f, 0.f, 0.f, 1.f); // red
  colors[1.0f] = osg::Vec4(0.f, 0.f, 0.f, 1.f); // black
  rv.function_->setColorMap(colors);
  return rv;
}

ColorGradient ColorGradient::newGreyscaleGradient()
{
  ColorGradient rv;

  std::map<float, osg::Vec4> colors;
  colors[0.0f] = osg::Vec4(0.f, 0.f, 0.f, 1.f);
  colors[1.0f] = osg::Vec4(1.f, 1.f, 1.f, 1.f);
  rv.function_->setColorMap(colors);
  return rv;
}

ColorGradient ColorGradient::newDopplerGradient()
{
  ColorGradient rv;

  std::map<float, osg::Vec4> colors;
  colors[0.0f] = osg::Vec4(0.753f, 0.753f, 0.753f, 1.f); // grey
  colors[0.00170775f] = osg::Vec4(0.753f, 0.753f, 0.753f, 1.f); // grey
  colors[0.126281f] = osg::Vec4(0.f, 0.878f, 1.f, 1.f); // cyan
  colors[0.250854f] = osg::Vec4(0.f, 0.941f, 0.f, 1.f); // green
  colors[0.375427f] = osg::Vec4(0.f, 0.584f, 0.f, 1.f); // olive green
  colors[0.50f] = osg::Vec4(0.f, 0.333f, 0.f, 1.f); // dark green
  colors[0.626838f] = osg::Vec4(1.f, 1.f, 0.f, 1.f); // yellow
  colors[0.749146f] = osg::Vec4(1.f, 0.5f, 0.f, 1.f); // orange
  colors[0.875984f] = osg::Vec4(1.f, 0.f, 0.f, 1.f); // red
  colors[0.998292f] = osg::Vec4(1.f, 0.f, 1.f, 1.f); // magenta
  colors[1.0f] = osg::Vec4(1.f, 1.f, 1.f, 1.f); // white
  rv.function_->setColorMap(colors);
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
  auto map = function_->getColorMap();
  auto iter = map.upper_bound(zeroToOne);
  if (iter != map.begin())
    --iter;
  return iter->second;
}

#ifdef OLD_SIMQT_COLORGRADIENT_API
int ColorGradient::setColor(float zeroToOne, const QColor& color)
{
  return setColor(zeroToOne, simQt::getOsgColorFromQt(color));
}

int ColorGradient::setColor(float zeroToOne, const osg::Vec4& color)
{
  if (zeroToOne < 0.f || zeroToOne > 1.f)
    return 1;

  function_->setColor(zeroToOne, color, true);
  return 0;
}

int ColorGradient::removeColor(float zeroToOne)
{
  if (function_->getColorMap().size() <= 1)
    return 1;

  auto& colorMap = function_->getColorMap();
  auto i = colorMap.find(zeroToOne);
  if (i == colorMap.end())
    return 1;
  colorMap.erase(i);
  return 0;
}

void ColorGradient::clearColors()
{
  function_->clear();
}

std::map<float, QColor> ColorGradient::colors() const
{
  std::map<float, QColor> map;
  for (const auto& stop : function_->getColorMap())
    map[stop.first] = simQt::getQtColorFromOsg(stop.second);
  return map;
}

osg::TransferFunction1D::ColorMap ColorGradient::getColorMap() const
{
  return function_->getColorMap();
}

int ColorGradient::colorCount() const
{
  const int count = static_cast<int>(function_->getColorMap().size());
  assert(count >= 1); // At least one stop is required
  return count;
}

int ColorGradient::setColors(const std::map<float, QColor>& colors)
{
  if (colors.empty())
    return 1;

  auto& colorMap = function_->getColorMap();
  std::map<float, osg::Vec4> oldColorMap = colorMap;

  // Manually clear map instead of calling clearColors(), since it sets default colors
  colorMap.clear();

  for (const auto& stop : colors)
  {
    // Remove values before 0. and after 1.
    if (stop.first >= 0.f && stop.first <= 1.f)
      colorMap[stop.first] = simQt::getOsgColorFromQt(stop.second);
  }

  // Restore old colors if new map doesn't have at least one stop
  if (colorMap.empty())
  {
    colorMap = oldColorMap;
    return 1;
  }
  return 0;
}

int ColorGradient::setColors(const std::map<float, osg::Vec4>& colors)
{
  if (colors.empty())
    return 1;

  auto& colorMap = function_->getColorMap();
  std::map<float, osg::Vec4> oldColorMap = colorMap;

  // Manually clear map instead of calling clearColors(), since it sets default colors
  colorMap.clear();

  for (const auto& stop : colors)
  {
    // Remove values before 0. and after 1.
    if (stop.first >= 0.f && stop.first <= 1.f)
      colorMap[stop.first] = stop.second;
  }

  // Restore old colors if new map doesn't have at least one stop
  if (colorMap.empty())
  {
    colorMap = oldColorMap;
    return 1;
  }
  return 0;
}
#endif

#ifdef NEW_SIMQT_COLORGRADIENT_API
size_t ColorGradient::addControlColor(float zeroToOne, const QColor& color)
{
  return addControlColor(zeroToOne, simQt::getOsgColorFromQt(color));
}

size_t ColorGradient::addControlColor(float zeroToOne, const osg::Vec4& color)
{
  auto& colorMap = function_->getColorMap();
  zeroToOne = simCore::clamp(zeroToOne, 0.f, 1.f);
  colorMap[zeroToOne] = color;
  auto iter = colorMap.find(zeroToOne);
  if (iter == colorMap.end())
  {
    // Should not be possible
    assert(0);
    return 0; // index of added color; invalid, return value can't make sense
  }
  return std::distance(colorMap.begin(), iter);
}

int ColorGradient::setControlColor(size_t index, float zeroToOne, const QColor& color)
{
  return setControlColor(index, zeroToOne, simQt::getOsgColorFromQt(color));
}

int ColorGradient::setControlColor(size_t index, float zeroToOne, const osg::Vec4& color)
{
  auto& colorMap = function_->getColorMap();
  if (index >= colorMap.size())
    return 1;
  auto iter = colorMap.begin();
  std::advance(iter, index);
  // First need to remove the value, then need to re-add it. Avoid editing std::map::iterator::first
  colorMap.erase(iter);
  colorMap[simCore::clamp(zeroToOne, 0.f, 1.f)] = color;
  return 0;
}

int ColorGradient::removeControlColor(size_t index)
{
  auto& colorMap = function_->getColorMap();
  if (index >= colorMap.size())
    return 1;
  auto iter = colorMap.begin();
  std::advance(iter, index);
  colorMap.erase(iter);
  return 0;
}

void ColorGradient::clearControlColors()
{
  function_->clear();
}

QColor ColorGradient::controlColor(size_t index) const
{
  const auto& colorMap = function_->getColorMap();
  if (index >= colorMap.size())
    return QColor(QRgb(0));
  auto iter = colorMap.begin();
  std::advance(iter, index);
  return simQt::getQtColorFromOsg(iter->second);
}

osg::Vec4 ColorGradient::osgControlColor(size_t index) const
{
  return simQt::getOsgColorFromQt(controlColor(index));
}

float ColorGradient::controlColorPct(size_t index) const
{
  const auto& colorMap = function_->getColorMap();
  if (index >= colorMap.size())
    return -1.f;
  auto iter = colorMap.begin();
  std::advance(iter, index);
  return iter->first;
}

size_t ColorGradient::numControlColors() const
{
  return function_->getColorMap().size();
}

void ColorGradient::importColorMap(const std::map<float, QColor>& colors)
{
  std::map<float, osg::Vec4> osgColors;
  for (const auto& pctColor : colors)
    osgColors[simCore::clamp(pctColor.first, 0.f, 1.f)] = simQt::getOsgColorFromQt(pctColor.second);

  if (osgColors.empty())
  {
    // Reset both ends to white
    osgColors[0.f] = osg::Vec4(1.f, 1.f, 1.f, 1.f);
    osgColors[1.f] = osg::Vec4(1.f, 1.f, 1.f, 1.f);
  }
  else
  {
    // Ensure values at 0.f and 1.f
    if (osgColors.begin()->first != 0.f)
      osgColors[0.f] = osgColors.begin()->second;
    if (osgColors.rbegin()->first != 1.f)
      osgColors[1.f] = osgColors.rbegin()->second;
  }

  function_->setColorMap(osgColors);
}

void ColorGradient::importColorVector(const std::vector<std::pair<float, QColor> >& colorVec)
{
  // Map the colors into 0-to-1 space, so long as the transfer function is underlying data struct
  std::map<float, QColor> colorMap;
  for (const auto& pctColor : colorVec)
    colorMap[pctColor.first] = pctColor.second;
  importColorMap(colorMap);
}

std::map<float, osg::Vec4> ColorGradient::effectiveColorMap() const
{
  return function_->getColorMap();
}

#endif // NEW_SIMQT_COLORGRADIENT_API

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

bool ColorGradient::operator==(const ColorGradient& rhs) const
{
  return discrete_ == rhs.discrete_ &&
    function_->getColorMap() == rhs.function_->getColorMap();
}

bool ColorGradient::operator!=(const ColorGradient& rhs) const
{
  return discrete_ != rhs.discrete_ ||
    function_->getColorMap() != rhs.function_->getColorMap();
}

}
