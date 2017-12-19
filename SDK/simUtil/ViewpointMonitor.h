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
#ifndef SIMUTIL_VIEWPOINTMONITOR_H
#define SIMUTIL_VIEWPOINTMONITOR_H

#include <map>
#include <memory>
#include <vector>

#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osgEarthUtil/EarthManipulator"
#include "simCore/Common/Export.h"
#include "simCore/Calc/Vec3.h"

namespace simVis {
  class View;
  class ViewManager;
  class EntityNode;
}

namespace simUtil
{

/**
 * Facade class that encapsulates the current state of the view w.r.t. eye position and the view.
 * Intended to capture the state of the view.  Responsible for querying the related parts of the
 * View to gather information into a common interface.
 */
class SDKUTIL_EXPORT EyePositionState
{
public:
  /** Initialize based on current state in a View */
  EyePositionState(simVis::View* view);
  virtual ~EyePositionState();

  /** View used to initialize this state */
  simVis::View* view() const;
  /** Node on which the view is tethered.  May be NULL if !isTethered().  In watch mode, this is the watcher node. */
  osg::Node* tetherNode() const;
  /** In watch mode, returns the node that is being watched.  May be NULL if !isWatching(). */
  simVis::EntityNode* watchedNode() const;

  /** Returns true if tethered to a node.  Must be true if isWatching() is true. */
  bool isTethered() const;
  /** Returns true if watching a node.  May only watch a node if tethered, and not in overhead mode. */
  bool isWatching() const;
  /** Returns true if in overhead mode. */
  bool isOverheadMode() const;

  /** Returns the center LLA of the eye.  This may be the tethered node's position, or a focal LLA in space; radians + meters */
  simCore::Vec3 centerLla() const;
  /** Returns the LLA of the eye itself (center LLA with range/az/el backed out); radians + meters */
  simCore::Vec3 eyeLla() const;
  /**
   * Range, Azimuth, Elevation (meters + radians) of the eye from center.  Corresponds to the same frame as the
   * osgEarth::Viewpoint class.  Specifically, -90 elevation looks straight down; 0 azimuth looks north; 90 azimuth looks east.
   */
  simCore::Vec3 rangeAzEl() const;
  /** X Y Z tangent plane offset of the eye from its center location; meters */
  simCore::Vec3 offsetXyz() const;

  /** Tether mode for the earth manipulator in the view, indicating the locked axes of the mouse. */
  osgEarth::Util::EarthManipulator::TetherMode tetherMode() const;

  /** Returns true if the heading/azimuth is locked from mouse manipulator changes. */
  bool isHeadingLocked() const;
  /** Returns true if the pitch/elevation is locked from mouse manipulator changes. */
  bool isPitchLocked() const;

private:
  /** Initializes all fields from a view pointer */
  void fillFromView_(simVis::View* view);

  osg::observer_ptr<simVis::View> view_;
  osg::observer_ptr<osg::Node> tetherNode_;
  osg::observer_ptr<simVis::EntityNode> watchedNode_;

  bool isTethered_;
  bool isWatching_;
  bool isOverheadMode_;

  simCore::Vec3 centerLla_;
  simCore::Vec3 eyeLla_;
  simCore::Vec3 rangeAzEl_;
  simCore::Vec3 offsetXyz_;
  osgEarth::Util::EarthManipulator::TetherMode tetherMode_;

  bool headingLocked_;
  bool pitchLocked_;
};

/**
 * Maintains a list of EyePositionState for every valid view and provides an Observer pattern
 * interface to be notified when view parameters change.
 */
class SDKUTIL_EXPORT ViewpointMonitor
{
public:
  /** Initialize the monitor with a non-NULL top level view */
  ViewpointMonitor(simVis::View* mainView);
  virtual ~ViewpointMonitor();

  /** Retrieves the most recently calculated eye position state for the provided view. */
  const EyePositionState* eyePositionState(simVis::View* view) const;

  /**
   * Derive your own Observer to be notified when View parameters change.  Note that the
   * callbacks may be delayed until the start of the next frame in order to simplify processing.
   */
  class Observer
  {
  public:
    virtual ~Observer() {}

    /**
     * Viewpoints can be tethered to an entity, or to a focal center point.  When the viewpoint changes
     * between them, this method is called.  Note that when in watch mode, this value remains true.
     * @param view View in which a viewpoint has become tethered or untethered.
     * @param isTethered True if the view is newly tethered (or watching), false if set to a focal point.
     */
    virtual void isTetheredChanged(simVis::View* view, bool isTethered) = 0;

    /**
     * The entity on which is being tethered has changed for the provided view.  The node is not
     * necessarily a simVis::EntityNode, but is likely either an EntityNode or a PlatformModelNode.
     * Note that the parent of a PlatformModelNode is an EntityNode.  This method is called with
     * a NULL newTether when untethering.  In watch mode, the tether node is the watcher.
     * @param view View in which the tether node has changed.
     * @param newTether Node being tethered to.  Likely either a simVis::EntityNode or a
     *    simVis::PlatformNode, but not guaranteed (EarthManipulator allows for any type of node).
     */
    virtual void tetherChanged(simVis::View* view, osg::Node* newTether) = 0;

    /**
     * simVis::View supports a Watch mode where the view is tethered to a Watcher node, and refers
     * to a Watched node.  When toggling in and out of Watch mode, this callback is issued.
     * @param view View in which Watch mode is turning on or off
     * @param isWatching True when Watch mode is enabled, false when watch is being disabled.
     */
    virtual void isWatchingChanged(simVis::View* view, bool isWatching) = 0;

    /**
     * When Watch mode is enabled, the tether node is used as the watcher node and there is
     * another node specified for the Watched node.  When the Watched node changes (including
     * being unset), this method is called.
     * @param view View in which the watched node has changed.
     * @param watchedNode Node being watched from the viewpoint of a tethered watcher node.  May
     *   be NULL when the watch mode is being disabled.
     */
    virtual void watchedChanged(simVis::View* view, simVis::EntityNode* watchedNode) = 0;

    /**
     * In overhead mode, the view looks straight down and the available mouse manipulators
     * change.  When the overhead mode flag changes, this callback is issued.
     * @param view View that has a change in overhead mode
     * @param isOverhead True when swapping into overhead mode, false when swapping out.
     */
    virtual void isOverheadChanged(simVis::View* view, bool isOverhead) = 0;

    /**
     * Viewpoints can be tethered to an entity or centered on a geodetic position.  In all
     * mouse modes, the viewpoint has either a focal point with a geodetic position, or it
     * is centered on an entity that has a geodetic position.  This is that centered geodetic
     * position, regardless of whether tethered or not.  This callback is issued when the
     * center position of the eye changes.
     * @param view View whose center position changes
     * @param lla Geodetic position of the new center point; radians and meters
     */
    virtual void centerLlaChanged(simVis::View* view, const simCore::Vec3& lla) = 0;

    /**
     * Whenever a view's focal point or tether entity is moved, its RAE changes, or its offset
     * XYZ changes, the eye's LLA will change.  This callback fires when the eye's LLA changes.
     * @param view View whose center position changes
     * @param lla Geodetic position of the new eye LLA; radians and meters.  Backed out from
     *   the center LLA by applying the range, azimuth, and elevation offsets.
     */
    virtual void eyeLlaChanged(simVis::View* view, const simCore::Vec3& lla) = 0;

    /**
     * The default SIMDIS mouse mode changes the azimuth and elevation with left click, and range
     * from the center point with the mouse wheel or third mouse button.  Whenever the range,
     * azimuth, or elevation changes, this callback is fired.
     * @param view View whose RAE is changing.
     * @param rangeAzEl Vector containing the range, azimuth, and elevation of the eye in meters
     *    and radians.  Azimuth is from 0 to M_TWOPI (0 is north; M_PI_2 is east).  Elevation is
     *    from -M_PI_2 to +M_PI_2 (-M_PI_2 looks straight down).
     */
    virtual void rangeAzElChanged(simVis::View* view, const simCore::Vec3& rangeAzEl) = 0;

    /**
     * Small position offsets can be applied to the eye center position along an X-East tangent
     * plane at the center of the eye.  This offset is useful when zoomed in to an entity and you
     * don't want it to occupy the full center of the screen, such as in Watch mode.  This callback
     * fires whenever the offset values for the given view change.
     * @param view View with changing offsets.
     * @param xyz X, Y, and Z tangent plane values in meters for the offset.
     */
    virtual void offsetXyzChanged(simVis::View* view, const simCore::Vec3& xyz) = 0;

    /**
     * The EarthManipulator supports the capability to change the tether mode so that the eye's
     * orientation can be partially or fully locked to a node's orientation.  This is used for
     * example in the Cockpit mode.  Note that although the view is locked in Watch mode, it does
     * not necessarily change the Tether mode.  This callback fires on changes to tether mode.
     * @param view View with a changing tether mode
     * @param tetherMode New tether mode for the view.
     */
    virtual void tetherModeChanged(simVis::View* view, osgEarth::Util::EarthManipulator::TetherMode tetherMode) = 0;

    /**
     * simVis::EarthManipulator has fields that let the developer lock the mouse's X/Y axes from
     * impacting the azimuth and elevation of the eye (setHeadingLocked(), setPitchLocked()).  This
     * callback fires when one of these values changes.
     * @param view View that has been changed.
     * @param isHeadingLocked True if the view's heading or azimuth is locked from end user changes, false if not
     * @param isPitchLocked True if the view's pitch or elevation is locked from end user changes, false if not
     */
    virtual void mouseAxisLockChanged(simVis::View* view, bool isHeadingLocked, bool isPitchLocked) = 0;

    /**
     * Callback fires when any of the values changes.
     * @param view View that has been changed.
     */
    virtual void changed(simVis::View* view) = 0;
  };

  /** Observers are stored as smart pointers to ease memory management */
  typedef std::shared_ptr<Observer> ObserverPtr;

  /** Adapter to the Observer interface to make deriving limited methods easier for developers only interested in a few fields. */
  class ObserverAdapter : public Observer
  {
    /** @see Observer::isTetheredChanged() */
    virtual void isTetheredChanged(simVis::View* view, bool isTethered) { }
    /** @see Observer::tetherChanged() */
    virtual void tetherChanged(simVis::View* view, osg::Node* newTether) { }
    /** @see Observer::isWatchingChanged() */
    virtual void isWatchingChanged(simVis::View* view, bool isWatching) { }
    /** @see Observer::watchedChanged() */
    virtual void watchedChanged(simVis::View* view, simVis::EntityNode* watchedNode) { }
    /** @see Observer::isOverheadChanged() */
    virtual void isOverheadChanged(simVis::View* view, bool isOverhead) { }
    /** @see Observer::centerLlaChanged() */
    virtual void centerLlaChanged(simVis::View* view, const simCore::Vec3& lla) { }
    /** @see Observer::eyeLlaChanged() */
    virtual void eyeLlaChanged(simVis::View* view, const simCore::Vec3& lla) { }
    /** @see Observer::rangeAzElChanged() */
    virtual void rangeAzElChanged(simVis::View* view, const simCore::Vec3& rangeAzEl) { }
    /** @see Observer::offsetXyzChanged() */
    virtual void offsetXyzChanged(simVis::View* view, const simCore::Vec3& xyz) { }
    /** @see Observer::tetherModeChanged() */
    virtual void tetherModeChanged(simVis::View* view, osgEarth::Util::EarthManipulator::TetherMode tetherMode) { }
    /** @see Observer::mouseAxisLockChanged() */
    virtual void mouseAxisLockChanged(simVis::View* view, bool isHeadingLocked, bool isPitchLocked) { }
    /** @see Observer::changed() */
    virtual void changed(simVis::View* view) { }
  };

  /** Add an observer that gets notified when any view parameters change. */
  void addObserver(ObserverPtr observer);
  /** Remove a previously registered observer. */
  void removeObserver(ObserverPtr observer);

private:
  /** Detects all changes on all views, firing off all change events */
  void detectAllChanges_();
  /** For a single view, detect changes from the old to new state and fire required observers */
  void detectChanges_(simVis::View* view, const EyePositionState& newState, const EyePositionState& oldState);

  void fireIsTetheredChanged_(simVis::View* view, bool isTethered);
  void fireTetherChanged_(simVis::View* view, osg::Node* newTether);
  void fireIsWatchingChanged_(simVis::View* view, bool isWatching);
  void fireWatchedChanged_(simVis::View* view, simVis::EntityNode* watchedNode);
  void fireIsOverheadChanged_(simVis::View* view, bool isOverhead);
  void fireCenterLlaChanged_(simVis::View* view, const simCore::Vec3& lla);
  void fireEyeLlaChanged_(simVis::View* view, const simCore::Vec3& lla);
  void fireRangeAzElChanged_(simVis::View* view, const simCore::Vec3& rangeAzEl);
  void fireOffsetXyzChanged_(simVis::View* view, const simCore::Vec3& xyz);
  void fireTetherModeChanged_(simVis::View* view, osgEarth::Util::EarthManipulator::TetherMode tetherMode);
  void fireMouseAxisLockChanged_(simVis::View* view, bool isHeadingLocked, bool isPitchLocked);
  void fireChanged_(simVis::View* view);

  std::vector<ObserverPtr> observers_;
  osg::observer_ptr<simVis::View> mainView_;
  class ViewManagerObserver;
  osg::ref_ptr<ViewManagerObserver> viewManagerObserver_;
  class RedrawHandler;
  osg::ref_ptr<RedrawHandler> redrawHandler_;

  std::map<simVis::View*, EyePositionState*> eyeStates_;
};

}

#endif /* SIMUTIL_VIEWPOINTMONITOR_H */
