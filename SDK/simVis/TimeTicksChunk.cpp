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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include "osg/Geode"
#include "osg/Geometry"
#include "osgText/Text"
#include "osgEarth/GeoData"
#include "osgEarth/GLUtils"
#include "osgEarth/LineDrawable"
#include "osgEarth/PointDrawable"
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
  geode_ = NULL;
  line_ = NULL;
  point_ = NULL;
  largePoint_ = NULL;
  line_ = NULL;
}

bool TimeTicksChunk::addPoint(const osg::Matrix& matrix, double time, const osg::Vec4& color, bool large)
{
  // first make sure there's room.
  if (isFull())
    return false;

  // if this is the first point added, set up the localization matrix.
  if (offset_ == 0 && count_ == 0)
  {
    world2local_.invert(matrix);
    setMatrix(matrix);
  }

  // record the timestamp and world coords
  times_[offset_ + count_] = time;
  worldCoords_[offset_ + count_] = matrix;

  // resolve the localized point and append it to the various geometries.
  append_(matrix, color, large);

  // advance the counter and update the psets.
  count_++;
  updatePrimitiveSets_();

  return true;
}

int TimeTicksChunk::getBeginMatrix(osg::Matrix& begin) const
{
  if (count_ < 1)
    return 1;
  begin = worldCoords_[offset_];
  return 0;
}

int TimeTicksChunk::getEndMatrix(osg::Matrix& end) const
{
  if (count_ < 1)
    return 1;
  end = worldCoords_[offset_ + count_-1];
  return 0;
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
    geode_ = new osgEarth::LineGroup();
    addChild(geode_.get());

    // cross hatch line ticks
    line_ = new osgEarth::LineDrawable(GL_LINES);
    line_->setDataVariance(osg::Object::DYNAMIC);
    line_->allocate(2 * maxSize_);
    geode_->addChild(line_.get());
  }

  // reset to identity matrices
  world2local_ = osg::Matrixd::identity();
}

void TimeTicksChunk::append_(const osg::Matrix& matrix, const osg::Vec4& color, bool large)
{
  // calculate the local point.
  const osg::Vec3d& world = matrix.getTrans();
  const osg::Vec3f local = world * world2local_;

  // insertion index:
  const unsigned int i = offset_ + count_;

  if (type_ == POINT_TICKS)
  {
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
    const osg::Matrix posMatrix = matrix * world2local_;
    double width = lineLength_ * (large ? largeSizeFactor_ : 1);

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
