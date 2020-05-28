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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_TRACK_HISTORY_H
#define SIMVIS_TRACK_HISTORY_H

#include "osg/Group"
#include "simCore/Time/Clock.h"
#include "simData/DataSlice.h"
#include "simData/DataTable.h"
#include "simVis/TrackChunkNode.h"
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
class TrackChunkNode;
class PlatformTspiFilterManager;

/**
  * Scene graph node that depicts a track history trail for a platform
  */
class SDKVIS_EXPORT TrackHistoryNode : public osg::Group
{
public:
  TrackHistoryNode(const simData::DataStore& ds, Locator* parentLocator, PlatformTspiFilterManager& manager, simData::ObjectId entityId);

  /**
    * Before using this class a call to installShaderProgram is required.  This
    * method installs the shader program and default uniform variables for
    * controlling the shader.
    */
  static void installShaderProgram(osg::StateSet* intoStateSet);

  /**
  * Reset the track history visualization, erasing everything that exists
  * so it can start building again from scratch.
  */
  void reset();

  /**
  * Accesses the updates for the associated platform and adds points to the TrackHistory, using current prefs settings
  * This method is intended to update the track history in normal operation, as well as to recreate the track history in response to user action
  * This may be slow if track history preferences are set to display many points.
  * History points will be created from the first available update time, factoring in data limiting and track length, up to the current scenario time
  * or up to the last update time if the scenario time is past the end of the data history
  */
  void update();

  /**
  * Update the track history based on the change in the Clock mode, e.g. to
  * change the time direction.
  * @param clock Clock that might have changed.
  */
  void updateClockMode(const simCore::Clock* clock);

  /**
  * Set the bounds of the host platform model. (internal)
  * @param bounds the left and right side boundaries of the host model
  */
  void setHostBounds(const osg::Vec2& bounds);

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
  virtual const char* className() const { return "TrackHistoryNode"; }

protected: // methods
  /// osg::Referenced-derived
  virtual ~TrackHistoryNode();

private: // methods

  class ColorChangeObserver;
  class ColorTableObserver;

  /** Copy constructor, not implemented or available. */
  TrackHistoryNode(const TrackHistoryNode&);

  /**
  * If the color history change is within the time span of the currently displayed track history, redraw all history points
  * @param table  color track history data table
  * @param row  row that was added to the color track history data table
  */
  void checkColorHistoryChange_(const simData::DataTable& table, const simData::TableRow& row);

  /**
  * Return a chunk to which you can add a new point
  * @return chunk that can accept a new point, or NULL if a new one needs to be created
  */
  TrackChunkNode* getCurrentChunk_();

  /**
  * Get the track history color at the specified time, querying the SIMDIS internal data table. Returns a default color if no valid entry found at time
  * @param time in seconds for history color
  * @return color at the specified time
  */
  osg::Vec4f historyColorAtTime_(double time);

  /**
  * Try to initialize the track history color table id
  */
  void initializeTableId_();

  /// Update the "flat mode" setting that zeros out the track history altitude; initialize shader programs if necessary
  void updateFlatMode_(bool enabled);

  /// Update the "altitude mode" settings that draws a line from the platform to the ground
  void updateAltMode_(bool enabled, const simData::PlatformUpdateSlice& updateSlice);
  void updateAltModePositionAndAppearance_(const simCore::Coordinate& ecefCoord, double time);


  /// Update the draw flag
  void updateVisibility_(const simData::TrackPrefs& prefs);

  /**
  * Remove all points with draw times older than specified time
  * @param oldestDrawTime oldest draw time that will remain in track history
  */
  void removePointsOlderThan_(double oldestDrawTime);

  /// set override color; initialize shader programs if necessary
  void setOverrideColor_(const simVis::Color& color);

  /**
  * Determines the time window that track history should display, then
  * determines what needs to be done to display that window, then
  * adds required datapoints to history.
  * @param currentTime latest history time to display
  * @param firstTime earliest history time to display
  */
  void updateTrackData_(double currentTime, double firstTime);

  /**
  * Accesses the updates for the associated platform and adds points to the TrackHistory for the interval [beginTime, endTime]
  * This may be slow depending on how may points must be backfilled
  * @param endTime last time to be added
  * @param beginTime starting time for points to be added
  */
  void backfillTrackHistory_(double endTime, double beginTime);

  /**
  * Update the track history visuals when the current point is interpolated
  * @param updateSlice platform update slice for associated platform
  */
  void updateCurrentPoint_(const simData::PlatformUpdateSlice& updateSlice);

  /**
  * Update the track history visuals with a point to correspond to the specified platform update
  * @param update platform update from which to obtain track position information
  * @param prevUpdate the previous platform update
  */
  void addUpdate_(const simData::PlatformUpdate& update, const simData::PlatformUpdate* prevUpdate);

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

  /// utility function to set a coordinate from a platform update's position and orientation
  bool getCoord_(const simData::PlatformUpdate& u, simCore::Coordinate& ecefCoord);
  /// utility function to set a locator from a platform update's position and orientation
  bool fillLocator_(const simData::PlatformUpdate& u, Locator* locator);

private: // data
  /// data store for initializing data slice and accessing table manager
  const simData::DataStore& ds_;
  /// flag indicates if current system supports using shaders
  bool supportsShaders_;
  osg::Vec2                   hostBounds_;
  simData::PlatformPrefs      lastPlatformPrefs_;
  simData::PlatformProperties lastPlatformProps_;
  unsigned int        chunkSize_;
  const osg::Vec4f    defaultColor_;
  osg::Vec4f          activeColor_;
  unsigned int        totalPoints_;

  // "draw time" is the same as the clock's update time, but adjusted
  // for time direction. i.e. it will be negated in the case of simCore::REVERSE.
  bool                   hasLastDrawTime_;
  double                 lastDrawTime_;
  double                 lastCurrentTime_;

  // Playback direction (follows a datastore-bound Clock).
  simCore::TimeDirection timeDirection_;
  double                 timeDirectionSign_;  // -1 = REVERSE, 1 = FORWARD

  osg::ref_ptr<osg::Uniform>    overrideColorUniform_;
  /// Used by the fragment shader to determine whether or not to apply the overrideColorUniform
  osg::ref_ptr<osg::Uniform>    enableOverrideColorUniform_;
  simVis::Color                 lastOverrideColor_;
  osg::ref_ptr<osg::Uniform>    flatModeUniform_;
  osg::ref_ptr<osg::Group>      chunkGroup_;
  osg::ref_ptr<osgEarth::LineDrawable>   dropVertsDrawable_;
  osg::ref_ptr<LocatorNode>     altModeXform_;
  const simData::DataSliceBase* updateSliceBase_;
  PlatformTspiFilterManager& platformTspiFilterManager_;
  /// entity id for the platform
  simData::ObjectId entityId_;
  /// cache the table id for the data table with track color history
  simData::TableId tableId_;
  osg::ref_ptr<TrackChunkNode>  currentPointChunk_;
  /// locator to parent all chunk locators
  osg::ref_ptr<simVis::Locator> parentLocator_;
  /// locator to calculate track point positions
  osg::ref_ptr<simVis::Locator> localLocator_;
  /// observer for changes to the internal track color data table
  simData::DataTable::TableObserverPtr colorChangeObserver_;
  /// observer for when the internal track color data table is added/removed
  simData::DataTableManager::ManagerObserverPtr colorTableObserver_;
};

} // namespace simVis

#endif // SIMVIS_TRACK_HISTORY_H
