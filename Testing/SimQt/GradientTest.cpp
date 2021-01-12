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
  rv += SDK_ASSERT(grad.colorAt(0.f) == Qt::blue);
  rv += SDK_ASSERT(grad.colorAt(0.25f) == Qt::cyan);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::red);

  // Test clamping
  rv += SDK_ASSERT(grad.colorAt(-0.5f) == Qt::blue);
  rv += SDK_ASSERT(grad.colorAt(1.5f) == Qt::red);

  // Test interpolation
  QColor color = grad.colorAt(0.125f);
  rv += SDK_ASSERT(color.red() == 0);
  rv += SDK_ASSERT(color.green() == 127);
  rv += SDK_ASSERT(color.blue() == 255);
  rv += SDK_ASSERT(color.alpha() == 255);

  // Test replacement
  rv += SDK_ASSERT(grad.setColor(0.125f, Qt::darkYellow) == 0);
  rv += SDK_ASSERT(grad.colorAt(0.125f) == Qt::darkYellow);

  // Test replacement of 0.0 and 1.0
  rv += SDK_ASSERT(grad.setColor(0.f, Qt::darkGreen) == 0);
  rv += SDK_ASSERT(grad.setColor(1.f, Qt::darkMagenta) == 0);
  rv += SDK_ASSERT(grad.colorAt(0.f) == Qt::darkGreen);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::darkMagenta);

  // Test that replacement of values outside 0 and 1 do not add points
  rv += SDK_ASSERT(grad.colors().size() == 6);
  rv += SDK_ASSERT(grad.setColor(-0.5f, Qt::darkGreen) != 0);
  rv += SDK_ASSERT(grad.setColor(1.5f, Qt::darkMagenta) != 0);

  // Remove color
  rv += SDK_ASSERT(grad.colors().size() == 6);
  rv += SDK_ASSERT(grad.removeColor(0.1f) != 0); // does not exist
  rv += SDK_ASSERT(grad.removeColor(1.5f) != 0); // does not exist
  rv += SDK_ASSERT(grad.removeColor(-1.5f) != 0); // does not exist
  rv += SDK_ASSERT(grad.removeColor(0.125f) == 0);
  rv += SDK_ASSERT(grad.colors().size() == 5);
  rv += SDK_ASSERT(grad.colorAt(0.125f) != Qt::darkYellow);

  // Test a "cleared" gradient
  grad.clearColors();
  rv += SDK_ASSERT(grad.colors().size() == 2);
  rv += SDK_ASSERT(grad.colorAt(0.f) == Qt::white);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::white);

  // Test setColors with points outside 0/1
  std::map<float, QColor> colorMap;
  colorMap[-1.f] = Qt::white;
  colorMap[0.2f] = Qt::red;
  colorMap[0.8f] = Qt::green;
  colorMap[2.f] = Qt::white;
  rv += SDK_ASSERT(grad.colors().size() == 2);
  grad.setColors(colorMap);
  rv += SDK_ASSERT(grad.colors().size() == 2);
  rv += SDK_ASSERT(grad.colorAt(0.f) == Qt::red);
  QColor mid = grad.colorAt(0.5f);
  rv += SDK_ASSERT(mid.red() == 127);
  rv += SDK_ASSERT(mid.green() == 127);
  rv += SDK_ASSERT(mid.blue() == 0);
  rv += SDK_ASSERT(mid.alpha() == 255);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::green);

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
  rv += SDK_ASSERT(grey.colorAt(0.f) == Qt::black);
  const QColor color = grey.colorAt(0.5f);
  rv += SDK_ASSERT(color.red() == 127);
  rv += SDK_ASSERT(color.green() == 127);
  rv += SDK_ASSERT(color.blue() == 127);
  rv += SDK_ASSERT(color.alpha() == 255);
  rv += SDK_ASSERT(grey.colorAt(1.) == Qt::white);
  return rv;
}

int testDiscrete()
{
  int rv = 0;

  std::map<float, QColor> colorMap;
  colorMap[0.2f] = Qt::red;
  colorMap[0.8f] = Qt::green;
  simQt::ColorGradient grad(colorMap);
  QColor col = grad.colorAt(0.5f);
  rv += SDK_ASSERT(col.red() == 127);
  rv += SDK_ASSERT(col.green() == 127);
  rv += SDK_ASSERT(col.blue() == 0);
  rv += SDK_ASSERT(col.alpha() == 255);
  grad.setDiscrete(true);
  // Discrete at 0.5 is red
  col = grad.colorAt(0.5f);
  rv += SDK_ASSERT(col == Qt::red);
  // Discrete at 0.0 is red
  col = grad.colorAt(0.f);
  rv += SDK_ASSERT(col == Qt::red);
  // Discrete at 0.8 is green
  col = grad.colorAt(0.8f);
  rv += SDK_ASSERT(col == Qt::green);
  // Discrete at 1.0 is green
  col = grad.colorAt(1.f);
  rv += SDK_ASSERT(col == Qt::green);

  return rv;
}

}

int GradientTest(int argc, char* argv[])
{
  int rv = 0;
  rv += testGradient();
  rv += testFactories();
  rv += testVariant();
  rv += testDiscrete();
  return rv;
}
