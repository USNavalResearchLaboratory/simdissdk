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
#ifndef SIMVIS_COMPASS_H
#define SIMVIS_COMPASS_H

#include <memory>
#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "osg/MatrixTransform"
#include "simCore/Common/Export.h"
#include "simData/DataStore.h"

namespace osgText { class Text; }

namespace simVis
{

class CompassNode;
class FocusManager;
class View;

/**
 * Adapter class that allows compass to work with FocusManager to switch its display to reflect newly focused view
 */
class SDKVIS_EXPORT CompassFocusManagerAdapter
{
public:
  CompassFocusManagerAdapter(simVis::FocusManager* focusManager, simVis::CompassNode* compass);
  virtual ~CompassFocusManagerAdapter();

  /**
  * Tell our compass to focus this view, which may be nullptr
  * @param focusedView Swap the compass to report values from this view (make it active)
  */
  void focusView(simVis::View* focusedView);

private:
  class FocusCallback;
  osg::observer_ptr<simVis::FocusManager> focusManager_;
  osg::observer_ptr<simVis::CompassNode> compass_;
  osg::ref_ptr<FocusCallback> callback_;
};

/** Callback to a datastore you can add to synchronize its wind values to a compass. */
class SDKVIS_EXPORT UpdateWindVaneListener : public simData::DataStore::ScenarioListener
{
public:
  explicit UpdateWindVaneListener(simVis::CompassNode* compass);

  /** Override to pass along wind values */
  virtual void onScenarioPropertiesChange(simData::DataStore* source);

private:
  osg::observer_ptr<simVis::CompassNode> compass_;
};

/**
 * Representation of a Compass Node that gets used in SIMDIS.  This is an image with text
 * that uses a 128x128 image to draw a compass on-screen.  The anchor point for the compass
 * is in the middle of the compass area.
 *
 * This class also shows a wind vane, if configured.  The wind vane can be enabled by
 * calling setWindVaneVisible(true) and using setWindParameters().  To have the wind vane
 * show the values from the Data Store, you can attach an UpdateWindVaneCallback to the
 * data store.
 */
class SDKVIS_EXPORT CompassNode : public osg::MatrixTransform
{
public:
  explicit CompassNode(const std::string& compassFilename);

  /**
   * Tell the Compass to show the specified view's heading
   * @param activeView View whose compass values to display (not necessarily the
   *   same as the view in which the compass is drawn)
   */
  void setActiveView(simVis::View* activeView);
  /** Retrieve the active view */
  simVis::View* activeView() const;

  /**
   * Get the width/height size of the image in pixels. Width and height are the same
   * @return size of the compass image in pixels, returns 0 if no image found
   */
  int size() const;

  /** Sets whether the wind vane is visible or not.  By default, the wind vane is not visible. */
  void setWindVaneVisible(bool visible);
  /** Returns whether the wind vane is visible. */
  bool isWindVaneVisible() const;

  /**
   * Updates the wind vane "direction from", and the speed.  Use a
   * UpdateWindVaneCallback to update it from the data store values.
   */
  void setWindParameters(double angleRad, double speedMs);

  /// Override traverse() to update the compass based on the view
  virtual void traverse(osg::NodeVisitor& nv);

protected:
  /** Protect destructor to avoid ref_ptr double delete issue */
  virtual ~CompassNode();

  /** Retrieves the last heading in degrees */
  double getHeading_() const;
  /** Updates the orientation of the compass to point in the right direction. */
  virtual void updateCompass_();

private:
  /** Initializes the nodes for the compass part. */
  void initCompass_(const std::string& compassFilename);
  /** Initializes the nodes for the wind vane part. */
  void initWindVane_();

  /// Reference to the view whose data the compass is showing
  osg::observer_ptr<simVis::View> activeView_;
  /// Contains the compass image node and is rotated around
  osg::ref_ptr<osg::MatrixTransform> compassImageXform_;
  /// Read-out text for the compass angle
  osg::ref_ptr<osgText::Text> valueText_;
  /// Last heading shown by the compass, required to keep the callbacks correct
  double lastHeadingDeg_;

  /// Holds the image of the wind vane
  osg::ref_ptr<osg::MatrixTransform> windVaneImage_;
  /// Groups together the two texts for easy show/hide
  osg::ref_ptr<osg::Group> windVaneTexts_;
  /// Shows the speed of the wind
  osg::ref_ptr<osgText::Text> windSpeedText_;
  /// Shows the direction of the wind
  osg::ref_ptr<osgText::Text> windFromText_;
};

/// define an interface for listeners for compass heading updates
class SDKVIS_EXPORT CompassUpdateListener
{
public:
  CompassUpdateListener() {}
  virtual ~CompassUpdateListener() {}

  /** Executed when the compass heading changes, passes in heading in degrees */
  virtual void onUpdate(double heading) = 0;
};

/// Shared pointer to a CompassUpdateListener
typedef std::shared_ptr<CompassUpdateListener> CompassUpdateListenerPtr;

/**
 * Creates a Compass which can be displayed as a HUD "widget" in a single view.  The
 * Compass is drawn on a single view, but may reflect the heading of a different view.
 * The view on which it is drawn is the Draw View (setDrawView(), drawView(), and
 * removeFromView()).  The view from which it pulls heading values is the Active view
 * (setActiveView()).  In single-view situations, these are often the same.  When
 * using insets, they may differ.  See the CompassFocusManagerAdapter class for an
 * easy way to tie focus-view changes to the setActiveView() method.
 */
class SDKVIS_EXPORT Compass : public CompassNode
{
public:
  /** Constructs a new Compass */
  explicit Compass(const std::string& compassFilename);

  /**
   * Set our listener
   * @param listener Observer for the compass updates
   */
  void setListener(simVis::CompassUpdateListenerPtr listener);

  /**
   * Unset our listener
   * @param listener Observer for the compass updates
   */
  void removeListener(const simVis::CompassUpdateListenerPtr& listener);

  /**
   * Display the Compass node as an overlay in the specified view
   * @param drawView View on which the compass is drawn (in lower right corner).  May
   *   be different than the active view, which feeds the heading values for compass.
   *   Passing in nullptr is equivalent to calling removeFromView().
   */
  void setDrawView(simVis::View* drawView);

  /**
   * Remove the Compass node from the draw view, hiding it.  No effect if the
   * compass is not currently being drawn.
   */
  void removeFromView();

  /** Retrieves the current draw view (may be nullptr) */
  simVis::View* drawView() const;

  /// Override traverse() to adjust the active view in some cases
  virtual void traverse(osg::NodeVisitor& nv);

protected:
  /** Destructor */
  virtual ~Compass();

  /** Override to fire off callbacks. */
  virtual void updateCompass_();

private:
  class RepositionEventHandler;
  /// Event Handler that repositions the compass to the lower-right
  osg::ref_ptr<RepositionEventHandler> repositionEventHandler_;

  /// Pointer to the view on which to overlay the compass
  osg::observer_ptr<simVis::View> drawView_;
  /// Listener for our updates, if any
  simVis::CompassUpdateListenerPtr compassUpdateListener_;
};

} // namespace simVis

#endif // SIMVIS_COMPASS_H
