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
#include <QVariantMap>
#include "simCore/Common/SDKAssert.h"
#include "simQt/ColorGradient.h"

namespace {

int testGradient()
{
  int rv = 0;

  simQt::ColorGradient grad;

  // Spot check on colors based on expectation of default gradient
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

  // Validate the stops before editing the gradient
  rv += SDK_ASSERT(grad.numControlColors() == 7); // 5 stops, plus 0 and 100%
  rv += SDK_ASSERT(grad.controlColorPct(2) == 0.f);
  rv += SDK_ASSERT(grad.controlColorPct(6) == 1.f);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::red);

  rv += SDK_ASSERT(grad.addControlColor(0.125f, Qt::darkYellow) == 7);
  rv += SDK_ASSERT(grad.colorAt(0.125f) == Qt::darkYellow);
  rv += SDK_ASSERT(grad.setControlColor(7, 0.125f, Qt::darkMagenta) == 0);
  rv += SDK_ASSERT(grad.colorAt(0.125f) == Qt::darkMagenta);
  rv += SDK_ASSERT(grad.setControlColor(7, 0.125f, Qt::darkYellow) == 0);

  // Remove red at 100% (we still have the red at 100% in slot 2)
  rv += SDK_ASSERT(grad.numControlColors() == 8);
  rv += SDK_ASSERT(grad.removeControlColor(8) != 0); // Invalid removal
  rv += SDK_ASSERT(grad.removeControlColor(6) == 0);
  rv += SDK_ASSERT(grad.numControlColors() == 7);
  rv += SDK_ASSERT(grad.colorAt(0.125f) == Qt::darkYellow);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::red); // due to slot 1
  // Ensure that the indices changed
  rv += SDK_ASSERT(grad.controlColorPct(6) == 0.125f);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::darkYellow);
  // Reset the value back so we're back to red
  rv += SDK_ASSERT(grad.setControlColor(6, 1.f, Qt::red) == 0);
  rv += SDK_ASSERT(grad.colorAt(0.125f) != Qt::darkYellow);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::red);
  rv += SDK_ASSERT(grad.controlColorPct(6) == 1.f);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::red);

  // Cannot remove or reassign control colors 0 and 1
  rv += SDK_ASSERT(grad.setControlColor(0, 0.5f, Qt::darkYellow) == 0);
  rv += SDK_ASSERT(grad.controlColorPct(0) == 0.f); // 0.5f did not take hold
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::darkYellow); // color assignment did take hold
  rv += SDK_ASSERT(grad.setControlColor(1, 0.5f, Qt::darkYellow) == 0);
  rv += SDK_ASSERT(grad.controlColorPct(1) == 1.f); // 0.5f did not take hold
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::darkYellow); // color assignment did take hold

  // Copy assignment
  rv += SDK_ASSERT(grad.setControlColor(6, 1.f, Qt::gray) == 0);
  simQt::ColorGradient grad2(grad);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::gray);
  rv += SDK_ASSERT(grad2.controlColor(6) == Qt::gray);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::gray);
  rv += SDK_ASSERT(grad2.colorAt(1.f) == Qt::gray);
  // Ensure that changing grad doesn't impact grad2
  rv += SDK_ASSERT(grad.setControlColor(6, 1.f, Qt::darkBlue) == 0);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.controlColor(6) == Qt::gray);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.colorAt(1.f) == Qt::gray);
  // Ensure that changing grad2 doesn't impact grad
  rv += SDK_ASSERT(grad2.setControlColor(6, 1.f, Qt::darkYellow) == 0);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.controlColor(6) == Qt::darkYellow);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.colorAt(1.f) == Qt::darkYellow);

  // Reset, and try assignment operator
  grad = simQt::ColorGradient::newDefaultGradient();
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::red);
  rv += SDK_ASSERT(grad2.controlColor(6) != Qt::red);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::red);
  rv += SDK_ASSERT(grad2.colorAt(1.f) != Qt::red);
  grad2 = grad;
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::red);
  rv += SDK_ASSERT(grad2.controlColor(6) == Qt::red);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::red);
  rv += SDK_ASSERT(grad2.colorAt(1.f) == Qt::red);
  // Ensure that changing grad doesn't impact grad2
  rv += SDK_ASSERT(grad.setControlColor(6, 1.f, Qt::darkBlue) == 0);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.controlColor(6) == Qt::red);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.colorAt(1.f) == Qt::red);
  // Ensure that changing grad2 doesn't impact grad
  rv += SDK_ASSERT(grad2.setControlColor(6, 1.f, Qt::darkYellow) == 0);
  rv += SDK_ASSERT(grad.controlColor(6) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.controlColor(6) == Qt::darkYellow);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::darkBlue);
  rv += SDK_ASSERT(grad2.colorAt(1.f) == Qt::darkYellow);

  // Test a "cleared" gradient
  grad.clearControlColors();
  rv += SDK_ASSERT(grad.numControlColors() == 2);
  rv += SDK_ASSERT(grad.colorAt(0.f) == Qt::white);
  rv += SDK_ASSERT(grad.colorAt(1.f) == Qt::white);
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::white);
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::white);

  return rv;
}

int testFactories()
{
  int rv = 0;
  rv += SDK_ASSERT(simQt::ColorGradient().numControlColors() == 7);
  rv += SDK_ASSERT(simQt::ColorGradient::newDefaultGradient().numControlColors() == 7);
  rv += SDK_ASSERT(simQt::ColorGradient::newDarkGradient().numControlColors() == 7);
  rv += SDK_ASSERT(simQt::ColorGradient::newGreyscaleGradient().numControlColors() == 4);
  rv += SDK_ASSERT(simQt::ColorGradient::newDopplerGradient().numControlColors() == 12);

  return rv;
}

int testVariant()
{
  // Test the expected usage in Plot-XY with regards to QVariantMap
  QVariantMap vMap;
  vMap["grey"] = QVariant::fromValue(simQt::ColorGradient::newGreyscaleGradient());
  const auto& grey = vMap["grey"].value<simQt::ColorGradient>();
  int rv = 0;
  rv += SDK_ASSERT(grey.colorAt(0.f) == Qt::black);
  const QColor& color = grey.colorAt(0.5f);
  rv += SDK_ASSERT(color.red() == 127);
  rv += SDK_ASSERT(color.green() == 127);
  rv += SDK_ASSERT(color.blue() == 127);
  rv += SDK_ASSERT(color.alpha() == 255);
  rv += SDK_ASSERT(grey.colorAt(1.) == Qt::white);
  rv += SDK_ASSERT(grey.numControlColors() == 4);
  return rv;
}

int testDiscrete()
{
  int rv = 0;

  std::map<float, QColor> colorMap;
  colorMap[0.2f] = Qt::red;
  colorMap[0.8f] = Qt::green;
  simQt::ColorGradient grad;
  grad.importColorMap(colorMap);
  rv += SDK_ASSERT(grad.numControlColors() == 4);
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::green);
  rv += SDK_ASSERT(grad.controlColor(2) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(3) == Qt::green);
  rv += SDK_ASSERT(grad.controlColorPct(2) == 0.2f);
  rv += SDK_ASSERT(grad.controlColorPct(3) == 0.8f);

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

int testImportColorVector()
{
  int rv = 0;
  simQt::ColorGradient grad;

  // Import vector without end points
  grad.importColorVector({
    {0.8f, Qt::blue},
    {0.2f, Qt::red},
    });
  rv += SDK_ASSERT(grad.numControlColors() == 4);
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::blue);
  // Though not defined, through white box testing we know that the internal order gets
  // rearranged from smallest to greatest for non-0/1 index
  rv += SDK_ASSERT(grad.controlColor(2) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(3) == Qt::blue);
  rv += SDK_ASSERT(grad.controlColorPct(2) == 0.2f);
  rv += SDK_ASSERT(grad.controlColorPct(3) == 0.8f);

  // Import vector with end points
  grad.importColorVector({
    {0.8f, Qt::blue},
    {0.0f, Qt::red},
    {1.0f, Qt::green},
    });
  rv += SDK_ASSERT(grad.numControlColors() == 5);
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::green);
  rv += SDK_ASSERT(grad.controlColor(2) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(3) == Qt::blue);
  rv += SDK_ASSERT(grad.controlColor(4) == Qt::green);
  rv += SDK_ASSERT(grad.controlColorPct(2) == 0.f);
  rv += SDK_ASSERT(grad.controlColorPct(3) == 0.8f);
  rv += SDK_ASSERT(grad.controlColorPct(4) == 1.f);

  // Import empty vector
  grad.importColorVector({});
  rv += SDK_ASSERT(grad.numControlColors() == 2);
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::white);
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::white);

  // Import vector with values outside range
  grad.importColorVector({
    {1.8f, Qt::blue},
    {0.2f, Qt::red},
    });
  rv += SDK_ASSERT(grad.numControlColors() == 3);
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::red);
  rv += SDK_ASSERT(grad.controlColor(2) == Qt::red);
  rv += SDK_ASSERT(grad.controlColorPct(2) == 0.2f);

  // Import vector where only value is outside range
  grad.importColorVector({
    {1.8f, Qt::blue},
    });
  rv += SDK_ASSERT(grad.numControlColors() == 2);
  rv += SDK_ASSERT(grad.controlColor(0) == Qt::white);
  rv += SDK_ASSERT(grad.controlColor(1) == Qt::white);

  return rv;
}

int testEffectiveColorMap()
{
  int rv = 0;

  const auto& m = simQt::ColorGradient::newGreyscaleGradient().effectiveColorMap();
  rv += SDK_ASSERT(m.size() == 2);
  rv += SDK_ASSERT(m.begin()->first == 0.f);
  rv += SDK_ASSERT(m.begin()->second == osg::Vec4(0.f, 0.f, 0.f, 1.f));
  rv += SDK_ASSERT(m.rbegin()->first == 1.f);
  rv += SDK_ASSERT(m.rbegin()->second == osg::Vec4(1.f, 1.f, 1.f, 1.f));
  return rv;
}

int testEquality()
{
  int rv = 0;
  rv += SDK_ASSERT(simQt::ColorGradient() == simQt::ColorGradient::newDefaultGradient());
  rv += SDK_ASSERT(!(simQt::ColorGradient() != simQt::ColorGradient::newDefaultGradient()));
  rv += SDK_ASSERT(simQt::ColorGradient::newDefaultGradient() == simQt::ColorGradient::newDefaultGradient());
  rv += SDK_ASSERT(!(simQt::ColorGradient::newDefaultGradient() != simQt::ColorGradient::newDefaultGradient()));
  rv += SDK_ASSERT(!(simQt::ColorGradient::newDefaultGradient() == simQt::ColorGradient::newDarkGradient()));
  rv += SDK_ASSERT(simQt::ColorGradient::newDefaultGradient() != simQt::ColorGradient::newDarkGradient());

  simQt::ColorGradient grad1;
  simQt::ColorGradient grad2;
  rv += SDK_ASSERT(grad1 == grad2);
  rv += SDK_ASSERT(!(grad1 != grad2));
  grad1.setDiscrete(true);
  rv += SDK_ASSERT(!(grad1 == grad2));
  rv += SDK_ASSERT(grad1 != grad2);
  grad1.setDiscrete(false);
  rv += SDK_ASSERT(grad1 == grad2);
  rv += SDK_ASSERT(!(grad1 != grad2));
  grad1.setDiscrete(true);
  grad2.setDiscrete(true);
  rv += SDK_ASSERT(grad1 == grad2);
  rv += SDK_ASSERT(!(grad1 != grad2));
  return rv;
}

int testCompression()
{
  int rv = 0;

  simQt::ColorGradient orig;

  // We know the default effective gradient
  rv += SDK_ASSERT(orig.colorAt(0.f) == Qt::blue);
  rv += SDK_ASSERT(orig.colorAt(0.25f) == Qt::cyan);
  rv += SDK_ASSERT(orig.colorAt(0.5f) == Qt::green);
  rv += SDK_ASSERT(orig.colorAt(0.75f) == Qt::yellow);
  rv += SDK_ASSERT(orig.colorAt(1.f) == Qt::red);
  // We also know the boundaries at 0% and 100%
  rv += SDK_ASSERT(orig.controlColor(0) == Qt::black);
  rv += SDK_ASSERT(orig.controlColor(1) == Qt::red);

  // No changes on compress(0,1)
  simQt::ColorGradient compress = orig.compress(0.f, 1.f);
  rv += SDK_ASSERT(compress.colorAt(0.f) == Qt::blue);
  rv += SDK_ASSERT(compress.colorAt(0.25f) == Qt::cyan);
  rv += SDK_ASSERT(compress.colorAt(0.5f) == Qt::green);
  rv += SDK_ASSERT(compress.colorAt(0.75f) == Qt::yellow);
  rv += SDK_ASSERT(compress.colorAt(1.f) == Qt::red);
  rv += SDK_ASSERT(compress.controlColor(0) == Qt::black);
  rv += SDK_ASSERT(compress.controlColor(1) == Qt::red);

  // Compressing backwards results in a backwards scale
  compress = orig.compress(1.f, 0.f);
  rv += SDK_ASSERT(compress.colorAt(0.f) == Qt::red);
  rv += SDK_ASSERT(compress.colorAt(0.25f) == Qt::yellow);
  rv += SDK_ASSERT(compress.colorAt(0.5f) == Qt::green);
  rv += SDK_ASSERT(compress.colorAt(0.75f) == Qt::cyan);
  rv += SDK_ASSERT(compress.colorAt(1.f) == Qt::blue);
  rv += SDK_ASSERT(compress.controlColor(0) == Qt::red);
  rv += SDK_ASSERT(compress.controlColor(1) == Qt::black);

  // Compress bottom only
  compress = orig.compress(0.5f, 1.f);
  rv += SDK_ASSERT(compress.colorAt(0.f) == Qt::black);
  rv += SDK_ASSERT(compress.colorAt(0.5f) == Qt::blue);
  rv += SDK_ASSERT(compress.colorAt(0.625f) == Qt::cyan);
  rv += SDK_ASSERT(compress.colorAt(0.75f) == Qt::green);
  rv += SDK_ASSERT(compress.colorAt(0.875f) == Qt::yellow);
  rv += SDK_ASSERT(compress.colorAt(1.f) == Qt::red);

  // Compress top only
  compress = orig.compress(0.f, 0.5f);
  rv += SDK_ASSERT(compress.colorAt(0.0f) == Qt::blue);
  rv += SDK_ASSERT(compress.colorAt(0.125f) == Qt::cyan);
  rv += SDK_ASSERT(compress.colorAt(0.25f) == Qt::green);
  rv += SDK_ASSERT(compress.colorAt(0.375f) == Qt::yellow);
  rv += SDK_ASSERT(compress.colorAt(0.5f) == Qt::red);
  rv += SDK_ASSERT(compress.colorAt(1.f) == Qt::red);

  // Compress both sides
  compress = orig.compress(0.25f, 0.75f);
  rv += SDK_ASSERT(compress.colorAt(0.0f) == Qt::black);
  rv += SDK_ASSERT(compress.colorAt(0.25f) == Qt::blue);
  rv += SDK_ASSERT(compress.colorAt(0.375f) == Qt::cyan);
  rv += SDK_ASSERT(compress.colorAt(0.5f) == Qt::green);
  rv += SDK_ASSERT(compress.colorAt(0.625f) == Qt::yellow);
  rv += SDK_ASSERT(compress.colorAt(0.75f) == Qt::red);
  rv += SDK_ASSERT(compress.colorAt(1.f) == Qt::red);

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
  rv += testImportColorVector();
  rv += testEffectiveColorMap();
  rv += testEquality();
  rv += testCompression();
  return rv;
}
