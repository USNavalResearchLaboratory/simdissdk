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
#include <cstring>
#include <iterator>
#include "simNotify/Notify.h"
#include "simVis/Gl3Utils.h"
#include "simVis/osgEarthVersion.h"
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
      osg::GraphicsContext* gc = dynamic_cast<osg::GraphicsContext*>(gc_obj);
      simVis::applyCoreProfileValidity(gc);
      simVis::applyMesaGeometryShaderFix(gc);
      const int width = gc->getTraits()->width;
      const int height = gc->getTraits()->height;

      for (unsigned int i = 0; i < viewman_->getNumViews(); ++i)
      {
        // View Manager does matching based on width/height against the view's
        // viewport, so we can't modify width/height here even if they are invalid (0)
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
      viewMan_(viewMan),
      width_(0),
      height_(0),
      resizeView_(NULL)
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
        viewMan_->handleResize(width_, height_);
        aa.requestRedraw();
        resizeView_ = NULL;
      }
      return false;
    }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "OnResize"; }

  private:
    osg::observer_ptr<simVis::ViewManager> viewMan_;
    int width_;
    int height_;
    osg::View* resizeView_;
  };
}

#if 0
//........................................................................


ViewManager::AddView::AddView(ViewManager* viewman, View* view) :
osg::Operation("ViewManager::AddView", false),
view_(view),
viewman_(viewman)
{
    //nop
}

void
ViewManager::AddView::operator()(osg::Object* obj)
{
    //viewman_->viewer_->addView( view );
    view_->setViewManager(viewman_.get());
    viewman_->fireCallbacks(view, Callback::VIEW_ADDED);
}
#endif

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

    viewman_->viewer_->removeView(view_.get());
    viewman_->fireCallbacks(view_.get(), Callback::VIEW_REMOVED);
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
  if (guiEventHandler_ != NULL)
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
  : fatalRenderFlag_(false),
    firstFrame_(true)
{
  init_();
}


ViewManager::ViewManager(osg::ArgumentParser& args)
  : fatalRenderFlag_(false)
{
  init_(args);
}


ViewManager::~ViewManager()
{
}


void ViewManager::addView(simVis::View* view)
{
  if (view)
  {
    viewer_->addView(view);
    view->setViewManager(this);
    view->addEventHandler(resizeHandler_.get());
    fireCallbacks(view, Callback::VIEW_ADDED);
  }
}


void ViewManager::removeView(simVis::View* view)
{
  if (view)
  {
    view->removeEventHandler(resizeHandler_.get());
    // viewer_->removeView() has to happen in the update operation.
    viewer_->addUpdateOperation(new RemoveView(this, view));
  }
}


void ViewManager::getViews(std::vector<simVis::View*>& views) const
{
  std::vector<osgViewer::View*> temp;
  viewer_->getViews(temp);

  views.clear();
  for (unsigned int i = 0; i < temp.size(); ++i)
  {
    simVis::View* view = dynamic_cast<simVis::View*>(temp[i]);
    if (view)
      views.push_back(view);
  }
}


unsigned int ViewManager::getNumViews() const
{
  return viewer_->getNumViews();
}


simVis::View* ViewManager::getView(unsigned int index) const
{
  return index < getNumViews() ? dynamic_cast<simVis::View*>(viewer_->getView(index)) : NULL;
}


simVis::View* ViewManager::getViewByName(const std::string& name) const
{
  std::vector<osgViewer::View*> temp;
  viewer_->getViews(temp);
  for (std::vector<osgViewer::View*>::const_iterator i = temp.begin(); i != temp.end(); ++i)
  {
    if ((*i)->getName() == name)
      return dynamic_cast<simVis::View*>(*i);
  }
  return NULL;
}


int ViewManager::getIndexOf(simVis::View* view) const
{
  std::vector<osgViewer::View*> temp;
  viewer_->getViews(temp);
  std::vector<osgViewer::View*>::const_iterator iter = std::find(temp.begin(), temp.end(), static_cast<osgViewer::View*>(view));
  if (iter == temp.end())
    return -1;
  return iter - temp.begin();
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

void ViewManager::handleResize(int newwidth, int newheight)
{
  unsigned int numViews = getNumViews();
  for (unsigned int i = 0; i < numViews; ++i)
  {
    getView(i)->processResize(newwidth, newheight);
  }
}

void ViewManager::init_()
{
  viewer_ = new osgViewer::CompositeViewer();
  viewer_->setThreadingModel(viewer_->SingleThreaded);
  viewer_->setRealizeOperation(new OnRealize(this));
  resizeHandler_ = new OnResize(this);

  // Note that in the case of multiple view managers in an application, the last
  // View Manager's viewer's frame stamp wins.
  simVis::Registry::instance()->setFrameStamp(viewer_->getFrameStamp());
}

void ViewManager::init_(osg::ArgumentParser& args)
{
  viewer_ = new osgViewer::CompositeViewer(args);
  viewer_->setThreadingModel(viewer_->SingleThreaded);
  viewer_->setRealizeOperation(new OnRealize(this));
  resizeHandler_ = new OnResize(this);

  // Note that in the case of multiple view managers in an application, the last
  // View Manager's viewer's frame stamp wins.
  simVis::Registry::instance()->setFrameStamp(viewer_->getFrameStamp());
}

int ViewManager::frame(double simulationTime)
{
  // Avoid rendering to a canvas that has an exception during the frame(), to avoid
  // rendering to something with unknown state (that will likely fail again)
  if (fatalRenderFlag_)
    return 1;

  // Add a small epsilon to the simulation time to avoid simulating at time 0.0 due
  // to rendering issues in Triton.  Note that negative time is acceptable, but time
  // at 0.0 is not due to minor rendering glitches at time 0 in Triton.
  static const double MINIMUM_TIME = 1e-5;
  if (fabs(simulationTime) < MINIMUM_TIME)
    simulationTime = (simulationTime < 0 ? -MINIMUM_TIME : MINIMUM_TIME);

  if (viewer_->getRunFrameScheme() == osgViewer::ViewerBase::CONTINUOUS ||
    viewer_->checkNeedToDoFrame())
  {
    try
    {
      fatalRenderFlag_ = true;

      if ( !viewer_->done() )
      {
        if (firstFrame_)
        {
          // Called on the first frame because viewer_->init() is protected
          viewer_->frame(simulationTime);
          if ( !viewer_->isRealized() )
          {
            viewer_->realize();
          }
          firstFrame_ = false;
        }

        viewer_->advance(simulationTime);
        viewer_->eventTraversal();
        viewer_->updateTraversal();

        // post-update. This is a good place to update anything that relies on the
        // current camera position.
        sendPostCameraFrameNotifications_();

        viewer_->renderingTraversals();
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
  return viewer_->ViewerBase::run();
}

}
