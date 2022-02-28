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
#ifndef SIMVIS_TIME_TICKS_CHUNK_H
#define SIMVIS_TIME_TICKS_CHUNK_H

#include <map>
#include <vector>
#include "simVis/TrackChunkNode.h"

namespace simVis
{

/** Implementation of the TrackPointsChunk to draw track history time ticks */
class SDKVIS_EXPORT TimeTicksChunk : public TrackPointsChunk, public LocatorNode
{
public:
  /// Draw mode for the time ticks
  enum Type
  {
    POINT_TICKS,
    LINE_TICKS
  };

  /**
  * Create a new chunk with a maximum size
  * @param maxSize maximum chunk size, in points
  * @param type draw style for rendering ticks
  * @param lineLength width in meters of line tick to draw
  * @param pointSize pixel size of points tick to draw
  * @param largeFactor large tick factor for line and point, multiple of lineLength for line, multiple of pointSize for point
  */
  TimeTicksChunk(unsigned int maxSize, Type type, double lineLength, double pointSize, unsigned int largeFactor);

  /**
  * Add a new point to the chunk
  * @param tickLocator the locator that contains point rotation, position, orientation (may be interpolated)
  * @param t time that corresponds to the platform update, seconds since scenario ref year
  * @param color color to render this chunk
  * @param large indicates if this is a large tick
  * @return true if point was added
  */
  bool addPoint(const Locator& tickLocator, double t, const osg::Vec4& color, bool large);

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
  void append_(const osg::Matrixd& matrix, const osg::Vec4f& color, bool large);

  virtual void fixGraphicsAfterRemoval_();
  /// Update the offset and count on each primitive set to draw the proper data.
  virtual void updatePrimitiveSets_();

private:
  /// draw type
  Type type_;
  /// line tick length
  double lineLength_;
  /// point tick size
  double pointSize_;
  /// large tick size factor
  double largeSizeFactor_;
  ///container for drawables
  osg::ref_ptr<osg::Group> lineGroup_;
  /// point graphic
  osg::ref_ptr<osgEarth::PointDrawable> point_;
  /// point graphic for large points
  osg::ref_ptr<osgEarth::PointDrawable> largePoint_;
  /// line graphic
  osg::ref_ptr<osgEarth::LineDrawable> line_;
  /// matrix to convert from world to local coords
  osg::Matrixd world2local_;
  /// cache the world coordinates for quick access
  std::vector<osg::Matrix> worldCoords_;
};

} // namespace simVis

#endif // SIMVIS_TIME_TICKS_CHUNK_H
