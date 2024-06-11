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
#include <cassert>
#include "osgText/Text"
#include "osgEarth/GeoData"
#include "osgEarth/GLUtils"
#include "osgEarth/LineDrawable"
#include "osgEarth/PointDrawable"
#include "simVis/Locator.h"
#include "simVis/Types.h"
#include "simVis/TimeTicksChunk.h"

namespace simVis
{

TimeTicksChunk::TimeTicksChunk(unsigned int maxSize, Type type, double lineLength, double pointSize, unsigned int largeFactor)
  : TrackPointsChunk(maxSize),
  type_(type),
  lineLength_(lineLength),
  pointSize_(pointSize),
  largeSizeFactor_(largeFactor)
{
  allocate_();
}

TimeTicksChunk::~TimeTicksChunk()
{
  lineGroup_ = nullptr;
  line_ = nullptr;
  point_ = nullptr;
  largePoint_ = nullptr;
  line_ = nullptr;
}

bool TimeTicksChunk::addPoint(const Locator& tickLocator, double time, const osg::Vec4& color, bool large)
{
  // ensure that chunk has room for a point, and that chunk has a locator
  if (isFull() || !getLocator())
    return false;

  osg::Matrixd tickMatrix;
  if (offset_ == 0 && count_ == 0)
  {
    // developer must ensure that nodemask is set: LocatorNode's matrix is only sync'd to tickLocator matrix when it has a nodemask
    assert(getNodeMask() != 0);
    tickMatrix = getMatrix();
    world2local_.invert(tickMatrix);
  }
  else
  {
    tickLocator.getLocatorMatrix(tickMatrix);
    if (tickLocator.isEci())
    {
      // world2local always derived from chunk matrix (not new pt matrix)
      // but even chunk matrix changes with each eci rotation, so need recalc on every add
      world2local_.invert(getMatrix());
    }
  }

  // record the timestamp and world coords
  times_[offset_ + count_] = time;
  worldCoords_[offset_ + count_] = tickMatrix;

  // resolve the localized point and append it to the various geometries.
  append_(tickMatrix, color, large);

  // advance the counter and update the psets.
  count_++;
  updatePrimitiveSets_();

  return true;
}

void TimeTicksChunk::allocate_()
{
  // clear existing:
  removeChildren(0, getNumChildren());

  // timestamp vector.
  times_.resize(maxSize_);
  times_[0] = 0.0;
  worldCoords_.resize(maxSize_);

  // pointers into the points list.
  offset_ = 0;
  count_  = 0;

  if (type_ == POINT_TICKS)
  {
    // large points
    largePoint_ = new osgEarth::PointDrawable();
    largePoint_->setDataVariance(osg::Object::DYNAMIC);
    largePoint_->allocate(maxSize_);
    largePoint_->setColor(osg::Vec4f(0.f, 0.f, 0.f, 0.f));
    largePoint_->finish();
    largePoint_->setFirst(offset_);
    largePoint_->setCount(count_);
    addChild(largePoint_.get());
    osgEarth::GLUtils::setPointSize(largePoint_->getOrCreateStateSet(), pointSize_ * largeSizeFactor_, osg::StateAttribute::ON);

    // points
    point_ = new osgEarth::PointDrawable();
    point_->setDataVariance(osg::Object::DYNAMIC);
    point_->allocate(maxSize_);
    point_->setColor(simVis::Color::White);
    point_->finish();
    point_->setFirst(offset_);
    point_->setCount(count_);
    addChild(point_.get());
    osgEarth::GLUtils::setPointSize(point_->getOrCreateStateSet(), pointSize_, osg::StateAttribute::ON);
  }
  else if (type_ == LINE_TICKS)
  {
    // geode to hold all line geometry:
    lineGroup_ = new osgEarth::LineGroup();
    addChild(lineGroup_.get());

    // cross hatch line ticks
    line_ = new osgEarth::LineDrawable(GL_LINES);
    line_->setDataVariance(osg::Object::DYNAMIC);
    line_->allocate(2 * maxSize_);
    lineGroup_->addChild(line_.get());
  }

  // reset to identity matrices
  world2local_ = osg::Matrixd::identity();
}

void TimeTicksChunk::append_(const osg::Matrixd& matrix, const osg::Vec4f& color, bool large)
{
  // insertion index:
  const unsigned int i = offset_ + count_;

  if (type_ == POINT_TICKS)
  {
    // calculate the local point.
    const osg::Vec3d& world = matrix.getTrans();
    const osg::Vec3f local = world * world2local_;
    if (large)
    {
      largePoint_->setVertex(i, local);
      largePoint_->setColor(i, color);
    }
    point_->setVertex(i, local);
    point_->setColor(i, color);
  }
  else if (type_ == LINE_TICKS)
  {
    // add a new cross hatch tick
    const osg::Matrixd& posMatrix = matrix * world2local_;
    const double width = lineLength_ * (large ? largeSizeFactor_ : 1);

    const osg::Vec3f left = osg::Vec3d(-width, 0.0, 0.0) * posMatrix;
    const osg::Vec3f right = osg::Vec3d(width, 0.0, 0.0) * posMatrix;

    line_->setVertex(2 * i, left);
    line_->setVertex(2 * i + 1, right);

    for (unsigned int c = 0; c < 2; ++c)
      line_->setColor(2 * i + c, color);
  }
}

void TimeTicksChunk::fixGraphicsAfterRemoval_()
{
  // no-op
}

void TimeTicksChunk::updatePrimitiveSets_()
{
  if (type_ == POINT_TICKS)
  {
    point_->setFirst(offset_);
    point_->setCount(count_);
    largePoint_->setFirst(offset_);
    largePoint_->setCount(count_);
  }
  else if (type_ == LINE_TICKS)
  {
    line_->setFirst(2 * offset_);
    line_->setCount(2 * count_);
  }
}

}
