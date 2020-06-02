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
#include <cassert>
#include "simCore/Calc/Interpolation.h"
#include "simQt/QtConversion.h"
#include "simQt/ColorGradient.h"

namespace simQt {

ColorGradient::ColorGradient()
{
  _colorMap[0.00f] = osg::Vec4(0.f, 0.f, 1.f, 1.f); // blue
  _colorMap[0.25f] = osg::Vec4(0.f, 1.f, 1.f, 1.f); // cyan
  _colorMap[0.50f] = osg::Vec4(0.f, 1.f, 0.f, 1.f); // green
  _colorMap[0.75f] = osg::Vec4(1.f, 1.f, 0.f, 1.f); // yellow
  _colorMap[1.00f] = osg::Vec4(1.f, 0.f, 0.f, 1.f); // red
}

ColorGradient::ColorGradient(const std::map<float, QColor>& colors)
{
  // setColors will fix values outside [0,1]
  setColors(colors);
}

ColorGradient::ColorGradient(const osg::TransferFunction1D::ColorMap& colors)
{
  // setColors will fix values outside [0,1]
  setColors(colors);
}

ColorGradient::ColorGradient(const ColorGradient& grad, const osg::CopyOp& copyOp)
  : osg::TransferFunction1D(grad, copyOp)
{
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
  osg::TransferFunction1D::ColorMap colors;
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
  osg::TransferFunction1D::ColorMap colors;
  colors[0.0f] = osg::Vec4(0.f, 0.f, 0.f, 1.f);
  colors[1.0f] = osg::Vec4(1.f, 1.f, 1.f, 1.f);
  return ColorGradient(colors);
}

ColorGradient ColorGradient::newDopplerGradient()
{
  osg::TransferFunction1D::ColorMap colors;
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
  if (_colorMap.empty())
    return Qt::black;

  return simQt::getQtColorFromOsg(getColor(zeroToOne));
}

int ColorGradient::setColor(float zeroToOne, const QColor& color)
{
  if (zeroToOne < 0.f || zeroToOne > 1.f)
    return 1;
  _colorMap[zeroToOne] = simQt::getOsgColorFromQt(color);
  return 0;
}

int ColorGradient::setColor(float zeroToOne, const osg::Vec4& color, bool updateImage)
{
  if (zeroToOne < 0.f || zeroToOne > 1.f)
    return 1;

  osg::TransferFunction1D::setColor(zeroToOne, color, updateImage);
  return 0;
}

int ColorGradient::removeColor(float zeroToOne)
{
  auto i = _colorMap.find(zeroToOne);
  if (i == _colorMap.end())
    return 1;
  _colorMap.erase(i);
  return 0;
}

void ColorGradient::clearColors()
{
  _colorMap.clear();
}

std::map<float, QColor> ColorGradient::colors() const
{
  std::map<float, QColor> map;
  for (const auto& stop : _colorMap)
    map[stop.first] = simQt::getQtColorFromOsg(stop.second);
  return map;
}

osg::TransferFunction1D::ColorMap ColorGradient::getColorMap() const
{
  return _colorMap;
}

void ColorGradient::setColors(const std::map<float, QColor>& colors)
{
  clearColors();

  for (const auto& stop : colors)
  {
    // Remove values before 0. and after 1.
    if (stop.first >= 0.f && stop.first <= 1.f)
      _colorMap[stop.first] = simQt::getOsgColorFromQt(stop.second);
  }
}

void ColorGradient::setColors(const osg::TransferFunction1D::ColorMap& colors)
{
  clearColors();

  for (const auto& stop : colors)
  {
    // Remove values before 0. and after 1.
    if (stop.first >= 0.f && stop.first <= 1.f)
      _colorMap[stop.first] = stop.second;
  }
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

bool ColorGradient::operator==(const ColorGradient& rhs) const
{
  return _colorMap == rhs._colorMap;
}

bool ColorGradient::operator!=(const ColorGradient& rhs) const
{
  return _colorMap != rhs._colorMap;
}

}
