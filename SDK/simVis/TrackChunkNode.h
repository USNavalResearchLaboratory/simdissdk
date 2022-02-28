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
#ifndef SIMVIS_TRACK_CHUNK_NODE_H
#define SIMVIS_TRACK_CHUNK_NODE_H

#include "osg/ref_ptr"
#include "osgEarth/LineDrawable"
#include "osgEarth/PointDrawable"
#include "simData/DataTypes.h"
#include "simVis/LocatorNode.h"

namespace simVis
{
/**
 * Node that renders a "chunk" of the track history trail points
 * A full track history trail is segmented into chunks. This allows a
 * number of things:
 *
 *  - efficiently add points to the track (only need to update the last chunk)
 *  - efficiently maintain a limited track size (restricting the total number of points)
 *  - dealing with long tracks (eliminate jitter and improve culling)
 *  - manage memory effectively (always allocate buffer objects of exactly the same size, which OSG likes)
 *
 * Each chunk will hold a limited (and specific) number of points. Once
 * the capacity of a chunk is exceeded, a new chunk gets appended to the
 * graph. Similarly, when point-limiting is in effect, we can adjust the
 * oldest chunk to "drop" points from the end of the track.
 *
 * Each chunk lives under its own MT to prevent single-precision jitter
 * effects in a geocentric map.
 *
 * Note: Choose the Chunk Size carefully. Each chunk pre-allocates all the memory
 * it will possibly need, so if you have a large number of entities with track
 * histories, you can quickly run out of memory.
*/
class SDKVIS_EXPORT TrackPointsChunk
{
public:
  /**
  * Create a new chunk with a maximum size
  * @param maxSize maximum chunk size, in points
  */
  TrackPointsChunk(unsigned int maxSize);

  /**
  * Is this chunk full? i.e. no room for more points?
  * @return true if this chunk is full
  */
  bool isFull() const;

  /**
  * How many points are rendered by this chunk?
  * @return number of active points in this chunk
  */
  unsigned int size() const;

  /**
  * Return time of the first point in this chunk in seconds since ref year, accounting for data limiting
  * @return begin time in seconds
  */
  double getBeginTime() const;

  /**
  * Return time of the last point in this chunk in seconds since ref year, accounting for data limiting
  * @return end time in seconds
  */
  double getEndTime() const;

  /**
  * Remove the oldest point in this chunk
  * @return true if point was removed
  */
  bool removeOldestPoint();

  /**
  * Remove points from the tail
  * @param t earliest time that will remain in this chunk
  * @return number of points removed
  */
  unsigned int removePointsBefore(double t);

  /** Allows the node to be re-used */
  void reset();

protected:
  virtual ~TrackPointsChunk() {}

  /// Is this chunk empty?
  bool isEmpty_() const;

  /// Fix graphics after points are removed
  virtual void fixGraphicsAfterRemoval_() = 0;

  /// Update the offset and count on each primitive set to draw the proper data.
  virtual void updatePrimitiveSets_() = 0;

  // data
  /// timestamp of each point
  std::vector<double> times_;
  /// offset into the point list to the start of points to render
  unsigned int offset_;
  /// number of points to render
  unsigned int count_;
  /// maximum allowable number of points in chunk
  unsigned int maxSize_;
};

/** Implementation of the TrackPointsChunk for drawing track history update points */
class SDKVIS_EXPORT TrackChunkNode : public TrackPointsChunk, public LocatorNode
{
public:
  /**
  * Create a new chunk with a maximum size
  * @param maxSize maximum chunk size, in points
  * @param mode track draw mode that this chunk will display
  */
  TrackChunkNode(unsigned int maxSize, simData::TrackPrefs_Mode mode = simData::TrackPrefs_Mode_POINT);

  /**
  * Add a new point to the chunk
  * @param locator provides point rotation/position/orientation information
  * @param t time that corresponds to the platform update
  * @param color color to render this chunk
  * @param hostBounds left and right boundaries of the host model
  * @return true if point was added
  */
  bool addPoint(const Locator& locator, double t, const osg::Vec4& color, const osg::Vec2& hostBounds);

  /**
  * Get the matrix and time associated with the newest point in this chunk
  * @param out_matrix position matrix for the newest point in the chunk
  * @param out_time update time of the newest point in the chunk
  * @return true if matrix and time are valid
  */
  bool getNewestData(osg::Matrix& out_matrix, double& out_time) const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "TrackChunkNode"; }

protected:
  virtual ~TrackChunkNode();

  /// Update the offset and count on each primitive set to draw the proper data.
  virtual void updatePrimitiveSets_();

  /// Fix the ribbon visual after points deletion to not show links to deleted point
  virtual void fixGraphicsAfterRemoval_();

private:
  /// Allocate the graphical elements for this chunk.
  void allocate_();
  /// Appends a new local track element to each geometry set.
  void append_(const Locator& locator, const osg::Vec4& color, const osg::Vec2& hostBounds);
  /// Appends a new ECI local track element to each geometry set.
  void appendEci_(const Locator& locator, const osg::Vec4& color, const osg::Vec2& hostBounds);
  /// Appends a new point or line to each geometry set.
  void appendPointLine_(unsigned int i, const osg::Vec3f& local, const osg::Vec4& color);
  /// Appends a new bridge element to each geometry set.
  void appendBridge_(unsigned int i, const osg::Vec3f& local, const osg::Vec3d& world, const osg::Vec4& color);
  /// Appends a new ribbon element to each geometry set.
  void appendRibbon_(unsigned int i, const osg::Matrixd& localMatrix, const osg::Vec4& color, const osg::Vec2& hostBounds);

private:
  osg::ref_ptr<Locator> localLocator_;
  osg::ref_ptr<osg::Group> lineGroup_;
  osg::ref_ptr<osgEarth::LineDrawable> centerLine_;
  osg::ref_ptr<osgEarth::PointDrawable> centerPoints_;
  osg::ref_ptr<osgEarth::LineDrawable> ribbon_;
  osg::ref_ptr<osgEarth::LineDrawable> drop_;
  osg::Matrixd world2local_;
  simData::TrackPrefs_Mode mode_;  ///<  track draw mode that this chunk will display
};

} // namespace simVis

#endif // SIMVIS_TRACK_CHUNK_NODE_H
