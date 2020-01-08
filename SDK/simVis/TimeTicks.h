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
#ifndef SIMVIS_TIME_TICKS_H
#define SIMVIS_TIME_TICKS_H

#include "simCore/Time/Clock.h"
#include "simData/DataSlice.h"
#include "simData/DataTable.h"
#include "osg/Geode"
#include "osg/MatrixTransform"
#include "simVis/Types.h"

//----------------------------------------------------------------------------
namespace osgEarth
{
  class LineDrawable;
  class SpatialReference;
}
namespace simData
{
  class DataStore;
  class PlatformPrefs;
  class TrackPrefs;
  class DataSliceBase;
}

namespace simVis
{
class Locator;
class TimeTicksChunk;
class PlatformTspiFilterManager;

/**
 * Scene graph node that depicts a time ticks trail for a platform
 */
class SDKVIS_EXPORT TimeTicks : public osg::Group
{
public:
  TimeTicks(const simData::DataStore& ds, const osgEarth::SpatialReference* srs, PlatformTspiFilterManager& manager, simData::ObjectId entityId);

  /**
  * Reset the time ticks visualization, erasing everything that exists
  * so it can start building again from scratch.
  */
  void reset();

  /**
  * Accesses the updates for the associated platform and adds points, using current prefs settings
  * This method is intended to update the time ticks in normal operation, as well as to recreate the time ticks in response to user action
  * This may be slow if track history preferences are set to display many points, since time ticks match track history for data limiting.
  * Time ticks will be created from the first available update time, factoring in data limiting and track length, up to the current scenario time
  * or up to the last update time if the scenario time is past the end of the data history, interpolating at a specified interval.
  */
  void update();

  /**
  * Update the time ticks based on the change in the Clock mode, e.g. to
  * change the time direction.
  * @param clock Clock that might have changed.
  */
  void updateClockMode(const simCore::Clock* clock);

  /**
   * Sets new preferences for this object.
   * @param[in ] platformPrefs Preferences to apply
   * @param[in ] platformProps Properties for the platform
   * @param[in ] force Apply them even if the current settings already match
   */
  void setPrefs(const simData::PlatformPrefs& platformPrefs, const simData::PlatformProperties& platformProps, bool force = false);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "TimeTicks"; }

protected: // methods
  /// osg::Referenced-derived
  virtual ~TimeTicks();

private: // methods

  /** Copy constructor, not implemented or available. */
  TimeTicks(const TimeTicks&);

  /**
  * Return a chunk to which you can add a new point
  * @return chunk that can accept a new point, or NULL if a new one needs to be created
  */
  TimeTicksChunk* getCurrentChunk_();

  /**
  * Return the last chunk in the group,
  * @return last chunk, or NULL if group is empty
  */
  TimeTicksChunk* getLastChunk_();

  /**
  * Return the first chunk in the group
  * @return first chunk, or NULL if group is empty
  */
  TimeTicksChunk* getFirstChunk_();

  /// Update the "flat mode" setting that zeros out the time ticks altitude; initialize shader programs if necessary
  void updateFlatMode_(bool enabled);

  /// Update the draw flag
  void updateVisibility_(const simData::TrackPrefs& prefs);

  /**
  * Remove all points with draw times older than specified time
  * @param oldestDrawTime oldest draw time that will remain in time ticks
  */
  void removePointsOlderThan_(double oldestDrawTime);

  /**
  * Determines the time window that time ticks should display, then
  * determines what needs to be done to display that window, then
  * adds required datapoints.
  * @param currentTime latest time to display
  * @param updateSlice platform update slice for associated platform to get current time
  */
  void updateTrackData_(double currentTime, const simData::PlatformUpdateSlice& updateSlice);

  /**
  * Accesses the updates for the associated platform and adds points for the interval [beginTime, endTime]
  * This may be slow depending on how may points must be backfilled
  * @param endTime last time to be added
  * @param beginTime starting time for points to be added
  */
  void backfillHistory_(double endTime, double beginTime);

  /**
  * Update the time ticks visuals for the current point when drawing line ticks
  * @param updateSlice platform update slice for associated platform
  * @param beginTime earliest platform update time to use when going in reverse
  */
  void updateCurrentPoint_(const simData::PlatformUpdateSlice& updateSlice, double beginTime);

  /**
  * Update the time ticks visuals with a point to correspond to the specified time
  * @param tickTime time to draw a tick
  */
  void addUpdate_(double tickTime);

  /**
  * Convert update time to draw time
  * To support REVERSE playback mode, we play a little trick and simply negate
  * the time so that time always appears to be increasing from the perspective
  * of the rendering code. We do this to avoid adding complex logic in the rendering
  * code for handling bi-directional track drawing.
  * Throughout this class, we use the term "draw time" to represent the adjusted
  * unidirectional time, versus "update time" which is the actual time in the
  * data store.
  * @param updateTime platform update time
  * @return draw time that corresponds to the update time
  */
  double toDrawTime_(double updateTime) const;

  /// utility function to get an OSG ENU matrix that corresponds to platform update's position and orientation
  bool getMatrix_(const simData::PlatformUpdate& u, osg::Matrix& hostMatrix);
  /// utility function to get an OSG ENU matrix that corresponds to the platform position at time, interpolated between the prevPoint and curPoint
  bool getMatrix_(const simData::PlatformUpdate& prevPoint, const simData::PlatformUpdate& curPoint, double time, osg::Matrix& hostMatrix);

private: // data
  /// data store for initializing data slice
  const simData::DataStore& ds_;
  /// flag indicates if current system supports using shaders
  bool supportsShaders_;

  simData::PlatformPrefs      lastPlatformPrefs_;
  simData::PlatformProperties lastPlatformProps_;
  unsigned int        chunkSize_;
  osg::Vec4f          color_;
  unsigned int        totalPoints_;

  // "draw time" is the same as the clock's update time, but adjusted
  // for time direction. i.e. it will be negated in the case of simCore::REVERSE.
  bool                   hasLastDrawTime_;
  double                 lastDrawTime_;
  double                 lastCurrentTime_;
  double                 lastLargeTickTime_;
  double                 largeTickInterval_;
  double                 lastLabelTime_;
  double                 labelInterval_;

  // Playback direction (follows a datastore-bound Clock).
  simCore::TimeDirection timeDirection_;

  osg::ref_ptr<osg::Uniform>    flatModeUniform_;
  osg::ref_ptr<osg::Group>      chunkGroup_;
  osg::ref_ptr<osg::Group>      labelGroup_;
  std::map<double, osg::MatrixTransform*> labels_;

  const simData::DataSliceBase* updateSliceBase_;
  PlatformTspiFilterManager& platformTspiFilterManager_;
  /// entity id for the platform
  simData::ObjectId entityId_;

  osg::ref_ptr<TimeTicksChunk>  currentPointChunk_;
  osg::ref_ptr<simVis::Locator> locator_;
};

} // namespace simVis

#endif // SIMVIS_TIME_TICKS_H
