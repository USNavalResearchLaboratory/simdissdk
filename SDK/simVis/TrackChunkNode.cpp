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
#include "osgEarth/GeoData"
#include "simVis/Types.h"
#include "simVis/TrackChunkNode.h"

namespace simVis
{

//----------------------------------------------------------------------------
/** Creates a new chunk with a maximum size. */
TrackChunkNode::TrackChunkNode(unsigned int maxSize, const osgEarth::SpatialReference* srs, simData::TrackPrefs_Mode mode)
  : maxSize_(maxSize),
    srs_(srs),
    mode_(mode)
{
  allocate_();
}

TrackChunkNode::~TrackChunkNode()
{
  geode_ = NULL;
  centerLine_ = NULL;
  centerPoints_ = NULL;
  ribbon_ = NULL;
  drop_ = NULL;
  srs_ = NULL;
}

/// is this chunk full? i.e. no room for more points?
bool TrackChunkNode::isFull() const
{
  return (offset_ + count_) >= maxSize_;
}

/// how many points are rendered by this chunk?
unsigned int TrackChunkNode::size() const
{
  return count_;
}

/// add a new point to the chunk.
bool TrackChunkNode::addPoint(const osg::Matrix& matrix, double t, const osg::Vec4& color, const osg::Vec2& hostBounds)
{
  // first make sure there's room.
  if (isFull())
    return false;

  // if this is the first point added, set up the localization matrix.
  if (offset_ == 0 && count_ == 0)
  {
    world2local_.invert(matrix);
    this->setMatrix(matrix);
  }

  // record the timestamp
  times_[offset_ + count_] = t;

  // resolve the localized point and append it to the various geometries.
  append_(matrix, color, hostBounds);

  // advance the counter and update the psets.
  count_++;
  updatePrimitiveSets_();

  return true;
}

bool TrackChunkNode::getNewestData(osg::Matrix& out_matrix, double& out_time) const
{
  if (count_ == 0)
    return false;
  const osg::Vec3& p = centerLine_->getVertex(offset_ + count_ - 1);
  out_matrix.makeTranslate(p * getMatrix());
  out_time = times_[offset_ + count_ - 1];
  return true;
}

/// remove the oldest point in this chunk.
bool TrackChunkNode::removeOldestPoint()
{
  if (count_ == 0)
    return false;

  offset_++;
  count_--;
  updatePrimitiveSets_();
  // don't bother updating the bound.

  // this does dirtyBound if ribbon mode
  fixRibbon_();
  return true;
}

/// remove points from the tail; return the number of points removed.
unsigned int TrackChunkNode::removePointsBefore(double t)
{
  const unsigned int origOffset = offset_;
  while (count_ > 0 && times_[offset_] < t)
  {
    offset_++;
    count_--;
  }

  if (origOffset != offset_)
  {
    updatePrimitiveSets_();
    // would normally dirtyBound(), but don't bother.

    // this does dirtyBound if ribbon mode
    fixRibbon_();
  }

  return offset_ - origOffset;
}

void TrackChunkNode::reset()
{
  times_[0] = 0.0;
  offset_ = 0;
  count_ = 0;
}

/// set the draw mode of the center line
void TrackChunkNode::setCenterLineMode(const simData::TrackPrefs_Mode& mode)
{
  bool usePoints = (mode == simData::TrackPrefs_Mode_POINT);
  centerLine_->setNodeMask(usePoints? 0 : ~0);
  centerPoints_->setNodeMask(usePoints? ~0 : 0);
}

/// allocate the graphical elements for this chunk.
void TrackChunkNode::allocate_()
{
  // clear existing:
  this->removeChildren(0, this->getNumChildren());

  // timestamp vector.
  times_.resize(maxSize_);
  times_[0] = 0.0;

  // pointers into the points list.
  offset_ = 0;
  count_  = 0;

  // geode to hold all geometry:
  geode_ = new osgEarth::LineGroup();
  this->addChild(geode_);

  // center line (line mode)
  centerLine_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
  centerLine_->setDataVariance(osg::Object::DYNAMIC);
  centerLine_->allocate(maxSize_);
  geode_->addChild(centerLine_.get());

  // center line (point mode)
  centerPoints_ = new osg::Geometry();
  centerPoints_->setUseVertexBufferObjects(true);
  centerPoints_->setUseDisplayList(false);
  osg::Vec3Array* verts = new osg::Vec3Array();
  verts->assign(maxSize_, osg::Vec3());
  centerPoints_->setVertexArray(verts);
  osg::Vec4Array* colors = new osg::Vec4Array();
  colors->setBinding(osg::Array::BIND_PER_VERTEX);
  colors->assign(maxSize_, simVis::Color::White);
  centerPoints_->setColorArray(colors);
  centerPoints_->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, offset_, count_));
  this->addChild(centerPoints_.get());

  if (mode_ == simData::TrackPrefs_Mode_BRIDGE)
  {
    drop_ = new osgEarth::LineDrawable(GL_LINES);
    drop_->setDataVariance(osg::Object::DYNAMIC);
    drop_->allocate(2*maxSize_);
    geode_->addChild(drop_.get());
  }
  else if (mode_ == simData::TrackPrefs_Mode_RIBBON)
  {
    ribbon_ = new osgEarth::LineDrawable(GL_LINES);
    ribbon_->setDataVariance(osg::Object::DYNAMIC);
    ribbon_->allocate(6*maxSize_);
    geode_->addChild(ribbon_.get());
  }

  // reset to identity matrices
  world2local_ = osg::Matrixd::identity();
}

/// time of the first point in this chunk
double TrackChunkNode::getStartTime_() const
{
  return count_ >= 1 ? times_[offset_] : -1.0;
}

/// time of the last point in this chunk
double TrackChunkNode::getEndTime_() const
{
  return count_ >= 1 ? times_[offset_+count_-1] : -1.0;
}

/// is this chunk empty?
bool TrackChunkNode::isEmpty_() const
{
  return count_ == 0;
}

/// remove all the points in this chunk that occur after the timestamp
unsigned int TrackChunkNode::removePointsAtAndBeyond_(double t)
{
  const unsigned int origCount = count_;
  while (count_ > 0 && times_[offset_+count_-1] >= t)
  {
    count_--;
  }

  if (origCount != count_)
  {
    updatePrimitiveSets_();
  }

  return origCount - count_;
}

/// appends a new local point to each geometry set.
void TrackChunkNode::append_(const osg::Matrix& matrix, const osg::Vec4& color, const osg::Vec2& hostBounds)
{
  // calculate the local point.
  const osg::Vec3d& world = matrix.getTrans();
  const osg::Vec3f local = world * world2local_;

  // insertion index:
  const unsigned int i = offset_ + count_;

  // append to the centerline track (1 vert)
  centerLine_->setVertex(i, local);
  centerLine_->setColor(i, color);

  // and update the center points track as well:
  osg::Vec3Array& centerPointsVerts = static_cast<osg::Vec3Array&>(*centerPoints_->getVertexArray());
  centerPointsVerts[i] = local;
  centerPointsVerts.dirty();
  osg::Vec4Array& centerPointsColors = static_cast<osg::Vec4Array&>(*centerPoints_->getColorArray());
  centerPointsColors[i] = color;
  centerPointsColors.dirty();
  centerPoints_->dirtyBound();

  if (mode_ == simData::TrackPrefs_Mode_BRIDGE)
  {
    // draw a new drop line (2 verts)
    osg::Vec3d up;
    osgEarth::GeoPoint geo;
    geo.fromWorld(srs_->getGeographicSRS(), world);
    geo.createWorldUpVector(up);
    up.normalize();

    drop_->setVertex(2*i, local);
    drop_->setVertex(2*i+1, (world - up*geo.alt()) * world2local_);
    drop_->setColor(2*i, color);
    drop_->setColor(2*i+1, color);
  }
  else if (mode_ == simData::TrackPrefs_Mode_RIBBON)
  {
    // add a new ribbon segment. A ribbon segment has 6 verts. The x value and y value for
    // hostBounds represents the xMin and xMax values of a bounding box respectively.
    const osg::Matrix posMatrix = matrix * world2local_;
    const osg::Vec3f left  = osg::Vec3d(hostBounds.x(), 0.0, 0.0) * posMatrix;
    const osg::Vec3f right = osg::Vec3d(hostBounds.y(), 0.0, 0.0) * posMatrix;

    const osg::Vec3& leftPrev  = count_ > 0 ? ribbon_->getVertex(6*i-2) : left;
    const osg::Vec3& rightPrev = count_ > 0 ? ribbon_->getVertex(6*i-1) : right;
    
    // add connector lines to previous sample
    // TODO: account for previous chunk
    ribbon_->setVertex(6*i, leftPrev);
    ribbon_->setVertex(6*i+1, left);
    ribbon_->setVertex(6*i+2, rightPrev);
    ribbon_->setVertex(6*i+3, right);
    // ..and the new sample:
    ribbon_->setVertex(6*i+4, left);
    ribbon_->setVertex(6*i+5, right);

    for (unsigned int c = 0; c < 6; ++c)
      ribbon_->setColor(6*i+c, color);
  }
}

/// update the offset and count on each primitive set to draw the proper data.
void TrackChunkNode::updatePrimitiveSets_()
{
  centerLine_->setFirst(offset_);
  centerLine_->setCount(count_);

  osg::DrawArrays& centerPointsPrimSet = static_cast<osg::DrawArrays&>(*centerPoints_->getPrimitiveSet(0));
  centerPointsPrimSet.setFirst(offset_);
  centerPointsPrimSet.setCount(count_);

  if (mode_ == simData::TrackPrefs_Mode_BRIDGE)
  {
    drop_->setFirst(2*offset_);
    drop_->setCount(2*count_);
  }
  else if (mode_ == simData::TrackPrefs_Mode_RIBBON)
  {
    // TODO: fix for the first segment, which only has 2 instead of 6.
    ribbon_->setFirst(6*offset_);
    ribbon_->setCount(6*count_);
  }
}

// only to be called when points are deleted, so that ribbon visual can be fixed to not show links to deleted point
void TrackChunkNode::fixRibbon_()
{
  if (mode_ == simData::TrackPrefs_Mode_RIBBON && !isEmpty_() && offset_ > 0)
  {
    // count>0 should mean offset_<size. if assert fails, check that:
    // point add and remove in this class correctly adjust count and offset, and that
    // TrackHistoryNode, when removing points, also removes chunks when their size = 0
    assert(offset_ < maxSize_);
    // reset verts that linked to a previous point that has been removed
    ribbon_->setVertex(6 * offset_, ribbon_->getVertex(6*offset_+1));
    ribbon_->setVertex(6 * offset_ + 2, ribbon_->getVertex(6*offset_+3));
  }
}

}
