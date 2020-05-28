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
#include "simQt/ColorGradient.h"

namespace simQt {

ColorGradient::ColorGradient()
{
  colors_[0.00] = qRgba(0, 0, 255, 255);
  colors_[0.25] = qRgba(0, 255, 255, 255);
  colors_[0.50] = qRgba(0, 255, 0, 255);
  colors_[0.75] = qRgba(255, 255, 0, 255);
  colors_[1.00] = qRgba(255, 0, 0, 255);
}

ColorGradient::ColorGradient(const std::map<double, QColor>& colors)
{
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
  std::map<double, QColor> colors;
  colors[0.0] = qRgba(0, 0, 0, 255);
  colors[0.2] = qRgba(0, 0, 255, 255);
  colors[0.4] = qRgba(0, 255, 255, 255);
  colors[0.5] = qRgba(0, 255, 0, 255);
  colors[0.6] = qRgba(255, 255, 0, 255);
  colors[0.8] = qRgba(255, 0, 0, 255);
  colors[1.0] = qRgba(0, 0, 0, 255);
  return ColorGradient(colors);
}

ColorGradient ColorGradient::newGreyscaleGradient()
{
  std::map<double, QColor> colors;
  colors[0.0] = qRgba(0, 0, 0, 255);
  colors[1.0] = qRgba(255, 255, 255, 255);
  return ColorGradient(colors);
}

ColorGradient ColorGradient::newDopplerGradient()
{
  std::map<double, QColor> colors;
  colors[0.0] = qRgba(192, 192, 192, 255); // grey
  colors[0.00170775] = qRgba(192, 192, 192, 255); // grey
  colors[0.126281] = qRgba(0, 224, 255, 255); // cyan
  colors[0.250854] = qRgba(0, 240, 0, 255); // green
  colors[0.375427] = qRgba(0, 149, 0, 255); // olive green
  colors[0.50] = qRgba(0, 85, 0, 255); // dark green
  colors[0.626838] = qRgba(255, 255, 0, 255); // yellow
  colors[0.749146] = qRgba(255, 128, 0, 255); // orange
  colors[0.875984] = qRgba(255, 0, 0, 255); // red
  colors[0.998292] = qRgba(255, 0, 255, 255); // magenta
  colors[1.0] = qRgba(255, 255, 255, 255); // white
  return ColorGradient(colors);
}

QColor ColorGradient::colorAt(double zeroToOne) const
{
  if (colors_.empty())
    return Qt::black;
  if (zeroToOne <= colors_.begin()->first)
    return colors_.begin()->second;
  if (zeroToOne >= colors_.rbegin()->first)
    return colors_.rbegin()->second;

  auto after = colors_.upper_bound(zeroToOne);
  // Should not be possible due to check on rbegin()
  assert(after != colors_.end());
  // Should not be possible due to check on begin()
  assert(after != colors_.begin());
  if (after == colors_.end() || after == colors_.begin())
    return Qt::black;

  auto before = after;
  --before;
  if (before->first == zeroToOne)
    return before->second;
  return ColorGradient::interpolate(before->second, after->second, before->first, zeroToOne, after->first);
}

int ColorGradient::setColor(double zeroToOne, const QColor& color)
{
  if (zeroToOne < 0. || zeroToOne > 1.)
    return 1;
  colors_[zeroToOne] = color;
  return 0;
}

int ColorGradient::removeColor(double zeroToOne)
{
  auto i = colors_.find(zeroToOne);
  if (i == colors_.end())
    return 1;
  colors_.erase(i);
  return 0;
}

void ColorGradient::clearColors()
{
  colors_.clear();
}

std::map<double, QColor> ColorGradient::colors() const
{
  return colors_;
}

void ColorGradient::setColors(const std::map<double, QColor>& colors)
{
  colors_ = colors;
  // Remove values before 0. and after 1.
  colors_.erase(colors_.begin(), colors_.lower_bound(0.));
  colors_.erase(colors_.upper_bound(1.), colors_.end());
}

QColor ColorGradient::interpolate(const QColor& lowColor, const QColor& highColor, double low, double val, double high)
{
  const double factor = simCore::getFactor(low, val, high);
  QColor rv;
  rv.setRedF(lowColor.redF() + (highColor.redF() - lowColor.redF()) * factor);
  rv.setGreenF(lowColor.greenF() + (highColor.greenF() - lowColor.greenF()) * factor);
  rv.setBlueF(lowColor.blueF() + (highColor.blueF() - lowColor.blueF()) * factor);
  rv.setAlphaF(lowColor.alphaF() + (highColor.alphaF() - lowColor.alphaF()) * factor);
  return rv;
}

bool ColorGradient::operator==(const ColorGradient& rhs) const
{
  return colors_ == rhs.colors_;
}

bool ColorGradient::operator!=(const ColorGradient& rhs) const
{
  return colors_ != rhs.colors_;
}

}
