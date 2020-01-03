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
#include "osgEarth/LineDrawable"
#include "simVis/PointSize.h"
#include "simVis/Types.h"
#include "simVis/TimeTicksChunk.h"

namespace simVis
{
// TODO: SIM-4428 these will likely be prefs, possibly derived from tick linewidth
static const int POINT_SIZE = 4;
static const int LARGE_POINT_SIZE = 8;

TimeTicksChunk::TimeTicksChunk(unsigned int maxSize, Type type, double lineTickWidth, double largeLineTickWidth)
  : TrackPointsChunk(maxSize),
    type_(type),
    lineTickWidth_(lineTickWidth),
    largeLineTickWidth_(largeLineTickWidth)
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
    largePoint_ = new osg::Geometry();
    largePoint_->setUseVertexBufferObjects(true);
    largePoint_->setUseDisplayList(false);
    largePoint_->setDataVariance(osg::Object::DYNAMIC);
    osg::Vec3Array* largeVerts = new osg::Vec3Array();
    largeVerts->assign(maxSize_, osg::Vec3());
    largePoint_->setVertexArray(largeVerts);
    osg::Vec4Array* largeColors = new osg::Vec4Array();
    largeColors->setBinding(osg::Array::BIND_PER_VERTEX);
    largeColors->assign(maxSize_, osg::Vec4(0, 0, 0, 0));
    largePoint_->setColorArray(largeColors);
    largePoint_->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, offset_, count_));
    addChild(largePoint_.get());
    PointSize::setValues(largePoint_->getOrCreateStateSet(), LARGE_POINT_SIZE, osg::StateAttribute::ON);

    // points
    point_ = new osg::Geometry();
    point_->setUseVertexBufferObjects(true);
    point_->setUseDisplayList(false);
    point_->setDataVariance(osg::Object::DYNAMIC);
    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->assign(maxSize_, osg::Vec3());
    point_->setVertexArray(verts);
    osg::Vec4Array* colors = new osg::Vec4Array();
    colors->setBinding(osg::Array::BIND_PER_VERTEX);
    colors->assign(maxSize_, simVis::Color::White);
    point_->setColorArray(colors);
    point_->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, offset_, count_));
    addChild(point_.get());
    PointSize::setValues(point_->getOrCreateStateSet(), POINT_SIZE, osg::StateAttribute::ON);
  }
  else if (type_ == LINE_TICKS)
  {
    // geode to hold all line geometry:
    geode_ = new osgEarth::LineGroup();
    addChild(geode_.get());

    // center line (line mode)
    line_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
    line_->setDataVariance(osg::Object::DYNAMIC);
    line_->allocate(4 * maxSize_);
    geode_->addChild(line_.get());
  }
  else if (type_ == LINE)
  {
    // geode to hold all line geometry:
    geode_ = new osgEarth::LineGroup();
    addChild(geode_.get());

    // center line (line mode)
    line_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
    line_->setDataVariance(osg::Object::DYNAMIC);
    line_->allocate(maxSize_);
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
      osg::Vec3Array* largeVerts = static_cast<osg::Vec3Array*>(largePoint_->getVertexArray());
      (*largeVerts)[i] = local;
      largeVerts->dirty();
      osg::Vec4Array* largeColors = static_cast<osg::Vec4Array*>(largePoint_->getColorArray());
      (*largeColors)[i] = color;
      largeColors->dirty();
      largePoint_->dirtyBound();
    }
    osg::Vec3Array* pointVerts = static_cast<osg::Vec3Array*>(point_->getVertexArray());
    (*pointVerts)[i] = local;
    pointVerts->dirty();
    osg::Vec4Array* colors = static_cast<osg::Vec4Array*>(point_->getColorArray());
    (*colors)[i] = color;
    colors->dirty();
    point_->dirtyBound();
  }
  else if (type_ == LINE_TICKS)
  {
    // add a new tick. The x value and y value for
    // hostBounds represents the xMin and xMax values of a bounding box respectively.
    const osg::Matrix posMatrix = matrix * world2local_;

    double width = (large ? largeLineTickWidth_ : lineTickWidth_);

    const osg::Vec3f left = osg::Vec3d(-width, 0.0, 0.0) * posMatrix;
    const osg::Vec3f right = osg::Vec3d(width, 0.0, 0.0) * posMatrix;

    line_->setVertex(4 * i, local);
    line_->setVertex(4 * i + 1, left);
    line_->setVertex(4 * i + 2, right);
    line_->setVertex(4 * i + 3, local);

    for (unsigned int c = 0; c < 4; ++c)
      line_->setColor(4 * i + c, color);
  }
  else if (type_ == LINE)
  {
    line_->setVertex(i, local);
    line_->setColor(i, color);
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
    osg::DrawArrays* pointSet = static_cast<osg::DrawArrays*>(point_->getPrimitiveSet(0));
    pointSet->setFirst(offset_);
    pointSet->setCount(count_);
    osg::DrawArrays* largeSet = static_cast<osg::DrawArrays*>(largePoint_->getPrimitiveSet(0));
    largeSet->setFirst(offset_);
    largeSet->setCount(count_);
  }
  else if (type_ == LINE_TICKS)
  {
    line_->setFirst(4 * offset_);
    line_->setCount(4 * count_);
  }
  else if (type_ == LINE)
  {
    line_->setFirst(offset_);
    line_->setCount(count_);
  }
}

}
