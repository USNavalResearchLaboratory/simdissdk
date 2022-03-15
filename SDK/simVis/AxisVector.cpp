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
#include "osg/Geode"
#include "osgEarth/LineDrawable"
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/AxisVector.h"

namespace simVis
{

// --------------------------------------------------------------------------
namespace
{
/// Number of points in the subdivided line strip
const unsigned int AXIS_NUM_POINTS_PER_LINE_STRIP = 4;
}

// --------------------------------------------------------------------------
AxisVector::AxisVector()
  : MatrixTransform(),
    lineWidth_(2.f),
    axisLengths_(1.f, 1.f, 1.f)
{
  setName("AxisVector");
  init_();
}

AxisVector::AxisVector(const AxisVector &rhs, const osg::CopyOp& copyOp)
  : MatrixTransform(rhs, copyOp),
    lineWidth_(rhs.lineWidth_),
    axisLengths_(rhs.axisLengths_)
{
  init_();
}

AxisVector::~AxisVector()
{
}

void AxisVector::init_()
{
  lineGroup_ = new osgEarth::LineGroup();
  createAxisVectors_();
  addChild(lineGroup_.get());
}

void AxisVector::setAxisLengths(osg::Vec3f axisLengths, bool force)
{
  if (force || axisLengths != axisLengths_)
  {
    MatrixTransform::setMatrix(osg::Matrix::scale(axisLengths));
    axisLengths_ = axisLengths;
  }
}

void AxisVector::setAxisLengths(float xLength, float yLength, float zLength, bool force)
{
  setAxisLengths(osg::Vec3f(xLength, yLength, zLength), force);
}

osg::Vec3f AxisVector::axisLengths() const
{
  return axisLengths_;
}

void AxisVector::setLineWidth(float lineWidth)
{
  lineWidth_ = lineWidth;
  lineGroup_->getLineDrawable(0)->setLineWidth(lineWidth);
  lineGroup_->getLineDrawable(1)->setLineWidth(lineWidth);
  lineGroup_->getLineDrawable(2)->setLineWidth(lineWidth);
}

float AxisVector::lineWidth() const
{
  return lineWidth_;
}

void AxisVector::setColors(const simVis::Color& x, const simVis::Color& y, const simVis::Color& z)
{
  // Avoid rebuilding unnecessarily
  if (x == xColor() && y == yColor() && z == zColor())
    return;

  lineGroup_->getLineDrawable(0)->setColor(x);
  lineGroup_->getLineDrawable(1)->setColor(y);
  lineGroup_->getLineDrawable(2)->setColor(z);
}

simVis::Color AxisVector::xColor() const
{
  return lineGroup_->getLineDrawable(0)->getColor();
}

simVis::Color AxisVector::yColor() const
{
  return lineGroup_->getLineDrawable(1)->getColor();
}

simVis::Color AxisVector::zColor() const
{
  return lineGroup_->getLineDrawable(2)->getColor();
}

void AxisVector::setPositionOrientation(const osg::Vec3f& pos, const osg::Vec3f& vec)
{
  osg::Matrixf rot;
  if (vec != osg::Vec3f())
  {
    // determine a rotation matrix that rotates x-axis vector to the specified vector
    rot.makeRotate(osg::X_AXIS, vec);
  }
  rot.postMultTranslate(pos);
  rot.preMultScale(axisLengths_);
  setMatrix(rot);
}

void AxisVector::createAxisVectors_() const
{
  // draw x axis vector
  osgEarth::LineDrawable* line = new osgEarth::LineDrawable(GL_LINE_STRIP);
  line->setName("simVis::AxisVector");
  line->allocate(AXIS_NUM_POINTS_PER_LINE_STRIP);
  VectorScaling::generatePoints(*line, osg::Vec3(), osg::X_AXIS);
  line->setColor(simVis::Color::Yellow);
  line->setLineWidth(lineWidth_);
  lineGroup_->addChild(line);

  // draw y axis vector
  line = new osgEarth::LineDrawable(GL_LINE_STRIP);
  line->setName("simVis::AxisVector");
  line->allocate(AXIS_NUM_POINTS_PER_LINE_STRIP);
  VectorScaling::generatePoints(*line, osg::Vec3(), osg::Y_AXIS);
  line->setColor(simVis::Color::Fuchsia);
  line->setLineWidth(lineWidth_);
  lineGroup_->addChild(line);

  // draw z axis vector
  line = new osgEarth::LineDrawable(GL_LINE_STRIP);
  line->setName("simVis::AxisVector");
  line->allocate(AXIS_NUM_POINTS_PER_LINE_STRIP);
  VectorScaling::generatePoints(*line, osg::Vec3(), osg::Z_AXIS);
  line->setColor(simVis::Color::Aqua);
  line->setLineWidth(lineWidth_);
  lineGroup_->addChild(line);
}

}
