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
#ifndef SIMVIS_EYE_POSITION_MANAGER_H
#define SIMVIS_EYE_POSITION_MANAGER_H

#include <string>
#include "osgEarthUtil/EarthManipulator"
#include "simCore/Common/Memory.h"
#include "simVis/View.h"

namespace simVis {

/** Common define for referencing the action of saving an eye position */
static const std::string SAVE_EYE_POSITION_TITLE = "Save Eye Position";
/** Common define for referencing the action of quickly saving an eye position */
static const std::string SAVE_INSTANT_EYE_POSITION_TITLE = "Save Instant Eye Position";

/** Describes the contents of an eye position. */
class EyePosition
{
public:
  virtual ~EyePosition() {}

  /** String-based name of the eye position, unique inside the Eye Position Manager. */
  virtual std::string name() const = 0;
  /** Position in 3-D space for the eye position. */
  virtual simVis::Viewpoint viewpoint() const = 0;
  /**
    * Entity name to which the view is tethered on. Either always returns real name or returns real/alias name depending  on the usealias preference
    * @param respectAliasFlag  true if the alias name is desired when useAlias is set to true in the prefs, false to always return real name.
    * @param dataStore         pointer to the dataStore, used to lookup the alias name
    * @return                  real/alias entity name string
    */
  virtual std::string tetherName(bool respectAliasFlag, const simData::DataStore& dataStore) const = 0;
  /**
    * Entity name to which the view is watching. Either always returns real name or returns real/alias name depending on the usealias preference
    * @param respectAliasFlag  true if the alias name is desired when useAlias is set to true in the prefs, false to always return real name.
    * @param dataStore         pointer to the dataStore, used to lookup the alias name
    * @return                  real/alias entity name string
    */
  virtual std::string watchName(bool respectAliasFlag, const simData::DataStore& dataStore) const = 0;
  /// retrieves objectId of the platform that the view is tethered on
  virtual simData::ObjectId tetherId() const = 0;
  /// retrieves objectId of the platform tha the view is watching
  virtual simData::ObjectId watchId() const = 0;
  /** Flagged true when the view was in overhead mode. */
  virtual bool overheadMode() const = 0;
  /** If true, the tether node is ignored when transitioning to this view, and instead the current tether is used. */
  virtual bool replaceCentered() const = 0;

  /** When true, azimuth changes from the mouse are ignored.  @see simVis::EarthManipulator::isHeadingLocked() */
  virtual bool lockMouseHeading() const = 0;
  /** When true, azimuth changes from the mouse are ignored.  @see simVis::EarthManipulator::isPitchLocked() */
  virtual bool lockMousePitch() const = 0;
  /** Indicates whether camera angles are offset by the host entity's yaw and/or pitch.  @see osgEarth::Util::EarthManipulator::tetherMode() */
  virtual osgEarth::Util::EarthManipulator::TetherMode tetherMode() const = 0;
};

/** Interface to eye position related functionality.  Provides an easy-to-use interface for working with eye positions. */
class EyePositionManager
{
public:
  virtual ~EyePositionManager() {}

  /// Removes all eye positions
  virtual void reset() = 0;

  /// Parent of callbacks to receive inset-view events
  class EyePositionCallback
  {
  public:
    /// event types
    enum EventType
    {
      ADDED = 0,
      REMOVED,
      RENAMED
    };

    virtual ~EyePositionCallback() {}

    /// Provide this method to receive an event
    virtual void operator()(EyePosition *eyePos, const EventType &e) = 0;
  };

  /// Shared pointer for an EyePositionCallback
  typedef std::shared_ptr<EyePositionCallback> EyePositionCallbackPtr;

  /// Adds a callback that is notified on eye position changes
  virtual void addCallback(EyePositionCallbackPtr cb) = 0;

  /// Removes a callback added from addCallback().
  virtual void removeCallback(EyePositionCallbackPtr cb) = 0;

  /// parse a SIMDIS 9 or SIMDIS 10 view string to create an eye position
  virtual void createEyePosition(const std::string &viewString) = 0;

  /// apply the given SIMDIS 10 view string to the given viewport
  virtual int applyEyePosition(const std::string &viewString, simVis::View *viewport) = 0;

  /// load a SIMDIS 9 or SIMDIS 10 view file
  virtual int loadFile(const std::string &fileName) = 0;

  /// load only a SIMDIS 10 view file
  virtual int loadFile(std::istream& is) = 0;

  /// save a SIMDIS 10 view file
  virtual int saveFile(const std::string &fileName) = 0;

  /// save a SIMDIS 10 view file with an option to include the defined eye positions
  virtual int saveFile(std::ostream& os, bool includeEyePositions) = 0;

  /// retrieve the view port's eye position string representation in SIMDIS 10 XML format
  virtual std::string eyePositionString(simVis::View *viewport) = 0;

  /// retrieve the inset's string representation in SIMDIS 10 XML format
  virtual std::string insetString(simVis::View *viewport) = 0;

  /// move to the next (or previous) eye position in the given view port
  virtual void cycleEyeView(simVis::View *viewport, bool forwardCycle) = 0;

  /// retrieve eye position with the given name
  virtual EyePosition* eyePositionByName(const std::string &eyePositionName) = 0;

  /// make the view port camera use the given eye position
  virtual void applyEyePositionToPort(EyePosition *eyePosition, simVis::View *viewport) = 0;

  /// delete the specified eye position, and remove it from the system
  virtual void removeEyePosition(EyePosition *eyePosition) = 0;

  /// delete the specified eye position, and remove it from the system
  virtual void removeEyePosition(const std::string& name) = 0;

  /// create a smooth transition for the view port to the new eye position
  virtual void moveToEyePosition(EyePosition *eyePosition, simVis::View *viewport, double duration) = 0;

  /// retrieve all the current loaded eye positions
  virtual void getEyePositions(std::vector<EyePosition*>& eyePositions) const = 0;

  /// retrieves real name of the platform that the view is tethered on
  virtual std::string tetherName(const EyePosition& eyePos) const = 0;

  /// retrieves real name of the platform the view is watching
  virtual std::string watchName(const EyePosition& eyePos) const = 0;
};

/** Null object implementation for EyePositionManager */
class NullEyePositionManager : public simVis::EyePositionManager
{
public:
  virtual void reset() {}
  virtual void addCallback(EyePositionCallbackPtr cb) { }
  virtual void removeCallback(EyePositionCallbackPtr cb) { }
  virtual void createEyePosition(const std::string &viewString) { }
  virtual int applyEyePosition(const std::string &viewString, simVis::View *viewport) { return 1; }
  virtual int loadFile(const std::string& fileName) { return 1; }
  virtual int loadFile(std::istream& is) { return 1; }
  virtual int saveFile(const std::string& fileName) { return 1; }
  virtual int saveFile(std::ostream& os, bool includeEyePositions) { return 1; }
  virtual std::string eyePositionString(simVis::View *viewport) { return ""; }
  virtual std::string insetString(simVis::View *viewport) { return ""; }
  virtual void cycleEyeView(simVis::View *viewport, bool forwardCycle) { }
  // Treated as an opaque pointer by consumers
  virtual simVis::EyePosition* eyePositionByName(const std::string &eyePositionName) { return NULL; }
  virtual void applyEyePositionToPort(simVis::EyePosition *eyePosition, simVis::View *viewport) { }
  virtual void removeEyePosition(simVis::EyePosition *eyePosition) { }
  virtual void removeEyePosition(const std::string& name) { }
  virtual void renameEyePosition(simVis::EyePosition* eyePosition, const std::string& newName) { }
  virtual void moveToEyePosition(simVis::EyePosition *eyePosition, simVis::View *viewport, double duration) { }
  virtual void getEyePositions(std::vector<simVis::EyePosition*>& eyePositions) const { }
  virtual std::string tetherName(const simVis::EyePosition& eyePos) const { return ""; }
  virtual std::string watchName(const simVis::EyePosition& eyePos) const { return ""; }
};

} // simVis

#endif /* SIMVIS_EYE_POSITION_MANAGER_H */
