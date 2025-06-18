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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 */
#ifndef SIMVIS_VIEW_MANAGER_H
#define SIMVIS_VIEW_MANAGER_H 1

#include <functional>
#include <optional>
#include <vector>
#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "osg/ArgumentParser"
#include "osgViewer/CompositeViewer"
#include "simCore/Common/Common.h"

namespace osg { class GraphicsContext; }

namespace simVis
{

class View;

/**
 * Manages one or more Views that all share and render a single SceneManager.
 *
 * There are two kinds of views: top-level and inset. The ViewManager tracks
 * them both. A top-level view takes up the entire window in which it's embedded,
 * and you typically create one by creating a new View object and calling
 * addView() directly.
 *
 * Each top-level view can then contain:
 *
 *   (a) inset views, which are small overlaid viewports within the top-level
 *       view's viewport boundaries; and
 *
 *   (b) a HUD stack, which is an ordered set of overlays rendered atop all
 *       insets that's typically used for text or other 2D graphics.
 *
 * Inset views are created by calling View::addInset(), which in turn will
 * automatically add the inset to this ViewManager for you (no need to do
 * so manually).
 *
 * An inset view:
 *
 *   (a) Cannot contain inset views; and
 *
 *   (b) Has no HUD stack.
 *
 * If you are using multiple osg::GraphicsContext instances with the same scene
 * and/or ViewManager, you may need to disable the image unref after apply, or you
 * may see texture glitches in the multiple graphics windows.  For example:
 *
 * <code>
 * // Prevent image data from being deleted on CPU after it's been sent to GPU, called at app startup:
 * osgEarth::Registry::instance()->unRefImageDataAfterApply() = false;
 * // Prevent image data from being deleted on CPU after sent to GPU, called once on the pager:
 * view->getScene()->getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, false);
 * </code>
 *
 * Note that although ViewManager can support multiple GraphicsContexts, underlying
 * osgQOpenGL code (referenced in simQt::ViewerWidgetAdapter) does not gracefully handle
 * this situation, and you will need one ViewManager per ViewerWidgetAdapter.
 */
class SDKVIS_EXPORT ViewManager : public osg::Referenced
{
public:
  /** Interface for activity callbacks. */
  class Callback : public osg::Referenced
  {
  public:
    /// Events that this callback processes
    enum EventType
    {
      /** Application added a View */
      VIEW_ADDED,

      /** Application removed a View */
      VIEW_REMOVED
    };

    /// Provide this method to receive an event
    virtual void operator()(simVis::View* inset, const EventType& e) = 0;

  protected:
    /// osg::Referenced-derived
    virtual ~Callback() {}
  };

  /** Lambda callback; particularly useful when you register and don't need to unregister. */
  class LambdaCallback : public Callback
  {
  public:
    /**
     * Instantiate with a lambda, e.g.:
     * <code>
     * new simVis::ViewManager::LambdaCallback([this](simVis::View* view, simVis::ViewManager::Callback::EventType evtType) {
     *   std::cout << "Callback issued.\n"
     * }));
     * </code>
     */
    explicit LambdaCallback(const std::function<void(simVis::View*, Callback::EventType)>& func) : func_(func) { }
    virtual void operator()(simVis::View* inset, const EventType& e) override { func_(inset, e); }

  protected:
    /// osg::Referenced-derived
    virtual ~LambdaCallback() {}

  private:
    std::function<void(simVis::View*, Callback::EventType)> func_;
  };

  class PostCameraEventHandler : public osg::Referenced
  {
  public:
    /// Provide this method to receive an event
    virtual void operator()() = 0;

  protected:
    /// osg::Referenced-derived
    virtual ~PostCameraEventHandler() {}
  };

  /** Constructs a new view manager. */
  ViewManager();

  /**
   * Constructs a new view manager, parsing the default OSG command-line
   * arguments.
   */
  ViewManager(osg::ArgumentParser& args);

  /** Adds a view. */
  void addView(simVis::View* view);

  /** Removes a view. */
  void removeView(simVis::View* view);

  /** Resizes all of the managed views */
  void handleResize(const osg::GraphicsContext* gc, int newwidth, int newheight);

  /** Gets a list of the managed views. Includes top level and inset views. */
  void getViews(std::vector<simVis::View*>& out_views) const;

  /** The total number of views (top level, inset views, and super hud) */
  unsigned int getNumViews() const;

  /** The View at index N */
  simVis::View* getView(unsigned int index) const;

  /** Retrieves the first view matching the name provided. */
  simVis::View* getViewByName(const std::string& name) const;

  /** Retrieves the topmost interactive view (not hidden, allowing event focus) at the mouse XY */
  simVis::View* getViewByMouseXy(const osg::Vec2d& mouseXy) const;

  /** Retrieves the index of the view provided, or -1 if not found. */
  int getIndexOf(simVis::View* view) const;

  /**
  * Install a callback that will be notified when views are added and removed.
  * @param[in ] callback Callback to install
  */
  void addCallback(Callback* callback);

  /**
  * Remove a callback installed with addCallback
  * @param[in ] callback Callback to remove
  */
  void removeCallback(Callback* callback);

  /**
  * Install a callback that will be notified when frame processing has positioned the camera.
  * @param[in ] handler PostCameraEventHandler to install
  */
  void addPostCameraEventHandler(PostCameraEventHandler* handler);

  /**
  * Remove a handler installed with addPostCameraEventHandler
  * @param[in ] handler PostCameraEventHandler to remove
  */
  void removePostCameraEventHandler(PostCameraEventHandler* handler);

  /**
   * Causes all views to update. Call this periodically to refresh
   * the display and process input events.
   * @param simtime Time to pass to the Composite Viewer's frame()
   * @return 0 on success, non-zero on fatal error while rendering
   */
  virtual int frame(double simtime = 0.0);

  /** Enters a run loop that will automatically call frame() continuously. */
  virtual int run();

  /** Access the underlying OSG viewer for the first view. */
  osgViewer::CompositeViewer* getViewer() const;
  /** Access the underlying OSG viewer for a given view. */
  osg::ref_ptr<osgViewer::CompositeViewer> getViewer(simVis::View* view) const;

protected:
  virtual ~ViewManager();

private:
  void init_();
  void init_(osg::ArgumentParser&);
  simVis::View* getTopLevelView_(simVis::View* view) const;

  osg::ref_ptr<osgViewer::CompositeViewer> initialViewer_;
  std::map<simVis::View*, osg::ref_ptr<osgViewer::CompositeViewer>> viewers_;

  typedef std::vector<osg::ref_ptr<Callback> > Callbacks;
  Callbacks callbacks_;
  void fireCallbacks(simVis::View* view, const Callback::EventType& e);

  typedef std::vector<osg::observer_ptr<PostCameraEventHandler> > PostCameraEventHandlers;
  PostCameraEventHandlers postCameraEventHandlers_;
  void sendPostCameraFrameNotifications_();

  struct RemoveView : public osg::Operation
  {
    RemoveView(ViewManager*, View*);
    void operator()(osg::Object*);
    osg::ref_ptr<View>             view_;
    osg::observer_ptr<ViewManager> viewman_;
  };
  friend struct RemoveView;

  osg::ref_ptr<osgGA::GUIEventHandler> resizeHandler_;
  /// Cache fatal rendering flag to prevent rendering to invalid GL canvases
  bool fatalRenderFlag_ = false;

  bool firstFrame_ = true;
  std::optional<osg::ArgumentParser> args_;
};

/**
 * Given a GUI Event Handler, will add the event handler to every new inset and
 * remove it from every removed inset, when the callback is activated
 */
class SDKVIS_EXPORT AddEventHandlerToViews : public simVis::ViewManager::Callback
{
public:
  /** Constructor; accepts a GUI Event Handler to add to views in the View Manager. */
  explicit AddEventHandlerToViews(osgGA::GUIEventHandler* guiEventHandler);

  /** Adds the handler to existing views in the view manager. */
  void addToViews(const simVis::ViewManager& viewManager);
  /** Removes the handler from all views in the view manager. */
  void removeFromViews(const simVis::ViewManager& viewManager);

  /** Add or remove the event handler (override from Callback) */
  virtual void operator()(simVis::View* inset, const EventType& e);

protected:
  /** Derived from osg::Referenced */
  virtual ~AddEventHandlerToViews();

private:
  osg::ref_ptr<osgGA::GUIEventHandler> guiEventHandler_;
};

}

#endif /* SIMVIS_VIEW_MANAGER_H */
