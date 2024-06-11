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
#ifndef SIMDIS_UTIL_EXAMPLE_RESOURCES_H
#define SIMDIS_UTIL_EXAMPLE_RESOURCES_H

#include "osg/observer_ptr"
#include "osg/NodeCallback"
#include "osgEarth/Controls"
#include "simCore/Common/Export.h"
#include "simCore/Time/Clock.h"

/**
* Resources for use with the SIMDIS SDK Examples
*/
#define EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS  "http://readymap.org/readymap/tiles/1.0.0/7/"
#define EXAMPLE_ELEVATION_LAYER_TMS       "http://readymap.org/readymap/tiles/1.0.0/116/"

// Environment variable to query for SIMDIS SDK data path
#define EXAMPLE_FILE_PATH_VAR             "SIMDIS_SDK_FILE_PATH"

// Path to use if the SIMDIS_SDK_FILE_PATH environment variable is not set
#ifdef WIN32
#define EXAMPLE_DEFAULT_DATA_PATH       "C:/Program Files/SIMDIS_SDK-Data"
#else // !WIN32
#define EXAMPLE_DEFAULT_DATA_PATH       "/usr/local/share/SIMDIS_SDK-Data"
#endif // WIN32

#define EXAMPLE_GLOBAL_IMAGERY_LAYER_DB   "lowResEarth.mbtiles"
#define EXAMPLE_HIRES_INSET_LAYER_DB      "highResKauaiNiihau.mbtiles"
#define EXAMPLE_ELEVATION_LAYER_DB        "kauaiElevation.mbtiles"

#define EXAMPLE_HAWAII_LOCAL_BATHYMETRY   "SRTM30Plus_Hawaii.tif" // includes bathymetry

#define EXAMPLE_HAWAII_TMS_EARTH          "simdis_tms.zip/doc.earth"

#define EXAMPLE_OCEAN_SURFACE_IMAGE       "watersurface1.png"

#define EXAMPLE_SHIP_ICON                 "USSV/ussv.ive"
#define EXAMPLE_AIRPLANE_ICON             "aqm-37c/aqm-37c.ive"
#define EXAMPLE_MISSILE_ICON              "mm-38_exocet/mm-38_exocet.ive"
#define EXAMPLE_IMAGE_ICON                "A6V.png" // PNG file
#define EXAMPLE_TANK_ICON                 "t72-tank/t72-tank_des.flt"

#define EXAMPLE_RCS_FILE                  "data/rcs/ship_rcs_1.rcs"

#define EXAMPLE_GOG_MISSILE_LL            "missile.ll"
#define EXAMPLE_GOG_MISSILE_LLA           "missile.lla"

#define EXAMPLE_ANTENNA_PATTERN           "SINXX"

#define EXAMPLE_ROCKET_BURN_TEXTURE       "p.rgb"

namespace osgEarth { class Map; }
namespace simCore { class ClockImpl; }
namespace simData { class DataStore; }
namespace simVis
{
  class SceneManager;
  class Viewer;
}

namespace simExamples
{
  /** Creates a map for use with the examples. */
  extern SDKUTIL_EXPORT osgEarth::Map* createDefaultExampleMap();

  /** Creates a map with a one image and one elevation layer. */
  extern SDKUTIL_EXPORT osgEarth::Map* createRemoteWorldMap();

  /** Creates a map in which the ocean elevation is 0. */
  extern SDKUTIL_EXPORT osgEarth::Map* createWorldMapWithFlatOcean();

  /** Creates a sample map that demonstrates .mbtiles format support (Hi-res Hawaii inset) */
  extern SDKUTIL_EXPORT osgEarth::Map* createHawaiiMap();

  /** Creates a sample map of Hawaii and surrounding area from local data (no Internet) */
  extern SDKUTIL_EXPORT osgEarth::Map* createHawaiiMapLocalWithBathymetry();

  /** Creates a sample map from the SIMDIS TMS example repo (hi-res Hawaii and San Diego) */
  extern SDKUTIL_EXPORT osgEarth::Map* createHawaiiTMSMap();

  /** List of model search paths */
  extern SDKUTIL_EXPORT void configureSearchPaths();

  /** Retrieves the file path to the sample data based on environment variable settings */
  extern SDKUTIL_EXPORT std::string getSampleDataPath();

  /** Retrieves default Triton path, if found */
  extern SDKUTIL_EXPORT std::string getTritonResourcesPath();

  /** Retrieves default SilverLining path, if found */
  extern SDKUTIL_EXPORT std::string getSilverLiningResourcesPath();

  /** Returns true i argc contains the pattern string */
  extern SDKUTIL_EXPORT bool hasArg(const std::string& pattern, int argc, char** argv);

  /** Returns true i argc contains the pattern string */
  extern SDKUTIL_EXPORT bool readArg(const std::string& pattern, int argc, char** argv, std::string& out);

  /**
   * add a default sky node to all the views in the viewer
   * Use osgEarth SimpleSky driver, creating a sky node with atmospheric lighting turned off, ambient value of 0.5 and exposure value of 2
   */
  extern SDKUTIL_EXPORT void addDefaultSkyNode(simVis::Viewer* viewer);

  /**
   * add a default sky node to the scene manager.
   * Use osgEarth SimpleSky driver, creating a sky node with atmospheric lighting turned off, ambient value of 0.5 and exposure value of 2
   */
  extern SDKUTIL_EXPORT void addDefaultSkyNode(simVis::SceneManager* sceneMan);

  /**
   * Callback that can be attached to a simCore::Clock to update the scene's sky node time.
   * To use, instantiate your simCore::Clock then call something like:
   *
   * <code>
   * clock_->registerTimeCallback(simCore::Clock::TimeObserverPtr(
   *     new simExamples::SkyNodeTimeUpdater(sceneManager_)
   * ));
   * </code>
   */
  class SDKUTIL_EXPORT SkyNodeTimeUpdater : public simCore::Clock::TimeObserver
  {
  public:
    /** Constructor */
    SkyNodeTimeUpdater(simVis::SceneManager* mgr=nullptr);
    /** Changes the scene manager */
    void setSceneManager(simVis::SceneManager* mgr);
    /** Update the scene manager's sky node with current clock time */
    virtual void onSetTime(const simCore::TimeStamp &t, bool isJump);
    /** No-op for sky node time updater */
    virtual void onTimeLoop();
    /** No-op for sky node time updater */
    virtual void adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime);
    /** Changes an "hours" offset intended to help adjust daylight */
    void setHoursOffset(double hours);
    /** Retrieves the current hours offset */
    double hoursOffset() const;

  private:
    osg::observer_ptr<simVis::SceneManager> sceneManager_;
    simCore::TimeStamp lastTime_;
    double hoursOffset_;
  };


  /**
   * Callback that will idle() a simCore::ClockImpl, then update the associated DataStore.
   * This is intended to be associated with a single node update callback, such as:
   * <code>
   *   scenarioManager_->addUpdateCallback(new simExamples::IdleClock(clock, dataStore));
   * </code>
   *
   * This will link the DataStore update time to the clock time.  Changing the clock time
   * will trigger a DataStore update on the next update callback.
   */
  class SDKUTIL_EXPORT IdleClockCallback : public osg::NodeCallback
  {
  public:
    /** Provides a clock to idle, and a DataStore to update when clock time changes. */
    IdleClockCallback(simCore::ClockImpl& clock, simData::DataStore& dataStore);

    /** Overrides NodeCallback::operator() to service the callback */
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

  private:
    simCore::ClockImpl& clock_;
    simData::DataStore& dataStore_;
  };

} // end namespace simExamples

#endif /* SIMDIS_UTIL_EXAMPLE_RESOURCES_H */
