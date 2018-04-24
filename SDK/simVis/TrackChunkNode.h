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
#ifndef SIMVIS_TRACK_CHUNK_NODE_H
#define SIMVIS_TRACK_CHUNK_NODE_H

#include "osg/ref_ptr"
#include "osg/MatrixTransform"
#include "simVis/LineDrawable.h"
#include "simData/DataTypes.h"

namespace osgEarth { class SpatialReference; }

namespace simVis
{
  /**
  * Node that renders a "chunk" of the track history
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
  class TrackChunkNode : public osg::MatrixTransform
  {
  public:
    /**
    * Create a new chunk with a maximum size
    * @param maxSize maximum chunk size, in points
    * @param srs spatial reference that is being used for track data
    * @param mode track draw mode that this chunk will display
    */
    TrackChunkNode(unsigned int maxSize, const osgEarth::SpatialReference* srs, simData::TrackPrefs_Mode mode = simData::TrackPrefs_Mode_POINT);

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
    * Add a new point to the chunk
    * @param matrix the position matrix that corresponds to the platform update position
    * @param t time that corresponds to the platform update
    * @param color color to render this chunk
    * @param hostBounds left and right boundaries of the host model
    * @return true if point was added
    */
    bool addPoint(const osg::Matrix& matrix, double t, const osg::Vec4& color, const osg::Vec2& hostBounds);

    /**
    * Get the matrix and time associated with the newest point in this chunk
    * @param out_matrix position matrix for the newest point in the chunk
    * @param out_time update time of the newest point in the chunk
    * @return true if matrix and time are valid
    */
    bool getNewestData(osg::Matrix& out_matrix, double& out_time) const;

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

    /**
    * Set the draw mode of the center line
    * @param mode track draw mode that this chunk will display
    */
    void setCenterLineMode(const simData::TrackPrefs_Mode& mode);

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "TrackChunkNode"; }

  protected:
    virtual ~TrackChunkNode();

  private:
    /// Allocate the graphical elements for this chunk.
    void allocate_();

    /// Return time of the first point in this chunk
    double getStartTime_() const;

    /// Return time of the last point in this chunk
    double getEndTime_() const;

    /// Is this chunk empty?
    bool isEmpty_() const;

    /// Remove all the points in this chunk that occur after the timestamp
    unsigned int removePointsAtAndBeyond_(double t);

    /// Appends a new local point to each geometry set.
    void append_(const osg::Matrix& matrix, const osg::Vec4& color, const osg::Vec2& hostBounds);

    /// Update the offset and count on each primitive set to draw the proper data.
    void updatePrimitiveSets_();

    /// Fix the ribbon visual after points deletion to not show links to deleted point
    void fixRibbon_();

  private:
    std::vector<double>           times_;   /// timestamp of each point
    unsigned int                  offset_;  /// offset into the point list to the start of points to render
    unsigned int                  count_;   /// number of points to render
    unsigned int                  maxSize_; /// maximum allowable number of points in chunk
    osg::ref_ptr<osg::Geode>      geode_;

    osg::ref_ptr<osgEarth::LineDrawable> centerLine_;
    osg::ref_ptr<osg::Geometry> centerPoints_;

    osg::ref_ptr<osgEarth::LineDrawable> ribbon_;

    osg::ref_ptr<osgEarth::LineDrawable> drop_;

    osg::ref_ptr<const osgEarth::SpatialReference> srs_;
    osg::Matrixd                  world2local_;
    simData::TrackPrefs_Mode      mode_;  ///  track draw mode that this chunk will display
  };

} // namespace simVis

#endif // SIMVIS_TRACK_CHUNK_NODE_H
