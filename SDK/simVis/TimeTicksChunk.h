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
#ifndef SIMVIS_TIME_TICKS_CHUNK_H
#define SIMVIS_TIME_TICKS_CHUNK_H

#include <map>
#include <vector>
#include "osg/ref_ptr"
#include "osg/MatrixTransform"
#include "simData/DataTypes.h"
#include "simVis/TrackChunkNode.h"

namespace simVis
{
/** Implementation of the TrackPointsChunk to draw track history time ticks */
class SDKVIS_EXPORT TimeTicksChunk : public TrackPointsChunk
{
public:
  /// Draw mode for the time ticks
  enum Type
  {
    POINT_TICKS,
    LINE_TICKS,
    LINE
  };

  /**
  * Create a new chunk with a maximum size
  * @param maxSize maximum chunk size, in points
  * @param drawPoints if true draw point ticks, otherwise draw line ticks
  */
  TimeTicksChunk(unsigned int maxSize, Type type, double lineTickWidth, double largeLineTickWidth);

  /**
  * Add a new point to the chunk
  * @param matrix the position matrix that corresponds to the platform position (may be interpolated)
  * @param t time that corresponds to the platform update, seconds since scenario ref year
  * @param color color to render this chunk
  * @param large indicates if this is a large tick
  * @return true if point was added
  */
  bool addPoint(const osg::Matrix& matrix, double t, const osg::Vec4& color, bool large);

  /**
  * Get the earliest position matrix added to the chunk, accounting for data limiting
  * @param position matrix that corresponds to a platform position (may be interpolated)
  * @return 0 on success, non-zero if chunk is empty
  */
  int getBeginMatrix(osg::Matrix& first) const;

  /**
  * Get the latest position matrix added to the chunk, accounting for data limiting
  * @param position matrix that corresponds to a platform position (may be interpolated)
  * @return 0 on success, non-zero if chunk is empty
  */
  int getEndMatrix(osg::Matrix& last) const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "TimeTicksChunk"; }

protected:
  virtual ~TimeTicksChunk();

private:
  /// Allocate the graphical elements for this chunk.
  void allocate_();

  /// Appends a new local point to each geometry set.
  void append_(const osg::Matrix& matrix, const osg::Vec4& color, bool large);

  virtual void fixGraphicsAfterRemoval_();
  /// Update the offset and count on each primitive set to draw the proper data.
  virtual void updatePrimitiveSets_();

private:
  /// draw type
  Type type_;
  /// line tick width
  double lineTickWidth_;
  /// large line tick width
  double largeLineTickWidth_;
  ///container for drawables
  osg::ref_ptr<osg::Geode> geode_;
  /// point graphic
  osg::ref_ptr<osg::Geometry> point_;
  /// point graphic for large points
  osg::ref_ptr<osg::Geometry> largePoint_;
  /// line graphic
  osg::ref_ptr<osgEarth::LineDrawable> line_;
  /// matrix to convert from world to local coords
  osg::Matrixd world2local_;
  /// cache the world coordinates for quick access
  std::vector<osg::Matrix> worldCoords_;
};

} // namespace simVis

#endif // SIMVIS_TIME_TICKS_CHUNK_H
