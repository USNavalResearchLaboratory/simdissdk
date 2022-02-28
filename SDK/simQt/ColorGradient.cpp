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
  setColors(colors);
}

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

ColorGradient::~ColorGradient()
{
}

ColorGradient ColorGradient::newDefaultGradient()
{
  return ColorGradient();
}

ColorGradient ColorGradient::newDarkGradient()
{
  std::map<float, osg::Vec4> colors;
  colors[0.0f] = osg::Vec4(0.f, 0.f, 0.f, 1.f); // black
  colors[0.2f] = osg::Vec4(0.f, 0.f, 1.f, 1.f); // blue
  colors[0.4f] = osg::Vec4(0.f, 1.f, 1.f, 1.f); // cyan
  colors[0.5f] = osg::Vec4(0.f, 1.f, 0.f, 1.f); // green
  colors[0.6f] = osg::Vec4(1.f, 1.f, 0.f, 1.f); // yellow
  colors[0.8f] = osg::Vec4(1.f, 0.f, 0.f, 1.f); // red
  colors[1.0f] = osg::Vec4(0.f, 0.f, 0.f, 1.f); // black
  return ColorGradient(colors);
}

ColorGradient ColorGradient::newGreyscaleGradient()
{
  std::map<float, osg::Vec4> colors;
  colors[0.0f] = osg::Vec4(0.f, 0.f, 0.f, 1.f);
  colors[1.0f] = osg::Vec4(1.f, 1.f, 1.f, 1.f);
  return ColorGradient(colors);
}

ColorGradient ColorGradient::newDopplerGradient()
{
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
  return ColorGradient(colors);
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
  return function_->getColorMap() == rhs.function_->getColorMap();
}

bool ColorGradient::operator!=(const ColorGradient& rhs) const
{
  return function_->getColorMap() != rhs.function_->getColorMap();
}

}
