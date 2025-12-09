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
#include <algorithm>
#include <cstring>
#include <iterator>
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "simVis/Gl3Utils.h"
#include "simVis/Registry.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"

namespace
{
  /**
   * GC realize operation that will propagate the initial window size
   * to all pre-configured views.
   */
  struct OnRealize : public osg::Operation
  {
    osg::observer_ptr<simVis::ViewManager> viewman_;
    explicit OnRealize(simVis::ViewManager* viewman) : viewman_(viewman) { }
    void operator()(osg::Object* gc_obj)
    {
      if (!viewman_.valid())
        return;

      osg::GraphicsContext* gc = dynamic_cast<osg::GraphicsContext*>(gc_obj);
      if (!gc)
        return;

      simVis::applyCoreProfileValidity(gc);
      simVis::applyMesaGeometryShaderFix(gc);
      const int width = gc->getTraits()->width;
      const int height = gc->getTraits()->height;

      for (unsigned int i = 0; i < viewman_->getNumViews(); ++i)
      {
        simVis::View* view = viewman_->getView(i);

        // View Manager does matching based on width/height against the view's
        // viewport, so we can't modify width/height here even if they are invalid (0)
        if (view && view->getCamera() && view->getCamera()->getGraphicsContext() == gc)
          viewman_->getView(i)->processResize(width, height);
      }
    }
  };

  /**
   * Event handler that will process resize events in order to
   * properly size insets.
   */
  struct OnResize : public osgGA::GUIEventHandler
  {
    explicit OnResize(simVis::ViewManager* viewMan) :
      viewMan_(viewMan)
      { }

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor*)
    {
      if (!viewMan_.valid())
        return false;

      if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE)
      {
        width_ = ea.getWindowWidth();
        height_ = ea.getWindowHeight();
        resizeView_ = aa.asView();
      }
      // wait until subsequent frame event to resize
      else if (resizeView_ && ea.getEventType() == osgGA::GUIEventAdapter::FRAME && aa.asView() == resizeView_)
      {
        const osg::Camera* camera = resizeView_->getCamera();
        viewMan_->handleResize(camera ? camera->getGraphicsContext() : nullptr, width_, height_);
        aa.requestRedraw();
        resizeView_ = nullptr;
      }
      return false;
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "OnResize"; }

  private:
    osg::observer_ptr<simVis::ViewManager> viewMan_;
    int width_ = 0;
    int height_ = 0;
    osg::View* resizeView_ = nullptr;
  };
}

//........................................................................
namespace simVis
{

ViewManager::RemoveView::RemoveView(ViewManager* viewman, View* view)
  : osg::Operation("ViewManager::RemoveView", false),
    view_(view),
    viewman_(viewman)
{
    //nop
}

void ViewManager::RemoveView::operator()(osg::Object* obj)
{
  if (viewman_.valid() && view_.valid())
  {
    // Removing the camera's children on the view prevents OSG from running
    // a releaseGLObjects() traversal on the deleted View's scene graph. This is
    // important since we are sharing the graph with the host and don't want
    // textures, etc. to be released.
    //
    // (Note: OSG would normally detect a situation where more than one camera was
    // sharing a common graph, but it only checks the root node. Since each of
    // our simVis::View objects has a unique root node this doesn't work. -gw)
    view_->getCamera()->removeChildren(0, view_->getCamera()->getNumChildren());

    auto viewer = viewman_->getViewer(view_.get());
    if (viewer.valid())
      viewer->removeView(view_.get());

    viewman_->fireCallbacks(view_.get(), Callback::VIEW_REMOVED);

    // Might have removed a top level, in which case we remove it from viewers_
    if (viewer.valid() && viewer->getNumViews() == 0)
      viewman_->viewers_.erase(view_.get());
  }
}

//........................................................................

AddEventHandlerToViews::AddEventHandlerToViews(osgGA::GUIEventHandler* guiEventHandler)
  : guiEventHandler_(guiEventHandler)
{
}

void AddEventHandlerToViews::addToViews(const simVis::ViewManager& viewManager)
{
  std::vector<simVis::View*> views;
  viewManager.getViews(views);
  for (std::vector<simVis::View*>::const_iterator i = views.begin(); i != views.end(); ++i)
    (*i)->addEventHandler(guiEventHandler_);
}

void AddEventHandlerToViews::removeFromViews(const simVis::ViewManager& viewManager)
{
  std::vector<simVis::View*> views;
  viewManager.getViews(views);
  for (std::vector<simVis::View*>::const_iterator i = views.begin(); i != views.end(); ++i)
    (*i)->removeEventHandler(guiEventHandler_);
}


void AddEventHandlerToViews::operator()(simVis::View* inset, const EventType& e)
{
  if (guiEventHandler_ != nullptr)
  {
    switch (e)
    {
    case VIEW_ADDED:
      inset->addEventHandler(guiEventHandler_);
      break;
    case VIEW_REMOVED:
      inset->removeEventHandler(guiEventHandler_);
      break;
    }
  }
}

AddEventHandlerToViews::~AddEventHandlerToViews()
{
}

//........................................................................

ViewManager::ViewManager()
{
  init_();
}


ViewManager::ViewManager(osg::ArgumentParser& args)
{
  init_(args);
}


ViewManager::~ViewManager()
{
}

simVis::View* ViewManager::getTopLevelView_(simVis::View* view) const
{
  // Recursive function to find the top-level ancestor.
  if (!view || view->getHostView() == nullptr)
    return view; // This is the top-level view.
  return getTopLevelView_(view->getHostView()); // Recursively check the host.
}

osg::ref_ptr<osgViewer::CompositeViewer> ViewManager::getViewer(simVis::View* view) const
{
  auto* topView = getTopLevelView_(view);
  if (!topView)
    return nullptr;
  auto it = viewers_.find(topView);
  if (it == viewers_.end())
    return nullptr;
  return it->second.get();
}

osgViewer::CompositeViewer* ViewManager::getViewer() const
{
  return initialViewer_.get();
}

void ViewManager::addView(simVis::View* view)
{
  if (!view)
    return;

  if (!view->getHostView())
  {
    osg::ref_ptr<osgViewer::CompositeViewer> compositeViewer;
    if (viewers_.empty() || !useMultipleViewers_)
      compositeViewer = initialViewer_;
    else
    {
      if (args_.has_value())
        compositeViewer = new osgViewer::CompositeViewer(args_.value());
      else
        compositeViewer = new osgViewer::CompositeViewer;
      compositeViewer->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
      compositeViewer->setRealizeOperation(new OnRealize(this));
    }
    compositeViewer->addView(view);
    viewers_[view] = compositeViewer;
    view->setViewManager(this);
    view->addEventHandler(resizeHandler_.get());
    fireCallbacks(view, Callback::VIEW_ADDED);

    // Also, set the framestamp on the registry.
    simVis::Registry::instance()->setFrameStamp(compositeViewer->getFrameStamp());
    return;
  }

  auto compositeViewer = getViewer(view);
  if (!compositeViewer.valid())
  {
    SIM_ERROR << "Error: Could not find CompositeViewer for top-level view of inset." << std::endl;
    assert(0); // Should not happen
  }

  // Add the inset view to the top-level viewer.
  compositeViewer->addView(view);
  view->setViewManager(this);
  view->addEventHandler(resizeHandler_.get());
  fireCallbacks(view, Callback::VIEW_ADDED);
}

void ViewManager::removeView(simVis::View* view)
{
  if (!view)
    return;

  view->removeEventHandler(resizeHandler_.get());

  // viewer->removeView() has to happen in the update operation.
  auto viewer = getViewer(view);
  if (viewer.valid())
    viewer->addUpdateOperation(new RemoveView(this, view));
}

void ViewManager::getViews(std::vector<simVis::View*>& views) const
{
  views.clear(); // Clear the output vector.

  // Iterate through all the CompositeViewers in the map.
  for (const auto& pair : viewers_)
  {
    const osg::ref_ptr<osgViewer::CompositeViewer>& compositeViewer = pair.second;
    std::vector<osgViewer::View*> osgViews;
    compositeViewer->getViews(osgViews); // Get the views from the CompositeViewer.

    // Convert the osgViewer::View* to simVis::View* and add them to the output.
    for (osgViewer::View* osgView : osgViews)
    {
      simVis::View* view = dynamic_cast<simVis::View*>(osgView);
      if (view)
        views.push_back(view);
    }
  }
}

unsigned int ViewManager::getNumViews() const
{
  unsigned int count = 0;

  // Iterate through all the CompositeViewers and sum the number of views in each.
  for (const auto& pair : viewers_)
  {
    const osg::ref_ptr<osgViewer::CompositeViewer>& compositeViewer = pair.second;
    count += compositeViewer->getNumViews();
  }

  return count;
}

simVis::View* ViewManager::getView(unsigned int index) const
{
  unsigned int current_index = 0;

  // Iterate through all the CompositeViewers.
  for (const auto& pair : viewers_)
  {
    const osg::ref_ptr<osgViewer::CompositeViewer>& compositeViewer = pair.second;
    unsigned int numViewsInViewer = compositeViewer->getNumViews();

    // Check if the index falls within the range of this CompositeViewer.
    if (index >= current_index && index < current_index + numViewsInViewer)
    {
      // Get the view at the specified index within this CompositeViewer.
      osgViewer::View* osgView = compositeViewer->getView(index - current_index);
      return dynamic_cast<simVis::View*>(osgView);
    }

    // Update the current index to reflect the views already processed.
    current_index += numViewsInViewer;
  }

  // The index is out of range.
  return nullptr;
}

simVis::View* ViewManager::getViewByName(const std::string& name) const
{
  // Iterate through all the CompositeViewers.
  for (const auto& pair : viewers_)
  {
    const osg::ref_ptr<osgViewer::CompositeViewer>& compositeViewer = pair.second;
    std::vector<osgViewer::View*> osgViews;
    compositeViewer->getViews(osgViews);

    // Search for the view with the given name in this CompositeViewer.
    for (osgViewer::View* osgView : osgViews)
    {
      simVis::View* view = dynamic_cast<simVis::View*>(osgView);
      if (view && view->getName() == name)
        return view;
    }
  }

  // View not found.
  return nullptr;
}

simVis::View* ViewManager::getViewByMouseXy(const osg::Vec2d& mouseXy) const
{
  std::vector<simVis::View*> allViews;
  getViews(allViews);

  simVis::View* rv = nullptr;
  for (auto* view : allViews)
  {
    // Ignore invalid views, and views set up to ignore event focus
    if (!view || !view->getCamera())
      continue;
    auto* camera = view->getCamera();
    if (!camera->getViewport() || !camera->getAllowEventFocus() || (camera->getNodeMask() == 0))
      continue;

    // Save the last view, which is front-most
    auto* vp = camera->getViewport();
    if (simCore::isBetween(mouseXy.x(), vp->x(), vp->x() + vp->width()) &&
      simCore::isBetween(mouseXy.y(), vp->y(), vp->y() + vp->height()))
    {
      rv = view;
    }
  }
  return rv;
}

int ViewManager::getIndexOf(simVis::View* view) const
{
  int current_index = 0;

  // Iterate through all the CompositeViewers.
  for (const auto& pair : viewers_)
  {
    const osg::ref_ptr<osgViewer::CompositeViewer>& compositeViewer = pair.second;
    std::vector<osgViewer::View*> osgViews;
    compositeViewer->getViews(osgViews);

    // Search for the view in this CompositeViewer.
    for (osgViewer::View* osgView : osgViews)
    {
      if (osgView == view)
        return current_index; // Found the view, return its index.
      ++current_index;
    }
  }

  // View not found.
  return -1;
}

void ViewManager::addCallback(Callback* value)
{
  if (value)
  {
    callbacks_.push_back(value);
  }
}


void ViewManager::removeCallback(Callback* value)
{
  if (value)
  {
    Callbacks::iterator i = std::find(callbacks_.begin(), callbacks_.end(), value);
    if (i != callbacks_.end())
      callbacks_.erase(i);
  }
}


void ViewManager::fireCallbacks(simVis::View* view, const Callback::EventType& e)
{
  for (Callbacks::iterator i = callbacks_.begin(); i != callbacks_.end(); ++i)
  {
    i->get()->operator()(view, e);
  }
}

void ViewManager::addPostCameraEventHandler(PostCameraEventHandler* value)
{
  if (value)
  {
    postCameraEventHandlers_.push_back(value);
  }
}

void ViewManager::removePostCameraEventHandler(PostCameraEventHandler* value)
{
  if (value)
  {
    PostCameraEventHandlers::iterator i = std::find(postCameraEventHandlers_.begin(), postCameraEventHandlers_.end(), value);
    if (i != postCameraEventHandlers_.end())
      postCameraEventHandlers_.erase(i);
  }
}

void ViewManager::sendPostCameraFrameNotifications_()
{
  for (PostCameraEventHandlers::iterator i = postCameraEventHandlers_.begin(); i != postCameraEventHandlers_.end(); ++i)
  {
    if (i->valid())
      i->get()->operator()();
  }
}

void ViewManager::handleResize(const osg::GraphicsContext* gc, int newwidth, int newheight)
{
  for (const auto& pair : viewers_)
  {
    osg::ref_ptr<osgViewer::CompositeViewer> compositeViewer = pair.second;
    std::vector<osgViewer::View*> views;
    compositeViewer->getViews(views);

    for (osgViewer::View* osgView : views)
    {
      simVis::View* view = dynamic_cast<simVis::View*>(osgView);
      if (view && view->getCamera() && view->getCamera()->getGraphicsContext() == gc)
        view->processResize(newwidth, newheight);
    }
  }
}

void ViewManager::init_()
{
  initialViewer_ = new osgViewer::CompositeViewer;
  initialViewer_->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
  initialViewer_->setRealizeOperation(new OnRealize(this));

  resizeHandler_ = new OnResize(this);
}

void ViewManager::init_(osg::ArgumentParser& args)
{
  args_ = args;
  initialViewer_ = new osgViewer::CompositeViewer(args);
  initialViewer_->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
  initialViewer_->setRealizeOperation(new OnRealize(this));

  resizeHandler_ = new OnResize(this);
}

int ViewManager::frame(double simulationTime)
{
  // Avoid rendering to a canvas that has an exception during the frame(), to avoid
  // rendering to something with unknown state (that will likely fail again)
  if (fatalRenderFlag_)
    return 1;

  // Retrieve the first viewer; this only works in single viewer mode
  assert(viewers_.size() == 1); // Must have one viewer only
  if (viewers_.empty())
    return 1;
  auto viewer = viewers_.begin()->second;

  // Add a small epsilon to the simulation time to avoid simulating at time 0.0 due
  // to rendering issues in Triton.  Note that negative time is acceptable, but time
  // at 0.0 is not due to minor rendering glitches at time 0 in Triton.
  static const double MINIMUM_TIME = 1e-5;
  if (fabs(simulationTime) < MINIMUM_TIME)
    simulationTime = (simulationTime < 0 ? -MINIMUM_TIME : MINIMUM_TIME);

  if (viewer->getRunFrameScheme() == osgViewer::ViewerBase::CONTINUOUS ||
    viewer->checkNeedToDoFrame())
  {
    try
    {
      fatalRenderFlag_ = true;

      if ( !viewer->done() )
      {
        if (firstFrame_)
        {
          // Called on the first frame because viewer->init() is protected
          viewer->frame(simulationTime);
          if ( !viewer->isRealized() )
          {
            viewer->realize();
          }
          firstFrame_ = false;
        }

        viewer->advance(simulationTime);
        viewer->eventTraversal();
        viewer->updateTraversal();

        // post-update. This is a good place to update anything that relies on the
        // current camera position.
        sendPostCameraFrameNotifications_();

        viewer->renderingTraversals();
      }


      fatalRenderFlag_ = false;
    }
    catch (const std::exception& exc)
    {
      SIM_FATAL << "Exception rendering frame: " << exc.what() << ".  Unable to continue.\n";
      return 1;
    }
    catch (...)
    {
      SIM_FATAL << "Unknown exception rendering frame.  Unable to continue.  Try updating video drivers.\n";
      return 1;
    }
  }
  return 0;
}

int ViewManager::run()
{
  return getViewer()->ViewerBase::run();
}

void ViewManager::setUseMultipleViewers(bool useMultipleViewers)
{
  useMultipleViewers_ = useMultipleViewers;
}

bool ViewManager::getUseMultipleViewers() const
{
  return useMultipleViewers_;
}

}
