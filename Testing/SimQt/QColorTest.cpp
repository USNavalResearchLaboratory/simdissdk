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
#include <cmath>
#include "simCore/Common/SDKAssert.h"
#include "simQt/QtConversion.h"

namespace {

bool closeEnough(const osg::Vec4f& c1, const osg::Vec4f& c2)
{
  static const float acceptable = 1.f / 255.f;
  return fabs(c1.r() - c2.r()) <= acceptable &&
    fabs(c1.g() - c2.g()) <= acceptable &&
    fabs(c1.b() - c2.b()) <= acceptable &&
    fabs(c1.a() - c2.a()) <= acceptable;
}
bool closeEnough(const QColor& c1, const QColor& c2)
{
  static const int acceptable = 1;
  return std::abs(c1.red() - c2.red()) <= acceptable &&
    std::abs(c1.green() - c2.green()) <= acceptable &&
    std::abs(c1.blue() - c2.blue()) <= acceptable &&
    std::abs(c1.alpha() - c2.alpha()) <= acceptable;
}

}

int QColorTest(int argc, char* argv[])
{
  int rv = 0;
  // Test variations in each octet; test 0, 128, and 255 (corresponding to 0.f, 0.5f, 1.f)
  // Note that we have an acceptable variation of 1 / 255 for float values, or 1 of 255 for integer values.
  rv += SDK_ASSERT(closeEnough(simQt::getQtColorFromOsg(osg::Vec4f(1.f, 1.f, 1.f, 1.f)), QColor::fromRgb(255, 255, 255, 255)));
  rv += SDK_ASSERT(closeEnough(simQt::getQtColorFromOsg(osg::Vec4f(1.f, 1.f, 1.f, 0.f)), QColor::fromRgb(255, 255, 255, 0)));
  rv += SDK_ASSERT(closeEnough(simQt::getQtColorFromOsg(osg::Vec4f(1.f, 0.f, 1.f, 0.5f)), QColor::fromRgb(255, 0, 255, 128)));
  rv += SDK_ASSERT(closeEnough(simQt::getQtColorFromOsg(osg::Vec4f(1.f, 1.f, 0.f, 1.f)), QColor::fromRgb(255, 255, 0, 255)));
  rv += SDK_ASSERT(closeEnough(simQt::getQtColorFromOsg(osg::Vec4f(0.f, 1.f, 1.f, 1.f)), QColor::fromRgb(0, 255, 255, 255)));
  rv += SDK_ASSERT(closeEnough(simQt::getOsgColorFromQt(QColor::fromRgb(255, 255, 255, 255)), osg::Vec4f(1.f, 1.f, 1.f, 1.f)));
  rv += SDK_ASSERT(closeEnough(simQt::getOsgColorFromQt(QColor::fromRgb(0, 255, 255, 255)), osg::Vec4f(0.f, 1.f, 1.f, 1.f)));
  rv += SDK_ASSERT(closeEnough(simQt::getOsgColorFromQt(QColor::fromRgb(128, 255, 0, 255)), osg::Vec4f(0.5f, 1.f, 0.f, 1.f)));
  rv += SDK_ASSERT(closeEnough(simQt::getOsgColorFromQt(QColor::fromRgb(255, 0, 255, 255)), osg::Vec4f(1.f, 0.f, 1.f, 1.f)));
  rv += SDK_ASSERT(closeEnough(simQt::getOsgColorFromQt(QColor::fromRgb(255, 255, 255, 0)), osg::Vec4f(1.f, 1.f, 1.f, 0.f)));

  return rv;
}

