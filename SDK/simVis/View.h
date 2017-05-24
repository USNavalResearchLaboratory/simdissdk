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
#ifndef SIMVIS_MANAGED_VIEW_H
#define SIMVIS_MANAGED_VIEW_H

#include "simVis/Entity.h"
#include "simVis/SceneManager.h"
#include "simVis/Types.h"

#include "osgViewer/View"
#include "osg/Camera"
#include "osgEarthUtil/Controls"
#include "osgEarthUtil/EarthManipulator"
#include "osgEarth/Viewpoint"
#include "simVis/ViewManager.h"

namespace simVis
{

class FocusManager;
class ViewManager;
class EarthManipulator;

/// Manipulator navigation modes
enum NavMode
{
  NAVMODE_ROTATEPAN,
  NAVMODE_GLOBESPIN,
  NAVMODE_ZOOM,
  NAVMODE_CENTERVIEW,
  NAVMODE_CENTERBOXZOOM,
  NAVMODE_BOXZOOM,
  NAVMODE_GIS
};

/// A camera viewpoint configuration
typedef osgEarth::Util::Viewpoint Viewpoint;

/// Some routines can't handle 90 elevation/pitch so limit to a close value
static const double MAX_ELEVATION_DEGREES = 89.8;

/// Name for the main view
static const std::string MAIN_VIEW_NAME = "MainView";

/**
 * Interface for a single viewport within a SIMDIS SDK application.
 *
 * A "View" is a UI rendering surface containing a 3D map. You can
 * have any number of Views in your application, all managed by a
 * ViewManager. (You need a ViewManager; a View cannot exist all by
 * itself.)
 *
 * Multiple Views can share a single SceneManager, which means they
 * all render the same map and data (though possibly from different
 * viewpoints), or a View can have its own SceneManager.
 *
 * A View can have "inset" Views. An inset is a View that shares the
 * same rendering canvas but exists in a viewport inside its host's
 * extents. We use the term "host" to refer to the View containing the
 * inset.
 */
class SDKVIS_EXPORT View : public osgViewer::View
{
public:
  /**
   * Appearance of an inset border.
   */
  struct BorderProperties
  {
    /// constructor
    BorderProperties(const simVis::Color& color, int thickness)
      : color_(color), thickness_(thickness) { }
    /// equality operator
    bool operator==(const BorderProperties& other) const
    {
      return color_ == other.color_ && thickness_ == other.thickness_;
    }
    /// Color for the border
    simVis::Color color_;
    /// Thickness of border in pixels
    int           thickness_;
  };

  /**
   * Extents defines the size of the view and whether the units
   * are in pixels or as a percentage of a host view.
   */
  struct Extents
  {
    /// Constructor for extents
    Extents(float x, float y, float width, float height, bool isRatio = false)
      : x_(x), y_(y), width_(width), height_(height), isRatio_(isRatio) { }
    /// X coordinate for the extents (either pixels or percentage based on isRatio_)
    float x_;
    /// Y coordinate for the extents (either pixels or percentage based on isRatio_)
    float y_;
    /// Width for the extents (either pixels or percentage based on isRatio_)
    float width_;
    /// Height for the extents (either pixels or percentage based on isRatio_)
    float height_;
    /// Flags whether extents are specified as absolute pixels or relative percentages
    bool  isRatio_;
  };

  /** Vector of View ref_ptr */
  typedef std::vector<osg::ref_ptr<View> > Insets;

  /** Interface for various callback events. */
  class Callback : public osg::Referenced
  {
  public:
    /// Events that this callback processes
    enum EventType
    {
      VIEW_NAME_CHANGE,
      VIEW_VISIBILITY_CHANGE,
      VIEW_COCKPIT_CHANGE,
      VIEW_ORTHO_CHANGE,
      VIEW_EXTENT_CHANGE
    };

    /// Provide this method to receive an event; will provide host_ if needed
    virtual void operator()(simVis::View* view, const EventType& e) = 0;

  protected:
    /// osg::Referenced-derived
    virtual ~Callback() {}
  };

public:
  /** Views can either be top-level, insets, or HUDs */
  enum ViewType
  {
    VIEW_TOPLEVEL = 0, ///< getHostView() will be NULL; standalone view (potentially with children)
    VIEW_INSET,        ///< getHostView() will be parent view
    VIEW_SUPERHUD      ///< getHostView() will be underlay view
  };

  /**
   * Constructs a new View.
   */
  View();

  /**
   * Returns the views "host view" if there is one.
   * See setUpViewAsInset() for a description of the host view.
   *
   * @return A managed view, or NULL if this view is independent.
   */
  simVis::View* getHostView() const;

  /**
   * Sets the view that is "host" to this view. If set, this view
   * will express its position and size relative to that of the host.
   * You usually do not need to call this directly.
   */
  void setHostView(simVis::View* view);

  /**
   * Returns a unique inset name
   * @return A unique inset name
   */
  std::string getUniqueInsetName() const;

  /**
   * Returns true if the given name is valid for a new inset name
   * @param newName Possible new name
   * @param view The view getting a new name
   * @return True if inset name is valid
   */
  bool isValidNewInsetName(const std::string& newName, const simVis::View* view) const;

  /**
   * Adds another view as an inset of this view. The inset will be
   * initialized automatically. Note: This will automatically call
   * inset->setHostView(this) so the inset tracks the extents of
   * this view.
   */
  void addInset(simVis::View* inset);

  /**
   * Removes an inset view.
   * @param[in ] inset Inset view to remove.
   */
  void removeInset(simVis::View* inset);

  /**
   * Gets a collection of inset view pointers. Note that the
   * elements in the output vector are not referenced.
   *
   * @param[out] output Insets views
   * @return Size of the output vector.
   */
  unsigned int getInsets(Insets& output) const;

  /**
   * Gets the object that manages the "focus" on inset views
   * if there are any. (Note: The object only manages the focus
   * if *insets* of this View, not this View itself.)
   */
  FocusManager* getFocusManager() const;

  /**
   * Returns the number of insets under this host.
   */
  unsigned int getNumInsets() const;

  /**
   * Retrieves the index of the view provided, or -1 if not found
   * under this host.
   */
  int getIndexOfInset(simVis::View* view) const;

  /**
   * Gets an inset by index, returning NULL on invalid index
   */
  simVis::View* getInset(unsigned int index) const;

  /**
   * Gets an inset by name, returning NULL if no inset has the name.
   */
  simVis::View* getInsetByName(const std::string& name) const;

  /**
   * Sets this view's viewport extents. This can either be absolute
   * extents (pixels) or ratio extents (percentage of a host view's
   * extents, if this is an inset view).
   *
   * @param[in ] extents Extents to which to size the view
   */
  bool setExtents(const Extents& extents);

  /**
   * Sets the view's viewport extents as a ratio of the host view's
   * extents (for insets only).
   *
   * @param[in ] x/y/width/height Percentages of the parent's size [0..1]
   */
  bool setExtentsAsRatio(float x, float y, float width, float height);

  /**
   * Current extents of the view
   */
  const Extents& getExtents() const { return extents_; }

  /**
   * Recalculates the view's extents based on the most recent call to
   * setExtents(). This is necessary if the host view's extents change
   * and this view's extents are expressed as a ratio.
   */
  void refreshExtents();

  /**
   * Tether the camera location to a scenario entity.  Transitions to the viewpoint with
   * a duration of 0 seconds.  Accounts for entity node graph to tether to correct node
   * in the platform graph.
   * @param node EntityNode or PlatformModelNode to which to tether the camera, or NULL to clear
   *   the tether.  The node will be processed through getModelNodeForTether_(), so the node in
   *   getViewpoint() may be different than the one requested.
   * @see tetherCamera(osg::Node*, const simVis::Viewpoint&, double)
   * @see setViewpoint(const simVis::Viewpoint&, double)
   */
  void tetherCamera(osg::Node *node);

  /**
   * Tether the camera location to a scenario entity and apply focal offsets. Will transition
   * to new camera view during duration specified.  Accounts for entity node graph to tether
   * to correct node in the platform graph.
   * @param node EntityNode or PlatformModelNode to which to tether the camera, or NULL to clear
   *   the tether.  The node will be processed through getModelNodeForTether_(), so the node in
   *   getViewpoint() may be different than the one requested.
   * @param vp Parameters for the viewpoint.  Tether node in Viewpoint will be replaced with
   *   the value of the parameter node (vp.getNode() is ignored).
   * @param durationSeconds  Time (seconds) over which to "fly" the camera to this position
   */
  void tetherCamera(osg::Node *node, const simVis::Viewpoint& vp, double durationSeconds);

  /**
  * Get the node to which the camera is tethered.
  * @return a node, or NULL if the camera is not tethered.
  * @see simVis::Viewpoint::getNode()
  * @see getViewpoint()
  */
  osg::Node* getCameraTether() const;

  /**
  * Get the mouse navigation mode.
  * @return mode current navigation mode
  */
  NavMode getNavigationMode() const;

  /**
  * Set the offsets of the camera relative to the current focal point.  Does not change the
  * current focal point or tether node.
  * @param heading_deg  Camera heading (degrees)
  * @param pitch_deg    Camera pitch (degrees)
  * @param range        Distance from camera to tethered node (meters)
  * @param transition_s Time (seconds) over which to "fly" the camera to this position
  */
  void setFocalOffsets(
    double heading_deg, double pitch_deg, double range,
    double transition_s = 0.0);

  /**
  * Move the camera to look at a point in space, with the specified
  * rotational offsets and range from target. You can also specify a transition
  * time in order to "fly" to the viewpoint.  Will untether from current tether node
  * if needed.
  * @param lat_deg      Latitude of camera (degrees)
  * @param lon_deg      Longitude of camera (degrees)
  * @param alt_m        Altitude of camera (meters ASL)
  * @param heading_deg  Heading of camera (degrees)
  * @param pitch_deg    Pitch of camera (degrees)
  * @param range        Distance from camera to focal point (meters)
  * @param transition_s Time (seconds) over which to "fly" the camera to this position
  */
  void lookAt(
    double lat_deg, double lon_deg, double alt_m,
    double heading_deg, double pitch_deg, double range,
    double transition_s = 0.0);

  /**
  * Set a camera view with an optional fly-to time.  This method is lower level than the
  * tetherCamera() methods, and will not process any PlatformModelNode-specific scene graph
  * adjustments to the viewpoint's tether node.
  * @param vp           Viewpoint to activate
  * @param transition_s Time (seconds) over which to "fly" the camera to this position
  * @see tetherCamera()
  * @see getViewpoint()
  */
  void setViewpoint(const Viewpoint &vp, double transition_s = 0.0);

  /**
  * Fetch the current camera parameters.
  * @return Viewpoint recording the camera's position
  */
  Viewpoint getViewpoint() const;

  /**
  * Set the mouse navigation mode.
  * @param mode Navigation mode to activate
  */
  void setNavigationMode(const NavMode &mode);

  /**
  * Enable/disable overhead mode (camera pitch locked at -90, north locked to up).
  * @param enableOverhead True=enable, false=disable
  */
  void enableOverheadMode(bool enableOverhead);

  /**
  * Tell if the view is in overhead mode.
  * @return Whether the camera is in "overhead" mode
  */
  bool isOverheadEnabled() const;

  /** Changes whether experimental clamping is enabled when in overhead mode */
  void setUseOverheadClamping(bool clamp);
  /** Retrieves whether experimental overhead clamping is enabled */
  bool useOverheadClamping() const;

  /**
  * Enable/disable cockpit mode (camera positioned in cockpit and using az/el/roll of tether platform)
  * @param tether Node to which to tether in cockpit mode, or NULL to disable
  */
  void enableCockpitMode(osg::Node* tether);

  /**
  * Tell if the view is in cockpit mode.
  * @return Whether the camera is in "cockpit" mode
  */
  bool isCockpitEnabled() const;

  /**
  * Enable/disable watch mode (camera positioned at watcher position, and pointed at watched position)
  * @param watched Node that will be watched, or NULL to disable
  * @param watcher Node that is watching, or NULL to disable
  */
  void enableWatchMode(osg::Node* watched, osg::Node* watcher);

  /**
  * Tell if the view is in watch mode.
  * @return Whether the view is in "watch" mode
  */
  bool isWatchEnabled() const;

  /**
  * Get the EntityNode that watch mode is using as the watcher
  * @return a EntityNode, or NULL if the view is not in watch mode
  */
  simVis::EntityNode* getWatcherNode() const;

  /**
   * Get the EntityNode that watch mode is using as the watched (i.e. target for the camera)
   * @return a EntityNode, or NULL if the view is not in watch mode
   */
  simVis::EntityNode* getWatchedNode() const;

  /**
   * Enable/disable an orthographic projection on the camera.
   * @param enable True=enable ortho, False=perspective
   */
  void enableOrthographic(bool enable);

  /**
   * Whether orthographic mode is enabled
   * @return True if orthographic mode is enabled
   */
  bool isOrthographicEnabled() const;

  /**
  * Add an 2D overlay control to this view.
  * @param control 2D control to add
  */
  void addOverlayControl(osgEarth::Util::Controls::Control *control);

  /**
  * Remove a 2D overlay control from this view.
  * @param control 2D control to remove
  */
  void removeOverlayControl(osgEarth::Util::Controls::Control *control);

  /**
  * Add a scene control (a 2D control that's positioned in map coordinates)
  * @param control  Control to add
  * @param location Georeferenced location of this control
  * @param priority Priority of this control over other scene controls (for conflict mitigation)
  */
  bool addSceneControl(osgEarth::Util::Controls::Control *control, const osgEarth::GeoPoint& location, float priority = 0.0f);

  /**
  * Remove a scene control.
  * @param control Control to remove
  */
  bool removeSceneControl(osgEarth::Util::Controls::Control *control);

  /**
  * Move a scene control.
  * @param control  Control to relocate
  * @param location New location for the control
  */
  bool moveSceneControl(osgEarth::Util::Controls::Control* control, const osgEarth::GeoPoint& location);

  /**
  * Toggle whether labels are allowed to overlap in this view.
  * @param value True to allow labels to overlap, false to disallow.
  */
  void setAllowLabelOverlap(bool value);

  /**
   * Configures this view as a HUD (heads up display), which is an orthographic
   * 2D transparent overlay view that tracks the extents of a host view. It
   * will always stay on top of all other views, and only responds to the
   * add/removeOverlayControl methods.
   */
  bool setUpViewAsHUD(simVis::View* host);

  /**
   * Set whether lighting is enabled for this view.
   *
   * @param value enabled True to enable surface lighting; i.e., to let the scene's
   *              lighting setup do its thing. Set to False to forcibly disable
   *              lighting on the scene content.
   */
  void setLighting(bool value);

  /**
   * Gets whether lighting is enabled for this view
   * @return True if lighting is enabled
   */
  bool getLighting() const;

  /** Changes the Field of View (Y) for the view, in degrees */
  void setFovY(double fovy);

  /** Returns the Field of View (Y) for the view, in degrees */
  double fovY() const;

  /**
   * Toggle the display of an Entity type globally.
   * @param[in ] displayMask Mask of the display types in simVis::DisplayMask.
   */
  void setDisplayMask(unsigned int displayMask);

  /**
   * Gets the active display mask.
   * @return A mask of the values in simVis::DisplayMask
   */
  unsigned int getDisplayMask() const;

  /**
   * Installs a set of event handlers for debugging (stats, state set,
   * window control, etc. (convenience function for testing)
   */
  void installDebugHandlers();

  /**
   * Installs the basic event handlers for debugging (stats, state set)
   * Useful for embedded widget viewers, since it does not add window control
   */
  void installBasicDebugHandlers();

  /**
   * Sets the appearance of the border (if this is an inset view).
   * param[in ] value Border appearance properties
   */
  void setBorderProperties(const BorderProperties& value);

  /**
  * Gets the appearance of the border (if this is an inset view).
  * return Border appearance properties
  */
  const BorderProperties& getBorderProperties() const;

  /**
   * Changes the camera node mask to show or hide view.  This may have no
   * effect on top level views, and will only affect insets.
   */
  void setVisible(bool visible);

  /**
   * Returns true if the View is visible to its parent.
   */
  bool isVisible() const;

  /** Set the name of object using C++ style string. (override from osg::Object) */
  virtual void setName(const std::string& name);

  /**
    * Install a callback that will be notified on view events.
    * @param[in ] callback Callback to install
    */
  void addCallback(Callback* callback);

  /**
   * Remove a callback installed with addCallback
   * @param[in ] callback Callback to remove
   */
  void removeCallback(Callback* callback);

  /// internal
  void setSceneManager(simVis::SceneManager* sceneMan);

  /// internal
  simVis::SceneManager* getSceneManager() const { return sceneData_.get(); }

  /// internal - gets the scene data that does NOT include the HUD.
  osg::Node* getVisibleSceneData() const;

  /// internal - convenience casting function
  template<typename T>
  T* as() { return dynamic_cast<T*>(this); }

  /// internal - convenient casting function
  template<typename T>
  const T* as() const { return dynamic_cast<const T*>(this); }

  /// Returns the camera manipulator cast to a simVis::EarthManipulator
  const simVis::EarthManipulator* getEarthManipulator() const;
  /// Returns the camera manipulator cast to a simVis::EarthManipulator
  simVis::EarthManipulator* getEarthManipulator();

  /// Copies the settings for an earth manipulator from another view's earth manipulator (e.g. when initializing insets)
  void applyManipulatorSettings(const simVis::View& copyFrom);

  /// internal - gets the HUD camera
  osg::Camera* getOrCreateHUD();

  /// handle window resize events for ratio-insets.
  void processResize(int newwidth, int newheight);

  /// assigns the view manager observer.
  void setViewManager(ViewManager* viewman);

  /// the view manager overseeing this view
  ViewManager* getViewManager() const { return viewman_.get(); }

  /// type of view either top level, inset, or super hud
  ViewType type() const;

  /**
   * Gets the entity node's model node if the specified node is an EntityNode.  Essentially
   * returns either the simVis::EntityNode, or for platforms will find the PlatformModelNode
   * under the provided EntityNode.  Tethering requires a PlatformModelNode for platforms,
   * or the EntityNode for other entity types.
   * @param node Either an EntityNode or PlatformModelNode representing the entity to tether.
   * @return the entity node of the tether node, or the original node if not a Platform Model node
   */
  osg::Node* getModelNodeForTether(osg::Node* node) const;

  /**
   * Up-casts an osg::Node to a simVis::EntityNode.  The incoming osg::Node may already be a
   * simVis::EntityNode, or it might be a simVis::PlatformModelNode.  The return is always
   * either a valid simVis::EntityNode pointer, or NULL.
   * @param node Either a simVis::EntityNode, or a simVis::PlatformModelNode, representing the
   *   entity in question.
   * @return Pointer to the EntityNode, or NULL if not able to pull out that pointer from the input.
   */
  simVis::EntityNode* getEntityNode(osg::Node* node) const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "View"; }

protected: // methods

  /// osg::Referenced-derived
  virtual ~View();

private: // methods
  /**
   * Configures this view to join another view's rendering canvas.
   * This is called by addInset().
   *
   * Each View has a rendering canvas (a.k.a. a graphics context). More than
   * one view can use a canvas - for example, you can tile Views within a canvas,
   * or you can create inset views over top of a main View.
   *
   * @param[in ] host Another view whose rendering canvas you wish to use.
   *                  The views will share the canvas and each can have its
   *                  own viewport.
   *
   * @return True upon success, false on failure (e.g., the host's graphics
   *         context does not exist)
   */
  bool setUpViewAsInset_(simVis::View* host);

  /**
  * Attach a default sky node if none currently exists
  * @param[in] sceneMgr the current scene manager node
  */
  void attachSky_(simVis::SceneManager* sceneMgr);

  /**
   * Creates a "HUD" camera for drawing overlays and inset border geometries
   */
  osg::Camera* createHUD_() const;

  /**
   * Update the watch mode view for current watcher position
   */
  void updateWatchView_();

  /** Adjusts the camera's Viewport and Projection matrix based on window extents */
  void fixProjectionForNewViewport_(double nx, double ny, double nw, double nh);

  /**
   * Sets the tether mode on the earth manipulator based on internal cockpitMode_ flag
   * @param node Node on which the view is centered
   * @param manip Earth manipulator on which to modify the tether mode.  Tether mode
   *    will be set based on state of "node" and "cockpitMode_"
   */
  void fixCockpitFlag_(osg::Node* node, osgEarth::Util::EarthManipulator* manip) const;

private: // data
  class UpdateWatchView;
  class ViewTetherCallback;

  /** This callback will update the content of the Watch view when active */
  osg::ref_ptr<osgGA::GUIEventHandler> updateWatchViewHandler_;

  /** The callback notifying when tether is broken or set */
  osg::ref_ptr<osgEarth::Util::EarthManipulator::TetherCallback> tetherCallback_;

  osg::observer_ptr<simVis::SceneManager> sceneData_;
  osg::observer_ptr<simVis::View>         host_;
  osg::ref_ptr<osg::Camera> hud_;
  osg::ref_ptr<osgEarth::Util::Controls::ControlCanvas> controlCanvas_;

  typedef std::vector< osg::ref_ptr<simVis::View> > InsetViews;
  InsetViews insets_;
  osg::observer_ptr<ViewManager> viewman_;

  bool overheadEnabled_;
  bool cockpitEnabled_;
  bool watchEnabled_;
  bool orthoEnabled_;
  NavMode currentMode_;
  osg::ref_ptr<osg::Group> sceneControls_;
  std::map<osgEarth::Util::Controls::Control*, osg::Node*> sceneControlsLUT_;
  osg::ref_ptr<osg::NodeCallback> autoClipCallback_;

  // inset border support.
  BorderProperties        borderProps_;
  osg::ref_ptr<osg::Node> borderNode_;
  osg::ref_ptr<FocusManager> focusMan_;
  /// Node that camera watches from when watchEnabled_
  osg::observer_ptr<simVis::EntityNode> watcherNode_;
  /// Node being watched when watchEnabled_
  osg::observer_ptr<simVis::EntityNode> watchedNode_;
  /// Represents the user-centric focal and position offsets from the watcherNode
  simVis::Viewpoint watchViewpoint_;
  Extents extents_;
  bool lighting_;

  /** Field of View (Y) for the view; Degrees */
  double fovy_;

  ViewType viewType_;

  typedef std::vector<osg::ref_ptr<Callback> > Callbacks;
  Callbacks callbacks_;
  void fireCallbacks_(const Callback::EventType& e);

  /// If true, then the overhead mode clamping gets used
  bool useOverheadClamping_;
  /// Points to a callback that is used for enforcing the near clipping plane when in overhead+ortho
  osg::ref_ptr<osg::NodeCallback> overheadNearFarCallback_;
};

class FocusManager;

/// Monitor the adding and removing of inset
class InsetAddDelete : public simVis::ViewManager::Callback
{
public:
  InsetAddDelete(FocusManager& parent);

  virtual void operator()(simVis::View* inset, const EventType& e);

protected:
  virtual ~InsetAddDelete();

private:
  FocusManager& parent_;
};

/// Monitors the changes to an inset
class InsetChange : public simVis::View::Callback
{
public:
  /** Constructor */
  InsetChange(FocusManager& parent);

  virtual void operator()(simVis::View* inset, const EventType& e);

protected:
  virtual ~InsetChange();

private:
  FocusManager& parent_;
};

/**
 * Manages focus among a group of inset views.
 */
class SDKVIS_EXPORT FocusManager : public osg::Referenced
{
public:

  /** Interface for focus events. */
  class Callback : public osg::Referenced
  {
  public:
    /// Events that this callback processes
    enum EventType
    {
      VIEW_FOCUSED
    };

    /// Provide this method to receive an event; will provide host_ if needed
    virtual void operator()(simVis::View* view, const EventType& e) = 0;

  protected:
    /// osg::Referenced-derived
    virtual ~Callback() {}
  };

  /** Construct a FocusManager. */
  FocusManager(simVis::View* host);

  /// assigns the view manager observer.
  void setViewManager(ViewManager* viewman);

  /** Gives a view focus. */
  void focus(simVis::View* view);

  /** Focuses the next inset in line after the currently focused inset. */
  void cycleFocus();

  /** Removes focus from all views. */
  void clearFocus();

  /** Notifies all callbacks that they need to refresh the view. */
  void reFocus();

  /** Gets the view in focus (or null). */
  simVis::View* getFocusedView() const;

  /** Gets the focus host. */
  simVis::View* getHost() const;

  /**
  * Install a callback that will be notified views are focused.
  * @param[in ] callback Callback to install
  */
  void addCallback(Callback* callback);

  /**
  * Remove a callback installed with addCallback
  * @param[in ] callback Callback to remove
  */
  void removeCallback(Callback* callback);

  /** Changes border properties for the focused inset view */
  void setFocusedBorderProperties(const simVis::View::BorderProperties& props);

  /** Changes border properties for insets that are not currently focused */
  void setUnfocusedBorderProperties(const simVis::View::BorderProperties& props);

  /** Applies the correct border properties on the view, based on whether it is focused or not */
  void applyBorderProperties(simVis::View* view) const;

  /** The given inset might have been added or deleted, should be private but I could not get InsetAddDelete as a private forward declare */
  void insetAddedOrDeleted(simVis::View* inset, const simVis::ViewManager::Callback::EventType& e);

protected:
  virtual ~FocusManager();

private:
  typedef std::vector< osg::ref_ptr<Callback> > Callbacks;

  void fireCallbacks_(simVis::View* view, const Callback::EventType& e);

  osg::observer_ptr<simVis::View> host_;
  osg::observer_ptr<ViewManager> viewman_;
  osg::observer_ptr<simVis::View> focused_;
  simVis::View::BorderProperties  borderIdle_;
  simVis::View::BorderProperties  borderFocus_;
  Callbacks callbacks_;
  osg::ref_ptr<InsetAddDelete> viewManagerCB_; ///< observer for monitoring for inset changes
  std::map< simVis::View*, osg::ref_ptr<InsetChange> > insets_;  ///< Keep track of inset callbacks
};

} // namespace simVis

#endif // SIMVIS_MANAGED_VIEW_H
