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
#include <algorithm>
#include <cassert>
#include "osg/Depth"
#include "osgGA/StateSetManipulator"
#include "osgViewer/ViewerEventHandlers"
#include "osgEarth/GLUtils"
#include "osgEarth/MapNode"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/Version"
#include "osgEarth/CullingUtils"
#include "osgEarthUtil/Sky"

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simNotify/Notify.h"

#include "simVis/osgEarthVersion.h"
#include "simVis/EarthManipulator.h"
#include "simVis/Entity.h"
#include "simVis/Gate.h"
#include "simVis/NavigationModes.h"
#include "simVis/OverheadMode.h"
#include "simVis/CustomRendering.h"
#include "simVis/PlatformModel.h"
#include "simVis/Popup.h"
#include "simVis/Registry.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/View.h"

namespace
{
static const float DEFAULT_VFOV = 60.0f; // Degrees
static const float DEFAULT_NEAR = 1.0f; // Meters
static const float DEFAULT_FAR = 10000.0f; // Meters
static const float MINIMUM_FOCAL_POINT_DISTANCE = -100.0f; // minimum camera zoom distance, Meters

/**
 * Internal geode that renders a border for an inset view.
 */
class BorderNode : public osg::Geode
{
public:
  BorderNode() : osg::Geode(), props_(simVis::Color::White, 2)
  {
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    geom->setName("simVis::BorderNode");
    geom->setUseVertexBufferObjects(true);
    geom->setDataVariance(osg::Object::DYNAMIC);

    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(10);
    geom->setVertexArray(verts.get());

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    geom->setColorArray(colors.get());

    geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 10));
    this->addDrawable(geom.get());

    simVis::setLighting(geom->getOrCreateStateSet(),
        osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    osg::ref_ptr<osg::Viewport> vp = new osg::Viewport(10.f, 10.f, 20.f, 20.f);
    set(vp.get());
  }

  void set(const osg::Viewport* vp)
  {
    set(vp, props_);
  }

  void set(const osg::Viewport* vp, const simVis::View::BorderProperties& props)
  {
    props_ = props;

    const float x = 0.0f;
    const float y = 0.0f;
    const float w = vp->width() - 1.f;  // Offset width and height by 1 to avoid border problem
    const float h = vp->height() - 1.f;
    const int t = props.thickness_;

    osg::ref_ptr<osg::Vec3Array> verts = static_cast<osg::Vec3Array*>(this->getDrawable(0)->asGeometry()->getVertexArray());
    (*verts)[0].set(x+t,   y+h-t, 0);
    (*verts)[1].set(x,     y+h,   0);
    (*verts)[2].set(x+t,   y+t,   0);
    (*verts)[3].set(x,     y,     0);
    (*verts)[4].set(x+w-t, y+t,   0);
    (*verts)[5].set(x+w,   y,     0);
    (*verts)[6].set(x+w-t, y+h-t, 0);
    (*verts)[7].set(x+w,   y+h,   0);
    (*verts)[8].set(x+t,   y+h-t, 0);
    (*verts)[9].set(x,     y+h,   0);
    verts->dirty();

    osg::ref_ptr<osg::Vec4Array> colors = static_cast<osg::Vec4Array*>(this->getDrawable(0)->asGeometry()->getColorArray());
    (*colors)[0] = props.color_;
    colors->dirty();

    // if the thickness is zero, don't draw it at all
    this->setNodeMask(t > 0 ? ~0 : 0);
  }

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "BorderNode"; }

  simVis::View::BorderProperties props_;
};

/// Cull callback that sets the N/F planes on an orthographic camera.
struct SetNearFarCallback : public osg::NodeCallback
{

  SetNearFarCallback()
  {
    // create a state set to turn off depth buffer when in overhead mode.
    // note: this will override the depth settings in the TwoPassAlphaRenderBin, and 
    // that's OK because we don't care about TPA when the depth buffer is off.
    depthState_ = new osg::StateSet();
    depthState_->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false),
      osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
  }

  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);

    // apply depth attribute when in overhead mode
    if (cv)
      cv->pushStateSet(depthState_.get());

    traverse(node, nv);
    if (cv)
    {
      cv->popStateSet();
      osg::Vec3d eye = osg::Vec3d(0, 0, 0)* cv->getCurrentCamera()->getInverseViewMatrix(); //cv->getCurrentCamera()->getViewMatrix().getTrans(); //osg::Vec3d(0,0,0)*(*cv->getModelViewMatrix());
      double eyeR = eye.length();
      const double earthR = simCore::EARTH_RADIUS;
      double eyeAlt = std::max(0.0, eyeR - earthR);
      const double gsoAlt = 35786000.0; // Geosynchronous orbit altitude (GS)
      double L, R, B, T, N, F;
      cv->getCurrentCamera()->getProjectionMatrixAsOrtho(L, R, B, T, N, F);
      N = eyeAlt - gsoAlt;
      F = eyeR;
      cv->getCurrentCamera()->setProjectionMatrixAsOrtho(L, R, B, T, N, F);
    }
  }
private:
  osg::ref_ptr<osg::StateSet> depthState_;
};

} // namespace

//-------------------------------------------------------------------

#undef  LC
#define LC "[FocusManager] "

namespace simVis
{

InsetAddDelete::InsetAddDelete(FocusManager& parent)
  : parent_(parent)
{
}

InsetAddDelete::~InsetAddDelete()
{
}

void InsetAddDelete::operator()(simVis::View* inset, const EventType& e)
{
  parent_.insetAddedOrDeleted(inset, e);
}


//-------------------------------------------------------------------------

InsetChange::InsetChange(FocusManager& parent)
  : parent_(parent)
{
}

InsetChange::~InsetChange()
{
}

void InsetChange::operator()(simVis::View* inset, const EventType& e)
{
  // If the given inset has focus, but goes invisible than clear the focus
  if (inset == parent_.getFocusedView())
  {
    if ((e == simVis::View::Callback::VIEW_VISIBILITY_CHANGE) && !inset->isVisible())
      parent_.clearFocus();  // earlier listeners will get visibility change first followed by focus change; later listeners will get the reverse order
  }
}


//-------------------------------------------------------------------------

FocusManager::FocusManager(simVis::View* host)
 : host_(host),
   viewman_(NULL),
   focused_(NULL),
   borderIdle_(simVis::Color::White,  2),
   borderFocus_(simVis::Color::Yellow, 3)
{
  if (host != NULL)
    host->setBorderProperties(borderIdle_);
}

FocusManager::~FocusManager()
{
  setViewManager(NULL);
}

void FocusManager::setViewManager(simVis::ViewManager* viewman)
{
  if (viewman_ == viewman)
    return;

  if (viewman_.valid())
  {
    viewman_->removeCallback(viewManagerCB_.get());
    viewManagerCB_ = NULL;

    for (std::map< simVis::View*, osg::ref_ptr<InsetChange> >::const_iterator it = insets_.begin(); it != insets_.end(); ++it)
      it->first->removeCallback(it->second.get());
  }

  insets_.clear();

  viewman_ = viewman;

  if (!viewman_.valid())
    return;

  viewManagerCB_ = new InsetAddDelete(*this);
  viewman_->addCallback(viewManagerCB_.get());

  std::vector<simVis::View*> views;
  viewman_->getViews(views);
  for (std::vector<simVis::View*>::const_iterator it = views.begin(); it != views.end(); ++it)
  {
    // ignore VIEW_TOPLEVEL and VIEW_SUPERHUD
    if ((*it)->type() != simVis::View::VIEW_INSET)
      continue;
    InsetChange* callback = new InsetChange(*this);
    insets_[*it] = callback;
    (*it)->addCallback(callback);
  }
}

void FocusManager::insetAddedOrDeleted(simVis::View* inset, const simVis::ViewManager::Callback::EventType& e)
{
  // No need to monitor Super HUD
  if (inset->type() == simVis::View::VIEW_SUPERHUD)
    return;

  switch (e)
  {
  case simVis::ViewManager::Callback::VIEW_REMOVED:
  {
    // The insert is about to be deleted so no need to remove callback
    insets_.erase(inset);
    break;
  }

  case simVis::ViewManager::Callback::VIEW_ADDED:
    InsetChange* callback = new InsetChange(*this);
    insets_[inset] = callback;
    inset->addCallback(callback);
    break;
  }
}

simVis::View* FocusManager::getFocusedView() const
{
  return focused_.valid() ? focused_.get() : host_.get();
}

simVis::View* FocusManager::getHost() const
{
  return host_.get();
}

void FocusManager::focus(simVis::View* view)
{
  if (!view)
  {
    // There should be no callback unless there is a non-NULL value
    assert(0);
    return;
  }

  if (view == host_.get())
  {
    clearFocus();
  }
  else if (host_.valid() && view != focused_.get())
  {
    simVis::View::Insets insets;
    host_->getInsets(insets);
    for (unsigned int i = 0; i < insets.size(); ++i)
    {
      if (insets[i].get() == view)
        insets[i]->setBorderProperties(borderFocus_);
      else
        insets[i]->setBorderProperties(borderIdle_);
    }
    focused_ = view;

    SIM_DEBUG << LC << "Focus: " << view->getName() << ", num insets = " <<  insets.size() << std::endl;
    fireCallbacks_(focused_.get(), Callback::VIEW_FOCUSED);
  }
}

void FocusManager::setFocusedBorderProperties(const simVis::View::BorderProperties& props)
{
  if (borderFocus_ == props)
    return;
  borderFocus_ = props;
  osg::observer_ptr<simVis::View> focus = getFocusedView();
  if (focus.valid())
    focus->setBorderProperties(props);
}

void FocusManager::setUnfocusedBorderProperties(const simVis::View::BorderProperties& props)
{
  if (borderIdle_ == props)
    return;
  borderIdle_ = props;

  // Update unfocused view properties
  if (host_.valid())
  {
    host_->setBorderProperties(borderIdle_);
    osg::observer_ptr<simVis::View> focus = getFocusedView();

    // Iterate over all insets
    simVis::View::Insets insets;
    host_->getInsets(insets);
    for (unsigned int i = 0; i < insets.size(); ++i)
    {
      // Only set border properties for unfocused views
      if (insets[i].get() != focus.get())
        insets[i]->setBorderProperties(borderIdle_);
    }
  }
}

void FocusManager::applyBorderProperties(simVis::View* view) const
{
  if (view != NULL)
  {
    if (view == focused_.get())
      view->setBorderProperties(borderFocus_);
    else
      view->setBorderProperties(borderIdle_);
  }
}

void FocusManager::cycleFocus()
{
  if (host_.valid())
  {
    simVis::View::Insets insets;
    host_->getInsets(insets);

    if (insets.size() > 0)
    {
      if (!focused_.valid())
      {
        focus(insets.begin()->get());
      }
      else
      {
        for (simVis::View::Insets::iterator i = insets.begin(); i != insets.end(); ++i)
        {
          if (i->get() == focused_.get())
          {
            ++i;
            if (i != insets.end())
              focus(i->get());
            else
              focus(insets.begin()->get());
            break;
          }
        }
      }
    }
  }
}

void FocusManager::clearFocus()
{
  if (focused_.valid())
  {
    SIM_DEBUG << LC << "clear focus" << std::endl;
  }

  if (host_.valid())
  {
    simVis::View::Insets insets;
    host_->getInsets(insets);
    for (unsigned int i = 0; i < insets.size(); ++i)
      insets[i]->setBorderProperties(borderIdle_);
  }
  focused_ = NULL;
  fireCallbacks_(NULL, Callback::VIEW_FOCUSED);
}

void FocusManager::reFocus()
{
  fireCallbacks_(getFocusedView(), Callback::VIEW_FOCUSED);
}

void FocusManager::addCallback(FocusManager::Callback* callback)
{
  if (callback)
    callbacks_.push_back(callback);
}

void FocusManager::removeCallback(FocusManager::Callback* callback)
{
  if (callback)
  {
    Callbacks::iterator i = std::find(callbacks_.begin(), callbacks_.end(), callback);
    if (i != callbacks_.end())
      callbacks_.erase(i);
  }
}

void FocusManager::fireCallbacks_(simVis::View* view, const FocusManager::Callback::EventType& e)
{
  if (view == NULL)
    view = host_.get();
  for (Callbacks::const_iterator i = callbacks_.begin(); i != callbacks_.end(); ++i)
    (*i)->operator()(view, e);
}

//-------------------------------------------------------------------

/**
 * Event handler that will process frame events to update watch view modes.
 */
class View::UpdateWatchView : public osgGA::GUIEventHandler
{
public:
  /**  constructor */
  explicit UpdateWatchView(simVis::View* view)
    : view_(view),
      active_(false)
  {
  }

  /** Process the frame event and if watch mode enabled, update it */
  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*)
  {
    if (active_ && ea.getEventType() == osgGA::GUIEventAdapter::FRAME && view_.valid() && view_->isWatchEnabled())
      view_->updateWatchView_();
    return false;
  }

  /** Enable watch mode updating */
  void setActive(bool active)
  {
    active_ = active;
  }

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "View::UpdateWatchView"; }

private:
  osg::observer_ptr<View> view_;
  bool active_;
};

/** Class to manage the callback from the earth manipulator when tether is broken.
* Will disable view's watch mode and cockpit mode when tether is broken */
class View::ViewTetherCallback : public osgEarth::Util::EarthManipulator::TetherCallback
{
public:
  /// Construct the callback class
  explicit ViewTetherCallback(simVis::View* view) : view_(view)
  {}
  /// Process the change-of-tether event
  void operator()(osg::Node* node)
  {
    // if node is NULL, tether is broken
    if (node == NULL && view_.valid())
    {
      if (view_->isWatchEnabled())
        view_->enableWatchMode(NULL, NULL);
      if (view_->isCockpitEnabled())
        view_->enableCockpitMode(NULL);

      // Note that the mouse azim/elev locks associated with Watch or Cockpit mode are
      // not unlocked here.  They are conditionally unlocked in either the enableWatchMode()
      // method, or the enableCockpitMode() method.
    }
  }

private:
  osg::observer_ptr<View> view_;
};

//------------------------------------------------------------------------

#undef  LC
#define LC "[View] "


View::View()
 : overheadEnabled_(false),
   cockpitEnabled_(false),
   watchEnabled_(false),
   orthoEnabled_(false),
   borderProps_(simVis::Color::White,  2),
   extents_(0, 0, 200, 100, false),
   lighting_(true),
   fovXEnabled_(false),
   fovXDeg_(60.0),
   fovYDeg_(DEFAULT_VFOV),
   viewType_(VIEW_TOPLEVEL),
   useOverheadClamping_(true),
   overheadNearFarCallback_(new SetNearFarCallback),
   updateCameraNodeVisitor_(NULL)
{
  // start out displaying all things.
  setDisplayMask(simVis::DISPLAY_MASK_ALL);

  // create tether callback
  tetherCallback_ = new ViewTetherCallback(this);
  // Create and add the callback for updating watch views (so it doesn't get added/removed at bad times)
  updateWatchViewHandler_ = new UpdateWatchView(this);
  static_cast<UpdateWatchView*>(updateWatchViewHandler_.get())->setActive(false);
  addEventHandler(updateWatchViewHandler_);

  // attach an earth manipulator to it, and install the startup nav mode.
  simVis::EarthManipulator* manip = new simVis::EarthManipulator();
  // Initialize good default settings
  manip->getSettings()->setTerrainAvoidanceEnabled(false);
  manip->getSettings()->setArcViewpointTransitions(false);
  manip->getSettings()->setMinMaxPitch(-89, 60.0);
  manip->setTetherCallback(tetherCallback_.get());
  setCameraManipulator(manip);

  setNavigationMode(NAVMODE_ROTATEPAN);

  // lighting is OFF by default.
  setLighting(false);

  // install a root group.
  osg::Group* root = new osg::Group();
  osgViewer::View::setSceneData(root);

  // Ready the overhead mode. This just installs the uniforms; it does not
  // activate the actual overhead mode on the view.
  OverheadMode::install(root);

  // install a control canvas for UI elements
  controlCanvas_ = new osgEarth::Util::Controls::ControlCanvas();
  root->addChild(controlCanvas_.get());

  // install a group for 'scene controls' like a platform pop up
  sceneControls_ = new osg::Group();
  root->addChild(sceneControls_.get());

  // initial camera configuration
  // disable 'small feature culling'
  osg::Camera* thisCamera = this->getCamera();
  thisCamera->setCullingMode(thisCamera->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING);

  // default our background to black
  thisCamera->setClearColor(simVis::Color::Black);

  // focus manager for insets, if present.
  focusMan_ = new FocusManager(this);

  // Apply the new viewport and new perspective matrix
  getCamera()->setProjectionMatrixAsPerspective(fovY(), extents_.width_ / extents_.height_, 1.f, 10000.f);

  // Install a viewport uniform on each camera, giving all shaders access
  // to the window size. The osgEarth::LineDrawable construct uses this.
  getCamera()->addCullCallback(new osgEarth::InstallViewportSizeUniform());

  // set global defaults for LineDrawable
  osgEarth::GLUtils::setGlobalDefaults(getCamera()->getOrCreateStateSet());
  osgEarth::GLUtils::setPointSmooth(getCamera()->getOrCreateStateSet(), osg::StateAttribute::ON);
}

View::~View()
{
  // remove our tether callback
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
  {
    manip->setTetherCallback(0L);
    manip->clearViewpoint();
  }
  // if we have insets, remove them.
  insets_.clear();

  // if this was an inset view, tear it down.
  simVis::View* host = getHostView();
  if (host)
  {
    osg::Group* bordercamera = this->getOrCreateHUD();
    if (bordercamera)
    {
      bordercamera->removeChild(borderNode_.get());
    }
  }

  // Get rid of the Watch updater
  removeEventHandler(updateWatchViewHandler_);
}

void View::setDisplayMask(unsigned int mask)
{
  this->getCamera()->setCullMask(mask);
}

unsigned int View::getDisplayMask() const
{
  return this->getCamera()->getCullMask();
}

bool View::setUpViewAsHUD(simVis::View* host)
{
  bool ok = true;

  if (host && host->getCamera())
  {
    osg::GraphicsContext* gc = host->getCamera()->getGraphicsContext();
    if (!gc)
    {
      SIM_WARN << LC << "Host has no graphics context, cannot share!" << std::endl;
      ok = false;
    }

    // if the user hasn't created a camera for this view, do so now.
    osg::Camera* camera = this->getCamera();

    // render this view just before the canvas; that way it will
    // always render atop everything else.
    camera->setRenderOrder(osg::Camera::POST_RENDER, controlCanvas_->getRenderOrderNum()+1);

    // tell the camera to use the same GC as the "host".
    camera->setGraphicsContext(gc);

    camera->setViewport(new osg::Viewport());

    // don't clear the frame buffer
    camera->setClearMask(0);

    // ignore events and pass them through
    camera->setAllowEventFocus(false);

    // don't need this
    setCameraManipulator(0L);

    // save a reference to the host.
    host_ = host;
    viewType_ = VIEW_SUPERHUD;

    // set viewport to full extent of the host.
    setExtentsAsRatio(0, 0, 1, 1);
  }
  else
  {
    SIM_WARN << LC << "setUpViewAsHUD: Host view is not set up or is NULL." << std::endl;
    ok = false;
  }
  return ok;
}

bool View::setUpViewAsInset_(simVis::View* host)
{
  bool ok = true;

  if (host && host->getCamera())
  {
    osg::GraphicsContext* gc = host->getCamera()->getGraphicsContext();
    if (!gc)
    {
      SIM_WARN << LC << "Host has no graphics context, cannot share!" << std::endl;
      ok = false;
    }

    // if the user hasn't created a camera for this view, do so now.
    fovYDeg_ = host->fovY();
    fovXDeg_ = host->fovX();
    osg::Camera* camera = this->getCamera();
    if (!camera)
    {
      camera = new osg::Camera();
      this->setCamera(camera);
    }

    // tell the camera to use the same GC as the "host".
    camera->setGraphicsContext(gc);

    // if the user hasn't set up a viewport already, create one and initialize it
    // to something reasonable (an inset)
    if (!camera->getViewport())
    {
      osg::Viewport* vp = new osg::Viewport(0, 0, 90, 60);
      const osg::Viewport* avp = host->getCamera()->getViewport();
      if (avp)
      {
        vp->setViewport(avp->x(), avp->y(), avp->width()/2, avp->height()/2);
      }
      camera->setViewport(vp);
      camera->setProjectionMatrixAsPerspective(DEFAULT_VFOV, vp->width()/vp->height(), DEFAULT_NEAR, DEFAULT_FAR);
    }

    // save a reference to the host.
    host_ = host;
    viewType_ = VIEW_INSET;

    // set the new view to use currently set nav mode
    setNavigationMode(host_->currentMode_);

    // Share the database pager from the host as well
    setDatabasePager(host->getDatabasePager());

    // install border geometry in the host's HUD camera.
    osg::Camera* bordercamera = this->getOrCreateHUD();
    simVis::FocusManager* focusManager = host->getFocusManager();
    borderNode_ = new BorderNode();
    if (focusManager != NULL)
      focusManager->applyBorderProperties(this);
    bordercamera->addChild(borderNode_.get());

    // Run shader generator to get the border to show up properly
    osgEarth::Registry::shaderGenerator().run(bordercamera);
  }
  else
  {
    SIM_WARN << LC << "setUpViewAsInset_: Host view is not set up or is NULL." << std::endl;
    ok = false;
  }
  return ok;
}

simVis::View* View::getHostView() const
{
  return host_.get();
}

void View::setHostView(simVis::View* host)
{
  host_ = host;
  refreshExtents();
}

std::string View::getUniqueInsetName() const
{
  for (int ii = 1; ii < 100; ++ii)
  {
    std::stringstream name;
    name << "NewInset"  << ii;
    osg::ref_ptr<simVis::View> inset = getInsetByName(name.str());
    if (!inset.valid())
      return name.str();
  }

  // Unlikely there are 99 insets
  assert(false);
  return "Invalid number of Inset Viewports.";
}

namespace
{

bool invalidChar(const char& c)
{
  return !isprint((unsigned)c);
}

}

bool View::isValidNewInsetName(const std::string& newName, const simVis::View* view) const
{
  // Must provide a name
  if (newName.empty())
    return false;

  // Valid characters are printable
  if (std::find_if(newName.begin(), newName.end(), invalidChar) != newName.end())
    return false;

  // Only the main view can be called the MainView
  if (newName == simVis::MAIN_VIEW_NAME)
    return false;

  // No duplicates
  simVis::View* potentialDuplicate = getInsetByName(newName);
  if (potentialDuplicate != NULL)
    return potentialDuplicate == view;  // If both point to the same view then do duplicate

  return true;
}

void View::addInset(simVis::View* inset)
{
  if (inset)
  {
    // make sure it isn't already in the list
    if (std::find(insets_.begin(), insets_.end(), inset) == insets_.end())
    {
      // set up the shared graphics context
      inset->setUpViewAsInset_(this);

      // save it in our list.
      insets_.push_back(inset);

      // initialize the extent
      inset->refreshExtents();

      // ask the view manager to manage this inset.
      if (viewman_.valid())
        viewman_->addView(inset);
    }
  }
}

void View::removeInset(simVis::View* inset)
{
  if (inset)
  {
    InsetViews::iterator i = std::find(insets_.begin(), insets_.end(), inset);
    if (i != insets_.end())
    {
      // a reference will delay destruction until after the event fires.
      insets_.erase(i);

      // ask the view manager to remove this inset.
      if (viewman_.valid())
          viewman_->removeView(inset);
    }
  }
}

unsigned int View::getInsets(View::Insets& output) const
{
  output.clear();
  output.resize(insets_.size());
  std::copy(insets_.begin(), insets_.end(), output.begin());
  return output.size();
}

FocusManager* View::getFocusManager() const
{
  return focusMan_.get();
}

unsigned int View::getNumInsets() const
{
  return static_cast<unsigned>(insets_.size());
}

int View::getIndexOfInset(simVis::View* view) const
{
  for (InsetViews::const_iterator iter = insets_.begin(); iter != insets_.end(); ++iter)
  {
    if (iter->get() == view)
    {
      return iter - insets_.begin();
    }
  }
  return -1;
}

simVis::View* View::getInset(unsigned int index) const
{
  return index < getNumInsets() ? insets_[index].get() : NULL;
}

simVis::View* View::getInsetByName(const std::string& name) const
{
  for (InsetViews::const_iterator i = insets_.begin(); i != insets_.end(); ++i)
  {
    if (i->get()->getName() == name)
      return i->get();
  }
  return NULL;
}

bool View::setExtents(const Extents& e)
{
  if (e.isRatio_)
  {
    simVis::View* host = getHostView();
    if (host)
    {
      const osg::Viewport* rvp = host->getCamera()->getViewport();
      // Note that clamping is not desired here, to avoid pixel/percentage conversion issues
      const double nx = rvp->x() + rvp->width() * e.x_;
      const double ny = rvp->y() + rvp->height() * e.y_;
      const double nw = rvp->width() * e.width_;
      const double nh = rvp->height() * e.height_;

      fixProjectionForNewViewport_(nx, ny, nw, nh);
    }
    else
    {
      // nop. cannot set ratio extents is there's no host view. But it's not an error;
      // it could be that the user simply hasn't added this view to its host yet.
    }
  }
  else
  {
    fixProjectionForNewViewport_(e.x_, e.y_, e.width_, e.height_);
  }

  // save a copy so we can adjust the viewport based on a resize event
  this->extents_ = e;

  // update the HUD
  const osg::Viewport* vp = this->getCamera()->getViewport();
  if (vp)
  {
    getOrCreateHUD()->setViewport(static_cast<int>(vp->x()), static_cast<int>(vp->y()),
      static_cast<int>(vp->width()), static_cast<int>(vp->height()));
    getOrCreateHUD()->setProjectionMatrix(osg::Matrix::ortho2D(0, vp->width()-1, 0, vp->height()-1));
  }
  // if we have a border node, update that too.
  if (borderNode_.valid() && vp)
  {
    static_cast<BorderNode*>(borderNode_.get())->set(vp, borderProps_);
  }

  // if we have inset views, refresh their extents now.
  for (InsetViews::iterator i = insets_.begin(); i != insets_.end(); ++i)
  {
    i->get()->refreshExtents();
  }

  fireCallbacks_(simVis::View::Callback::VIEW_EXTENT_CHANGE);
  return true;
}

bool View::setExtentsAsRatio(float x, float y, float w, float h)
{
  return setExtents(Extents(x, y, w, h, true));
}

void View::refreshExtents()
{
  setExtents(this->extents_);
}

void View::processResize(int width, int height)
{
  // each main view is responsible for resizing its insets in setExtents(), by iterating over and calling refreshExtents()
  if (viewType_ != VIEW_INSET)
  {
    // limit the resize processing to the main view that has same height/width as the event report
    if (this->getCamera())
    {
      const osg::Viewport* vp = this->getCamera()->getViewport();
      if (vp && width == vp->width() && height == vp->height())
      {
        // this is the main view that the resize event was for
        setExtents(Extents(this->extents_.x_, this->extents_.y_, width, height));
      }
    }
  }
  //else TODO: resizing insets (via lasso-like mouse control on the inset) is not yet implemented
}

void View::setViewManager(simVis::ViewManager* viewman)
{
  viewman_ = viewman;
  focusMan_->setViewManager(viewman);
}

View::ViewType View::type() const
{
  return viewType_;
}

void View::setBorderProperties(const BorderProperties& value)
{
  borderProps_ = value;
  const osg::Viewport* vp = this->getCamera()->getViewport();
  if (borderNode_.valid() && vp)
  {
    static_cast<BorderNode*>(borderNode_.get())->set(vp, borderProps_);
  }
}

const View::BorderProperties& View::getBorderProperties() const
{
  return borderProps_;
}

void View::setVisible(bool visible)
{
  if (visible != isVisible())
  {
    getCamera()->setNodeMask(visible ? DISPLAY_MASK_ALL : DISPLAY_MASK_NONE);
    getCamera()->setAllowEventFocus(visible);
    fireCallbacks_(simVis::View::Callback::VIEW_VISIBILITY_CHANGE);
    // Assertion failure means disconnect between logic states and node mask
    assert(visible == isVisible());
  }
}

bool View::isVisible() const
{
  return getCamera()->getNodeMask() != DISPLAY_MASK_NONE;
}

void View::setSceneManager(simVis::SceneManager* node)
{
  // In some invocations of this function, we need to get/set viewpoints
  bool resetViewpoint = false;
  osg::Group* root = osgViewer::View::getSceneData()->asGroup();

  // remove the old one:
  if (sceneData_.valid())
  {
    root->removeChild(sceneData_.get());

    // This covers a special case where we call setSceneManager(getSceneManager()),
    // such as when the sky needs to be reattached or manipulators updated
    resetViewpoint = (node == getSceneManager());
  }

  // install the new one:
  if (node)
    root->addChild(node);

  sceneData_ = node;

  if (node)
  {
    // If checking memory do not load the stars
    if (!simVis::Registry::instance()->isMemoryCheck())
    {
      attachSky_(node);
    }

#if 0
    // NOTE: Changed 5/12/14 to prevent conflict with the auto clipping callback from Scene Manager
    // Then changed on 6/4/14 to prevent conflict with LogarithmicDepthBuffer in ViewManager

    // install an APCH.
    osgEarth::Util::AutoClipPlaneCullCallback* autoClip = new osgEarth::Util::AutoClipPlaneCullCallback(node->getMapNode());
    autoClip->setMaxNearFarRatio(1.5e-5);
    this->getCamera()->addCullCallback(autoClip);
    autoClipCallback_ = autoClip;
    this->getCamera()->setNearFarRatio(1.0e-5); //15 );
#endif
  }

  // reset the earth manip.
  simVis::EarthManipulator* oldManip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (oldManip)
  {
    Viewpoint oldVP = oldManip->getViewpoint();
    osg::ref_ptr<osg::Node> oldTetherNode;
    oldVP.getNode(oldTetherNode);
    oldManip->setTetherCallback(0L);
    simVis::EarthManipulator* newManip = new simVis::EarthManipulator();

    // The following lines will change the manipulator, which resets the viewpoint.  In
    // some cases we want to save the old viewpoint, and restore it afterwards.
    Viewpoint vp = getViewpoint();
    newManip->applySettings(oldManip->getSettings());
    if (oldTetherNode.valid())
    {
      vp.setNode(oldTetherNode.get());
      newManip->setViewpoint(vp);
    }
    newManip->setTetherCallback(tetherCallback_.get());
    newManip->setHeadingLocked(oldManip->isHeadingLocked());
    newManip->setPitchLocked(oldManip->isPitchLocked());
    this->setCameraManipulator(newManip);

    // Restore the viewpoint if needed.  Doing this unconditionally can result in
    // poor display of the initial view (too close to earth).
    if (resetViewpoint)
      setViewpoint(vp);
  }
}

void View::setLighting(bool value)
{
  if (value)
  {
    // Note that when on, lighting should be set to inherit so that items higher
    // in the scene graph can impact our lighting values
    simVis::setLightingToInherit(this->getCamera()->getStateSet());
  }
  else
  {
    // Lighting should be off, but not override-off (which would impact children negatively, including terrain lighting)
    simVis::setLighting(this->getCamera()->getOrCreateStateSet(), osg::StateAttribute::OFF);
  }
  lighting_ = value;
}

double View::fovX() const
{
  return fovXDeg_;
}

void View::setFovX(double fovXDeg)
{
  // do a simple check on invalid values, since EarthManipulator doesn't protect against invalid values
  if (fovXDeg <= 0.0 || fovXDeg >= 360.0)
    return;

  if (fovXDeg == fovXDeg_)
    return;
  fovXDeg_ = fovXDeg;
  refreshExtents();
}

double View::fovY() const
{
  return fovYDeg_;
}

void View::setFovY(double fovYDeg)
{
  // do a simple check on invalid values, since EarthManipulator doesn't protect against invalid values
  if (fovYDeg <= 0.0 || fovYDeg >= 360.0)
    return;

  // always update the earth manipulator first
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
    manip->setFovY(fovYDeg);

  if (fovYDeg == fovYDeg_)
    return;
  fovYDeg_ = fovYDeg;
  refreshExtents();
}

osg::Node* View::getVisibleSceneData() const
{
  return sceneData_.get();
}

osg::Camera* View::getOrCreateHUD()
{
  if (!hud_.valid())
  {
    hud_ = createHUD_();
    osgViewer::View::getSceneData()->asGroup()->addChild(hud_.get());
  }

  return hud_.get();
}

void View::setAllowLabelOverlap(bool value)
{
  controlCanvas_->setAllowControlNodeOverlap(value);
}

void View::tetherCamera(osg::Node* node)
{
  tetherCamera(node, getViewpoint(), 0.0);
}

void View::tetherCamera(osg::Node *node, const simVis::Viewpoint& vp, double durationSeconds)
{
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
  {
    simVis::Viewpoint newVp(vp);
    osg::Node* realTether = getModelNodeForTether(node);
    fixCockpitFlag_(realTether, manip);
    newVp.setNode(realTether);

    // Set the focal point if needed (i.e. if there is no tether node)
    if (realTether == NULL && vp.nodeIsSet())
    {
      osg::ref_ptr<osg::Node> oldTether;
      vp.getNode(oldTether);
      simCore::Vec3 lla = simVis::computeNodeGeodeticPosition(oldTether.get());
      newVp.focalPoint()->set(osgEarth::SpatialReference::create("wgs84"),
        osg::Vec3d(lla.lon() * simCore::RAD2DEG, lla.lat() * simCore::RAD2DEG, lla.alt()),
        osgEarth::ALTMODE_ABSOLUTE);
    }

    // Pass it to the setViewpoint() method
    setViewpoint(newVp, durationSeconds);
  }
}

osg::Node* View::getCameraTether() const
{
  osg::Node* result = NULL;
  const simVis::EarthManipulator* manip = dynamic_cast<const simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
  {
    Viewpoint vp = manip->getViewpoint();
    osg::ref_ptr<osg::Node> node;
    if (vp.getNode(node))
      result = node.release();
  }
  return result;
}

NavMode View::getNavigationMode() const
{
  return currentMode_;
}

void View::setFocalOffsets(double heading_deg, double pitch_deg, double range, double transition_s)
{
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
  {
    Viewpoint vp;
    vp.heading()->set(heading_deg, Units::DEGREES);
    if (pitch_deg > MAX_ELEVATION_DEGREES)
      pitch_deg = MAX_ELEVATION_DEGREES;
    else if (pitch_deg < -MAX_ELEVATION_DEGREES)
      pitch_deg = -MAX_ELEVATION_DEGREES;
    vp.pitch()->set(pitch_deg, Units::DEGREES);
    vp.range()->set(range, Units::METERS);
    manip->setViewpoint(vp, transition_s);
  }
}

void View::lookAt(double lat_deg, double lon_deg, double alt_m, double heading_deg, double pitch_deg, double range, double transition_s)
{
  simVis::Viewpoint vp;
  vp.name() = "lookat";
  vp.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::create("wgs84"), lon_deg, lat_deg, alt_m);
  vp.heading()->set(heading_deg, Units::DEGREES);
  if (pitch_deg > MAX_ELEVATION_DEGREES)
    pitch_deg = MAX_ELEVATION_DEGREES;
  else if (pitch_deg < -MAX_ELEVATION_DEGREES)
    pitch_deg = -MAX_ELEVATION_DEGREES;
  vp.pitch()->set(pitch_deg, Units::DEGREES);
  vp.range()->set(range, Units::METERS);
  // Clear the viewpoint's position offsets for look-at's
  vp.positionOffset() = osg::Vec3();
  setViewpoint(vp, transition_s);
}

void View::addOverlayControl(osgEarth::Util::Controls::Control* control)
{
  // There is no reason to store the same control more than once
  if (!controlCanvas_->containsNode(control))
    controlCanvas_->addControl(control);
}

void View::removeOverlayControl(osgEarth::Util::Controls::Control* control)
{
  controlCanvas_->removeControl(control);
}

bool View::addSceneControl(osgEarth::Util::Controls::Control* control, const osgEarth::GeoPoint& location, float priority)
{
  if (!sceneData_.valid() || !sceneData_->getMap())
    return false;

  osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform();
  xform->addChild(new osgEarth::Util::Controls::ControlNode(control, priority));

  osg::Matrixd placer;
  if (location.createLocalToWorld(placer))
  {
    xform->setMatrix(placer);
    sceneControls_->addChild(xform.get());
    sceneControlsLUT_[control] = xform.get();
    return true;
  }
  else
  {
    return false;
  }
}

bool View::removeSceneControl(osgEarth::Util::Controls::Control* control)
{
  std::map<osgEarth::Util::Controls::Control*, osg::Node*>::iterator i = sceneControlsLUT_.find(control);
  if (i != sceneControlsLUT_.end())
  {
    sceneControls_->removeChild(i->second);
    sceneControlsLUT_.erase(i);
  }
  return true;
}

bool View::moveSceneControl(osgEarth::Util::Controls::Control* control, const osgEarth::GeoPoint& location)
{
  if (!sceneData_.valid() || !sceneData_->getMap())
    return false;

  osg::Matrixd placer;
  if (location.createLocalToWorld(placer))
  {
    std::map<osgEarth::Util::Controls::Control*, osg::Node*>::iterator i = sceneControlsLUT_.find(control);
    if (i != sceneControlsLUT_.end())
    {
      osg::MatrixTransform* xform = dynamic_cast<osg::MatrixTransform*>(i->second);
      if (xform)
      {
        xform->setMatrix(placer);
      }
    }
  }
  return true;
}

simVis::Viewpoint View::getViewpoint() const
{
  const simVis::EarthManipulator* manip = dynamic_cast<const simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
  {
    // If we are in watch mode, we've taken over the manipulator settings and they're
    // not going to make sense to the caller.  Create a reasonable return for caller.
    simVis::Viewpoint manipViewpoint = manip->getViewpoint();
    if (isWatchEnabled())
    {
      simVis::Viewpoint vp = watchViewpoint_;
      // Make sure the returned viewpoint has at least a focal point OR a tether node
      if (manipViewpoint.focalPoint().isSet())
        vp.focalPoint() = manipViewpoint.focalPoint();
      else if (manipViewpoint.nodeIsSet())
      {
        osg::ref_ptr<osg::Node> tether;
        manipViewpoint.getNode(tether);
        vp.setNode(tether.get());
      }
      else
      {
        // Not centered and no tether.  Make something up to avoid errors
        assert(0);
        vp.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::create("wgs84"), 0, 0);
      }
      return vp;
    }
    return manipViewpoint;
  }
  return simVis::Viewpoint();
}

void View::setViewpoint(const simVis::Viewpoint& vp, double transitionTime_s)
{
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
  {
    // If in watch mode, record the setViewpoint() as an update from the user
    if (isWatchEnabled())
    {
      // Ignore updates to node / focal point; only respect changes to RAE, pos offsets, and name
      if (vp.positionOffset().isSet())
        watchViewpoint_.positionOffset() = vp.positionOffset();
      if (vp.heading().isSet())
        watchViewpoint_.heading() = vp.heading();
      if (vp.pitch().isSet())
        watchViewpoint_.pitch() = vp.pitch();
      if (vp.range().isSet())
        watchViewpoint_.range() = vp.range();
      if (vp.name().isSet())
        watchViewpoint_.name() = vp.name();
    }
    manip->setViewpoint(vp, transitionTime_s);
  }
}

void View::setNavigationMode(const NavMode& mode)
{
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (!manip)
    return;

  // Retain some settings across mouse modes
  const bool arcTransitions = manip->getSettings()->getArcViewpointTransitions();
  const bool terrainAvoidance = manip->getSettings()->getTerrainAvoidanceEnabled();

  if (mode == NAVMODE_ROTATEPAN)
    manip->applySettings(new RotatePanNavigationMode(this, overheadEnabled_, watchEnabled_));
  else if (mode == NAVMODE_GLOBESPIN)
    manip->applySettings(new GlobeSpinNavigationMode(overheadEnabled_, watchEnabled_));
  else if (mode == NAVMODE_ZOOM)
    manip->applySettings(new ZoomNavigationMode(overheadEnabled_, watchEnabled_));
  else if (mode == NAVMODE_CENTERVIEW)
    manip->applySettings(new CenterViewNavigationMode(overheadEnabled_, watchEnabled_));
  else if (mode == NAVMODE_GIS)
    manip->applySettings(new GisNavigationMode(this, overheadEnabled_, watchEnabled_));

  // Restore the retained settings
  manip->getSettings()->setArcViewpointTransitions(arcTransitions);
  // Restore the collision avoidance flag
  manip->getSettings()->setTerrainAvoidanceEnabled(terrainAvoidance);
  // set minimum camera to focal point distance
  manip->getSettings()->setMinMaxDistance(MINIMUM_FOCAL_POINT_DISTANCE, manip->getSettings()->getMaxDistance());

  currentMode_ = mode;
}

void View::enableOverheadMode(bool enableOverhead)
{
  if (enableOverhead == overheadEnabled_)
    return;

  // need to verify that the earth manipulator has the correct fov,
  // which may not be initialized properly if overhead mode is set too soon
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
    manip->setFovY(fovYDeg_);

  // if this is the first time enabling overhead mode, install the node camera-update
  // node visitor in the earth manipulator to facilitate tethering. This NodeVisitor
  // does not actually do anything except convey the "overhead mode enabled" flag
  // to the LocatorNode::computeLocalToWorldMatrix() method.
  if (updateCameraNodeVisitor_.valid() == false)
  {
    updateCameraNodeVisitor_ = new osg::NodeVisitor();
#if SDK_OSGEARTH_VERSION_GREATER_THAN(1,7,0)
    manip->setUpdateCameraNodeVisitor(updateCameraNodeVisitor_.get());
#endif
  }

  osg::StateSet* cameraState = getCamera()->getOrCreateStateSet();
  if (enableOverhead)
  {
    // Disable watch mode if needed
    if (isWatchEnabled())
    {
      enableWatchMode(NULL, NULL);
    }
    // always have north up in overhead mode
    simVis::Viewpoint vp = getViewpoint();
    vp.heading()->set(0.0, Units::DEGREES);
    vp.pitch()->set(-90.0, Units::DEGREES);
    this->setViewpoint(vp);

    // Set an orthographic camera. We don't call enableOrthographic() here
    // because we'd rather quitely reset the original mode once overhead mode
    // is disabled later.
    if (orthoEnabled_ == false)
    {
#if SDK_OSGEARTH_VERSION_GREATER_THAN(1,6,0)
      // Only go into orthographic past 1.6 -- before then, the LDB would cause significant issues with platform and GOG display
      getCamera()->setProjectionMatrixAsOrtho(-1.0, 1.0, -1.0, 1.0, -5e6, 5e6);
      getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
      if (overheadNearFarCallback_->referenceCount() == 1)
        getCamera()->addCullCallback(overheadNearFarCallback_);
#endif
    }

    // disable elevation rendering on the terrain surface
    cameraState->setDefine("OE_TERRAIN_RENDER_ELEVATION", osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
  }
  else
  {
    // quitely revert to the perspective camera if necessary
#if SDK_OSGEARTH_VERSION_GREATER_THAN(1,6,0)
    if (orthoEnabled_ == false)
    {
      const osg::Viewport* vp = getCamera()->getViewport();
      const double aspectRatio = vp ? vp->aspectRatio() : 1.5;

      if (!fovXEnabled_)
      {
        getCamera()->setProjectionMatrixAsPerspective(fovY(), aspectRatio, 1.0, 100.0);
      }
      else
      {
        double left = 0.0;
        double right = 0.0;
        double bottom = 0.0;
        double top = 0.0;
        getFrustumBounds_(left, right, bottom, top, 1.0);
        getCamera()->setProjectionMatrix(osg::Matrixd::frustum(left, right, bottom, top, 1.0, 100.0));
      }

      getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
      getCamera()->removeCullCallback(overheadNearFarCallback_);
    }
#endif

    // remove elevation rendering override.
    cameraState->removeDefine("OE_TERRAIN_RENDER_ELEVATION");
  }

  // Toggle the overhead clamping features on/off
  OverheadMode::setEnabled(enableOverhead && useOverheadClamping(), this);

  overheadEnabled_ = enableOverhead;

  // Turn on near frustum culling for normal mode, and off for overhead mode.
  // Note that this does come with a slight performance hit, but solves the problem
  // where entities outside the frustum SHOULD be drawn but are not in overhead.
  if (!overheadEnabled_)
    getCamera()->setCullingMode(getCamera()->getCullingMode() | osg::CullSettings::NEAR_PLANE_CULLING);
  else
    getCamera()->setCullingMode(getCamera()->getCullingMode() & (~osg::CullSettings::NEAR_PLANE_CULLING));

  // Fix navigation mode
  setNavigationMode(currentMode_);

  if (getHostView())
  {
    // For insets in the main view
    getHostView()->getFocusManager()->reFocus();
  }
  else
  {
    // For the main view
    getFocusManager()->reFocus();
  }

  // Update the EarthManipulator's camera update node visitor with the new state.
  simVis::OverheadMode::prepareVisitor(this, updateCameraNodeVisitor_.get());
}

bool View::isOverheadEnabled() const
{
  return overheadEnabled_;
}

void View::enableCockpitMode(osg::Node* tether)
{
  // cockpit mode requires a tether
  if (tether)
  {
    bool changed = (cockpitEnabled_ == false);
    cockpitEnabled_ = true;
    tetherCamera(tether);
    simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
    // Force the heading/pitch lock on
    if (manip)
    {
      manip->setHeadingLocked(true);
      manip->setPitchLocked(true);
    }
    if (changed)
      fireCallbacks_(Callback::VIEW_COCKPIT_CHANGE);
  }
  else if (cockpitEnabled_)
  {
    simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
    if (manip)
      manip->getSettings()->setTetherMode(osgEarth::Util::EarthManipulator::TETHER_CENTER);
    if (cockpitEnabled_)
    {
      cockpitEnabled_ = false;
      // Disable the lock on heading/pitch too
      if (manip)
      {
        manip->setHeadingLocked(false);
        manip->setPitchLocked(false);
      }
      fireCallbacks_(Callback::VIEW_COCKPIT_CHANGE);
    }
  }
}

bool View::isCockpitEnabled() const
{
  return cockpitEnabled_;
}

void View::enableWatchMode(osg::Node* watched, osg::Node* watcher)
{
  if (watched && watcher)
  {
    // Get a simVis::EntityNode out of the passed in Node
    simVis::EntityNode* watcherNode = getEntityNode(watcher);

    // Can only continue if watcherNode (an EntityNode) is valid
    if (watcherNode)
    {
      watcherNode_ = watcherNode;

      // Convert into an EntityNode.  Need EntityNode on both sides (watched and watcher)
      simVis::EntityNode* watchedEntityNode = getEntityNode(watched);
      watchedNode_ = watchedEntityNode;
      if (watchedNode_.valid())
      {
        // Disable overhead mode if we're in overhead mode
        if (isOverheadEnabled())
          enableOverheadMode(false);

        // Set the viewpoint so that we're not tethered
        simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
        if (manip && manip->isTethering())
        {
          osg::ref_ptr<osg::Node> tetherNode;
          manip->getViewpoint().getNode(tetherNode);
          simVis::Viewpoint untether;
          untether.setNode(NULL);
          // Set a focal point to force a clear-out of the node; this will get updated to a better place in updateWatchView_()
          simCore::Vec3 lla = simVis::computeNodeGeodeticPosition(tetherNode.get());
          untether.focalPoint()->set(osgEarth::SpatialReference::create("wgs84"),
            osg::Vec3d(lla.lon() * simCore::RAD2DEG, lla.lat() * simCore::RAD2DEG, lla.alt()),
            osgEarth::ALTMODE_ABSOLUTE);
          setViewpoint(untether, 0.0);
        }

        // Update the watch viewpoint (user values) based on current viewpoint
        watchViewpoint_ = getViewpoint();
        watchEnabled_ = true;
        updateWatchView_();

        // add event handler to refresh the watch view every frame
        static_cast<UpdateWatchView*>(updateWatchViewHandler_.get())->setActive(true);
        setNavigationMode(currentMode_);

        // In watch mode, turn off the manipulation of heading/pitch
        if (manip)
        {
          manip->setHeadingLocked(true);
          manip->setPitchLocked(true);
        }

        // Assert various post-conditions of enabling watch mode.  Failing any of
        // these assertions will jump us out of watch mode immediately on next frame.
        assert(isWatchEnabled());
        assert(manip != NULL);
        assert(watcherNode_.valid());
        assert(watchedNode_.valid());
        assert(!manip->isTethering());
        return;
      }
    }
  }
  else if (watcherNode_ == NULL && !watchEnabled_)
    return;

  // Reset the eye azim/elev/range to what it was before we started monkeying with it.  In
  // watch mode, we would have changed the heading/pitch/range drastically to get the view right
  simVis::Viewpoint resetVp;
  resetVp.heading() = watchViewpoint_.heading();
  resetVp.pitch() = watchViewpoint_.pitch();
  resetVp.range() = watchViewpoint_.range();
  resetVp.setNode(getModelNodeForTether(watcherNode_.get()));

  // Clear out watch values so that our observer doesn't pick up anything
  watcherNode_ = NULL;
  watchedNode_ = NULL;
  watchEnabled_ = false;

  // Swap the view back to what we had before, tethered to the watcher.
  setViewpoint(resetVp);

  // Turn heading/pitch manipulation back on unconditionally
  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  if (manip)
  {
    manip->setHeadingLocked(false);
    manip->setPitchLocked(false);
  }

  setNavigationMode(currentMode_);
  static_cast<UpdateWatchView*>(updateWatchViewHandler_.get())->setActive(false);
}

bool View::isWatchEnabled() const
{
  return watchEnabled_;
}

simVis::EntityNode* View::getWatcherNode() const
{
  if (isWatchEnabled())
    return watcherNode_.get();
  return NULL;
}

simVis::EntityNode* View::getWatchedNode() const
{
  if (isWatchEnabled())
    return watchedNode_.get();
  return NULL;
}

void View::updateWatchView_()
{
  // Make sure we break out early if watch is not enabled
  if (!isWatchEnabled())
    return;

  simVis::EarthManipulator* manip = dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
  // Jump out of watch mode if we're tethering, or if one of the watch nodes is invalid
  if (!manip || !watcherNode_.valid() || !watchedNode_.valid() || manip->isTethering())
  {
    enableWatchMode(NULL, NULL);
    return;
  }

  // Convert into an EntityNode.  Need EntityNode on both sides (watched and watcher)
  simVis::EntityNode* watchedEntityNode = getWatchedNode();
  if (!watchedEntityNode)
  {
    enableWatchMode(NULL, NULL);
    return;
  }

  // use focal offsets to set camera displacement from tether/watched entity to watcher entity, using calculated az, el, range from watcher to watched
  simCore::Vec3 watchedLla;
  simVis::Locator* watchedLocator = watchedEntityNode->getLocator();
  watchedLocator->getLocatorPosition(&watchedLla, simCore::COORD_SYS_LLA);

  simCore::Vec3 watcherLla;
  simVis::Locator* watcherLocator = watcherNode_->getLocator();
  watcherLocator->getLocatorPosition(&watcherLla, simCore::COORD_SYS_LLA);

  // The point of watch mode is to position the eye's view a given distance from the watcher, then orient
  // the camera such that it keeps the watched entity in the middle of the screen.  Because EarthManipulator
  // does not permit independent rotation of the camera post-transformation, we provide a work-around.  The
  // camera position is first calculated using the watcher LLA position, the offset, and the focal offsets.
  // Next, the angle from that location to the watched LLA is calculated, and the camera is oriented in that
  // vector.  The range is set to 0.

  // In a tangent plane system, adjust for the viewpoint RAE and the position offset
  simCore::Vec3 eyeNed;
  // Only need to calculate angles if the range is non-zero
  if (watchViewpoint_.range().isSet() && watchViewpoint_.range()->as(osgEarth::Units::METERS) != 0.0)
  {
    // create DCM based on specified orientation (NED frame)
    double dcm[3][3];
    // Invert the pitch (e.g. -75 becomes +75) and reverse the heading (e.g. +45 becomes +215)
    simCore::d3EulertoDCM(simCore::Vec3(watchViewpoint_.heading()->as(osgEarth::Units::RADIANS) + M_PI,
      -watchViewpoint_.pitch()->as(osgEarth::Units::RADIANS), 0), dcm);
    // create vector along body axis (NED frame) in the length of viewpoint, then calculate XYZ
    simCore::d3MTv3Mult(dcm, simCore::Vec3(watchViewpoint_.range()->as(osgEarth::Units::METERS), 0, 0), eyeNed);
  }
  // Only need to calculate position offset if it's set
  if (watchViewpoint_.positionOffset().isSet())
  {
    // XYZ is ENU -- swap to NED
    eyeNed.setX(eyeNed.x() + watchViewpoint_.positionOffset()->y());
    eyeNed.setY(eyeNed.y() + watchViewpoint_.positionOffset()->x());
    eyeNed.setZ(eyeNed.z() - watchViewpoint_.positionOffset()->z());
  }
  // At this point, if the NED eye is non-zero, we need to calculate the LLA of the eye position
  simCore::Vec3 realEyeLla = watcherLla;
  if (eyeNed.x() != 0 || eyeNed.y() != 0 || eyeNed.z() != 0)
  {
    // Create a coordinate converter centered on the watcher entity
    simCore::CoordinateConverter cc;
    cc.setReferenceOrigin(watcherLla);
    simCore::Coordinate offsetCoord(simCore::COORD_SYS_NED, eyeNed);
    // Convert to LLA and replace the watcherLla value
    simCore::Coordinate outLla;
    cc.convert(offsetCoord, outLla, simCore::COORD_SYS_LLA);
    realEyeLla = outLla.position();
  }

  // Create a new viewpoint on top of the eye position
  simVis::Viewpoint updatedViewpoint;
  updatedViewpoint.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::create("WGS84"),
    realEyeLla.lon() * simCore::RAD2DEG,
    realEyeLla.lat() * simCore::RAD2DEG,
    realEyeLla.alt());
  updatedViewpoint.positionOffset() = osg::Vec3();
  updatedViewpoint.range()->set(0.0, osgEarth::Units::METERS);

  // Now that we know where the eye is, calculate the orientation to the watched node
  double azR;
  double elR;
  simCore::calculateAbsAzEl(realEyeLla, watchedLla, &azR, &elR, NULL, simCore::WGS_84, NULL);
  updatedViewpoint.heading()->set(azR, osgEarth::Units::RADIANS);
  updatedViewpoint.pitch()->set(elR, osgEarth::Units::RADIANS);

  // Finally, pass this into the manipulator
  manip->setViewpoint(updatedViewpoint, 0.0);
}

void View::enableOrthographic(bool whether)
{
  if (orthoEnabled_ == whether)
    return;

  if (whether)
  {
    // Switch to an Ortho camera. The actual values here don't matter because the EarthManipulator
    // will take control of them in order to track the last-known YFOV.
    getCamera()->setProjectionMatrixAsOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  }
  else
  {
    // Set up the perspective camera. Near/Far don't matter since OSG automatically calculates them.
    const osg::Viewport* vp = getCamera()->getViewport();
    double aspectRatio = vp ? vp->aspectRatio() : 1.5;
    getCamera()->setProjectionMatrixAsPerspective(fovY(), aspectRatio, 1.0, 100.0);
  }

  orthoEnabled_ = whether;
  fireCallbacks_(simVis::View::Callback::VIEW_ORTHO_CHANGE);
}

bool View::isOrthographicEnabled() const
{
  return orthoEnabled_;
}

void View::attachSky_(simVis::SceneManager* sceneMgr)
{
  // add a sky node if we need one:
  osgEarth::Util::SkyNode* sky = sceneMgr->getSkyNode();
  if (sky)
    sky->attach(this);
}

void View::installDebugHandlers()
{
  installBasicDebugHandlers();

  // Allows toggling between full screen and windowed mode ('f')
  addEventHandler(new osgViewer::WindowSizeHandler());
}

void View::installBasicDebugHandlers()
{
  // Allows toggling through statistics pages ('s')
  osgViewer::StatsHandler* stats = new osgViewer::StatsHandler();
  stats->getCamera()->setAllowEventFocus(false);
  simVis::fixStatsHandlerGl2BlockyText(stats);
  addEventHandler(stats);

  // Allows cycling of polygon mode, textures, lighting back face enabling
  addEventHandler(new osgGA::StateSetManipulator(getCamera()->getOrCreateStateSet()));
}

osg::Camera* View::createHUD_() const
{
  const osg::Viewport* vp = this->getCamera()->getViewport();
  osg::Camera* hud = new osg::Camera();
  // Be sure to render after the controls widgets.
  // "10" is arbitrary, so there's room between the two (default Control Canvas value is 25000)
  hud->setRenderOrder(osg::Camera::POST_RENDER, controlCanvas_->getRenderOrderNum() + 10);
  hud->setViewport(osg::clone(vp, osg::CopyOp::DEEP_COPY_ALL));
  hud->setProjectionMatrix(osg::Matrix::ortho2D(0, vp->width()-1, 0, vp->height()-1));
  hud->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  hud->setViewMatrix(osg::Matrix::identity());
  hud->setClearMask(GL_DEPTH_BUFFER_BIT);
  hud->setAllowEventFocus(true);
  hud->getOrCreateStateSet()->setRenderBinDetails(0, BIN_TRAVERSAL_ORDER_SIMSDK);
#if OSG_VERSION_LESS_OR_EQUAL(3,4,1)
  // Set up a program so that text is not blocky for older OSG that didn't bake in programs
  hud->getOrCreateStateSet()->setAttributeAndModes(new osg::Program(), 0);
#endif
  hud->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
  return hud;
}

void simVis::View::setName(const std::string& name)
{
  if (name != getName())
  {
    osgViewer::View::setName(name);
    fireCallbacks_(simVis::View::Callback::VIEW_NAME_CHANGE);
  }
}

void simVis::View::addCallback(simVis::View::Callback* callback)
{
  if (callback)
    callbacks_.push_back(callback);
}

void simVis::View::removeCallback(simVis::View::Callback* callback)
{
  if (callback)
  {
    Callbacks::iterator i = std::find(callbacks_.begin(), callbacks_.end(), callback);
    if (i != callbacks_.end())
      callbacks_.erase(i);
  }
}

void simVis::View::fireCallbacks_(const simVis::View::Callback::EventType& e)
{
  for (Callbacks::const_iterator i = callbacks_.begin(); i != callbacks_.end(); ++i)
    (*i)->operator()(this, e);
}

void simVis::View::fixProjectionForNewViewport_(double nx, double ny, double nw, double nh)
{
  // Avoid divide-by-0
  osg::Camera* camera = getCamera();
  if (nh == 0.0 || nw == 0.0 || camera == NULL)
    return;

  // Apply the new viewport:
  osg::ref_ptr<osg::Viewport> newViewport = new osg::Viewport(nx, ny, nw, nh);
  camera->setViewport(newViewport.get());

  // Apply the new projection matrix:
  const osg::Matrix& proj = camera->getProjectionMatrix();

  if (osg::equivalent(proj(3,3), 0.0)) // perspective
  {
    double oldFovY = DEFAULT_VFOV;
    double oldAspectRatio = 1;
    double oldNear = DEFAULT_NEAR;
    double oldFar = DEFAULT_FAR;

    // Pull out the old values from the projection matrix
    proj.getPerspective(oldFovY, oldAspectRatio, oldNear, oldFar);
    if (!fovXEnabled_)
    {
      camera->setProjectionMatrixAsPerspective(fovY(), newViewport->aspectRatio(), oldNear, oldFar);
    }
    else
    {
      double left = 0.0;
      double right = 0.0;
      double bottom = 0.0;
      double top = 0.0;
      getFrustumBounds_(left, right, bottom, top, oldNear);
      camera->setProjectionMatrix(osg::Matrixd::frustum(left, right, bottom, top, oldNear, oldFar));
    }
  }
  else
  {
      // In orthographic, do nothing since the EarthManipulator will automatically
      // be tracking the last perspective FovY.
  }
}

void View::fixCockpitFlag_(osg::Node* node, osgEarth::Util::EarthManipulator* manip) const
{
  if (!manip)
    return;

  if (node && cockpitEnabled_)
    manip->getSettings()->setTetherMode(osgEarth::Util::EarthManipulator::TETHER_CENTER_AND_ROTATION);
  else
    manip->getSettings()->setTetherMode(osgEarth::Util::EarthManipulator::TETHER_CENTER);
}

osg::Node* View::getModelNodeForTether(osg::Node* node) const
{
  EntityNode* entityNode = dynamic_cast<EntityNode*>(node);
  if (entityNode)
  {
    // Entity nodes typically have proxies (children) that we center on.
    osg::Node* proxyNode = entityNode->findAttachment<PlatformModelNode>();
    // Fall back to Gate centroids
    if (!proxyNode)
      proxyNode = entityNode->findAttachment<GateCentroid>();

    if ((!proxyNode) && (entityNode->type() == simData::CUSTOM_RENDERING))
    {
      auto customNode = static_cast<CustomRenderingNode*>(entityNode);
      proxyNode = customNode->locatorNode();
    }

    if (proxyNode)
      node = proxyNode;
  }
  return node;
}

simVis::EntityNode* View::getEntityNode(osg::Node* node) const
{
  // Get a simVis::EntityNode out of the passed in Node
  simVis::EntityNode* watcherNode = dynamic_cast<simVis::EntityNode*>(node);
  if (watcherNode)
    return watcherNode;

  // Maybe it's really a Platform Model or Centroid node, which is the child of an EntityNode
  if (node)
  {
    //TESTING: When watching from a centroid, the parent is a simVis::CentroidManager, not an EntityNode
    simVis::EntityNode* entityNode = dynamic_cast<simVis::EntityNode*>(node->getParent(0));
    // If assert triggers, there's some weird unexpected hierarchy; investigate and resolve weirdness
    assert(entityNode != NULL);
    return entityNode;
  }
  return NULL;
}

simVis::EarthManipulator* View::getEarthManipulator()
{
  return dynamic_cast<simVis::EarthManipulator*>(getCameraManipulator());
}

const simVis::EarthManipulator* View::getEarthManipulator() const
{
  return dynamic_cast<const simVis::EarthManipulator*>(getCameraManipulator());
}

void View::applyManipulatorSettings(const simVis::View& copyFrom)
{
  osg::ref_ptr<simVis::EarthManipulator> insetManip = getEarthManipulator();
  osg::ref_ptr<const simVis::EarthManipulator> hostManip = copyFrom.getEarthManipulator();
  if (insetManip.valid() && hostManip.valid())
    insetManip->applySettings(hostManip->getSettings());
}

bool View::useOverheadClamping() const
{
  return useOverheadClamping_;
}

void View::setUseOverheadClamping(bool clamp)
{
  if (clamp == useOverheadClamping_)
    return;
  useOverheadClamping_ = clamp;
  OverheadMode::setEnabled(isOverheadEnabled() && useOverheadClamping(), this);
}

void View::setFovXEnabled(bool fovXEnabled)
{
  if (fovXEnabled_ == fovXEnabled)
    return;
  fovXEnabled_ = fovXEnabled;
  refreshExtents();
}

bool View::isFovXEnabled() const
{
  return fovXEnabled_;
}

void View::getFrustumBounds_(double& left, double& right, double& bottom, double& top, double zNear) const
{
  double tanFovX = tan(simCore::DEG2RAD * fovX() * 0.5);
  double tanFovY = tan(simCore::DEG2RAD * fovY() * 0.5);

  right = tanFovX * zNear;
  left = -right;
  top = tanFovY * zNear;
  bottom = -top;
}

}
