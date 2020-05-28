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
#include <QVariantMap>
#include "simCore/Common/SDKAssert.h"
#include "simQt/ColorGradient.h"

namespace {

int testGradient()
{
  int rv = 0;
  simQt::ColorGradient grad;
  rv += SDK_ASSERT(grad.colors().size() == 5);
  rv += SDK_ASSERT(grad.colorAt(0.) == Qt::blue);
  rv += SDK_ASSERT(grad.colorAt(0.25) == Qt::cyan);
  rv += SDK_ASSERT(grad.colorAt(1.) == Qt::red);

  // Test clamping
  rv += SDK_ASSERT(grad.colorAt(-0.5) == Qt::blue);
  rv += SDK_ASSERT(grad.colorAt(1.5) == Qt::red);

  // Test interpolation
  QColor color = grad.colorAt(0.125);
  rv += SDK_ASSERT(color.red() == 0);
  rv += SDK_ASSERT(color.green() == 128);
  rv += SDK_ASSERT(color.blue() == 255);
  rv += SDK_ASSERT(color.alpha() == 255);

  // Test replacement
  rv += SDK_ASSERT(grad.setColor(0.125, Qt::darkYellow) == 0);
  rv += SDK_ASSERT(grad.colorAt(0.125) == Qt::darkYellow);

  // Test replacement of 0.0 and 1.0
  rv += SDK_ASSERT(grad.setColor(0., Qt::darkGreen) == 0);
  rv += SDK_ASSERT(grad.setColor(1., Qt::darkMagenta) == 0);
  rv += SDK_ASSERT(grad.colorAt(0.) == Qt::darkGreen);
  rv += SDK_ASSERT(grad.colorAt(1.) == Qt::darkMagenta);

  // Test that replacement of values outside 0 and 1 do not add points
  rv += SDK_ASSERT(grad.colors().size() == 6);
  rv += SDK_ASSERT(grad.setColor(-0.5, Qt::darkGreen) != 0);
  rv += SDK_ASSERT(grad.setColor(1.5, Qt::darkMagenta) != 0);

  // Remove color
  rv += SDK_ASSERT(grad.colors().size() == 6);
  rv += SDK_ASSERT(grad.removeColor(0.1) != 0); // does not exist
  rv += SDK_ASSERT(grad.removeColor(1.5) != 0); // does not exist
  rv += SDK_ASSERT(grad.removeColor(-1.5) != 0); // does not exist
  rv += SDK_ASSERT(grad.removeColor(0.125) == 0);
  rv += SDK_ASSERT(grad.colors().size() == 5);
  rv += SDK_ASSERT(grad.colorAt(0.125) != Qt::darkYellow);

  // Test 0 control points, and outside-0 points
  grad.clearColors();
  rv += SDK_ASSERT(grad.colors().size() == 0);
  rv += SDK_ASSERT(grad.colorAt(0.) == Qt::black);
  rv += SDK_ASSERT(grad.colorAt(1.) == Qt::black);
  const QColor midGray = qRgba(128, 128, 128, 255);
  rv += SDK_ASSERT(grad.setColor(0.5, midGray) == 0);
  rv += SDK_ASSERT(grad.colors().size() == 1);
  rv += SDK_ASSERT(grad.colorAt(0.) == midGray);
  rv += SDK_ASSERT(grad.colorAt(0.4) == midGray);
  rv += SDK_ASSERT(grad.colorAt(0.5) == midGray);
  rv += SDK_ASSERT(grad.colorAt(0.6) == midGray);
  rv += SDK_ASSERT(grad.colorAt(1.) == midGray);

  // Add one at 0.75
  const QColor darkGray = qRgba(192, 192, 192, 255);
  rv += SDK_ASSERT(grad.setColor(0.75, darkGray) == 0);
  rv += SDK_ASSERT(grad.colorAt(0.) == midGray);
  rv += SDK_ASSERT(grad.colorAt(0.4) == midGray);
  rv += SDK_ASSERT(grad.colorAt(0.5) == midGray);
  rv += SDK_ASSERT(grad.colorAt(0.75) == darkGray);
  rv += SDK_ASSERT(grad.colorAt(0.85) == darkGray);
  rv += SDK_ASSERT(grad.colorAt(1.) == darkGray);

  // Test setColors with points outside 0/1
  std::map<double, QColor> colorMap;
  colorMap[-1.] = Qt::white;
  colorMap[2.] = Qt::white;
  rv += SDK_ASSERT(grad.colors().size() == 2);
  grad.setColors(colorMap);
  rv += SDK_ASSERT(grad.colors().size() == 0);
  rv += SDK_ASSERT(grad.colorAt(0.) == Qt::black);
  rv += SDK_ASSERT(grad.colorAt(0.5) == Qt::black);
  rv += SDK_ASSERT(grad.colorAt(1.) == Qt::black);

  // Test constructor with outside-0/1 points
  colorMap[0.5] = Qt::green;
  grad = simQt::ColorGradient(colorMap);
  rv += SDK_ASSERT(grad.colors().size() == 1);
  rv += SDK_ASSERT(grad.colorAt(0.) == Qt::green);
  rv += SDK_ASSERT(grad.colorAt(0.5) == Qt::green);
  rv += SDK_ASSERT(grad.colorAt(1.) == Qt::green);

  return rv;
}

int testFactories()
{
  int rv = 0;
  rv += SDK_ASSERT(simQt::ColorGradient().colors().size() == 5);
  rv += SDK_ASSERT(simQt::ColorGradient::newDefaultGradient().colors().size() == 5);
  rv += SDK_ASSERT(simQt::ColorGradient::newDarkGradient().colors().size() == 7);
  rv += SDK_ASSERT(simQt::ColorGradient::newGreyscaleGradient().colors().size() == 2);
  rv += SDK_ASSERT(simQt::ColorGradient::newDopplerGradient().colors().size() == 11);
  return rv;
}

int testVariant()
{
  // Test the expected usage in Plot-XY with regards to QVariantMap
  QVariantMap vMap;
  vMap["grey"] = QVariant::fromValue(simQt::ColorGradient::newGreyscaleGradient());
  const auto& grey = vMap["grey"].value<simQt::ColorGradient>();
  int rv = 0;
  rv += SDK_ASSERT(grey.colors().size() == 2);
  rv += SDK_ASSERT(grey.colorAt(0.) == Qt::black);
  const QColor color = grey.colorAt(0.5);
  rv += SDK_ASSERT(color.red() == 128);
  rv += SDK_ASSERT(color.green() == 128);
  rv += SDK_ASSERT(color.blue() == 128);
  rv += SDK_ASSERT(color.alpha() == 255);
  rv += SDK_ASSERT(grey.colorAt(1.) == Qt::white);
  return rv;
}

}

int GradientTest(int argc, char* argv[])
{
  int rv = 0;
  rv += testGradient();
  rv += testFactories();
  rv += testVariant();
  return rv;
}
