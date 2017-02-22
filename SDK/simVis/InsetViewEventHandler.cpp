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
#include "simVis/InsetViewEventHandler.h"
#include "simVis/Utils.h"
#include "simVis/EarthManipulator.h"
#include "simNotify/Notify.h"
#include "osg/Geometry"
#include "osg/Geode"
#include "osg/MatrixTransform"
#include "osg/Notify"
#include "osg/LineStipple"
#include "osgDB/ReadFile"

#define SIM_TEST SIM_DEBUG_FP
//#define SIM_TEST_SIM_WARN

using namespace simVis;


namespace
{
  // builds the geometry representing the "rubber band" graphic for selecting a new
  // inset view rectangle
  static osg::MatrixTransform* createRubberBand()
  {
    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(4);
    (*verts)[0].set(0, 0, 0);
    (*verts)[1].set(0, 1, 0);
    (*verts)[2].set(1, 1, 0);
    (*verts)[3].set(1, 0, 0);

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].set(1, 1, 1, 1);

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    geom->setUseVertexBufferObjects(true);

    geom->setVertexArray(verts);
    geom->setColorArray(colors);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, 0, 4));
    geom->getOrCreateStateSet()->setAttributeAndModes(new osg::LineStipple(6, 0x5555), 1);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    geode->addDrawable(geom);
    simVis::setLighting(geode->getOrCreateStateSet(),
        osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    osg::MatrixTransform* xform = new osg::MatrixTransform();
    xform->addChild(geode);

    geom->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, 0);
    geode->setCullingActive(false);

    return xform;
  }

  /// true if the two events combine to make a mouse click at a
  /// single location
  static bool isMouseClick(const osgGA::GUIEventAdapter* downEv, const osgGA::GUIEventAdapter* upEv)
  {
    return
      downEv &&
      upEv &&
      osg::absolute(upEv->getX() - downEv->getX()) <= 3.0f &&
      osg::absolute(upEv->getY() - downEv->getY()) <= 3.0f;
  }
}

#undef  LC
#define LC "[FocusDetector] "

namespace
{
  /**
   * Event handler that detect actions in a view and reports
   * focus based on those actions.
   */
  struct FocusDetector : public osgGA::GUIEventHandler
  {
    /** Constructor */
    FocusDetector(FocusManager* focusMan, InsetViewEventHandler* handler)
      : focusMan_(focusMan), handler_(handler) { }

    /// process events.
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
      if (focusMan_.valid() && handler_.valid())
      {
        int mask = handler_->getFocusActions();
        osgGA::GUIEventAdapter::EventType e = ea.getEventType();

        if ((mask & handler_->ACTION_HOVER))
        {
          if (e == ea.MOVE)
          {
            focusMan_->focus(dynamic_cast<simVis::View*>(aa.asView()));
          }
        }
        else if (mask & InsetViewEventHandler::ACTION_CLICK_SCROLL)
        {
          if (e == ea.PUSH || e == ea.SCROLL)
          {
            focusMan_->focus(dynamic_cast<simVis::View*>(aa.asView()));
          }
        }

        if (mask & InsetViewEventHandler::ACTION_TAB)
        {
          if (e == ea.KEYDOWN && ea.getKey() == ea.KEY_Tab)
          {
            focusMan_->cycleFocus();
          }
        }
      }
      return false;
    };

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "FocusDetector"; }

    /** Focus manager */
    osg::observer_ptr<FocusManager>          focusMan_;
    /** Inset view event handler */
    osg::observer_ptr<InsetViewEventHandler> handler_;
  };


  /**
   * ViewManager callback that notifies us of new insets.
   */
  struct ViewListener : public ViewManager::Callback
  {
    /** Constructor */
    explicit ViewListener(osgGA::GUIEventHandler* focusDetector) : focusDetector_(focusDetector) { }

    /** Adds or removes a focus detector when a view is created in the View Manager */
    void operator()(simVis::View* view, const EventType& e)
    {
      if (focusDetector_.valid())
      {
        if (e == VIEW_ADDED && view->getHostView()) // it's an inset
        {
          view->addEventHandler(focusDetector_.get());
        }
        else if (e == VIEW_REMOVED)
        {
          view->removeEventHandler(focusDetector_.get());
        }
      }
    }

    /** Responsible for detecting focus changes */
    osg::observer_ptr<osgGA::GUIEventHandler> focusDetector_;
  };
}


//------------------------------------------------------------------------

#undef  LC
#define LC "[InsetViewEventHandler] "

InsetViewEventHandler::InsetViewEventHandler(simVis::View* host) :
addInsetMode_(false),
newInsetActionInProgress_(false),
viewNameGen_(0),
focusActionsMask_(ACTION_HOVER),
host_(host),
rubberBand_(createRubberBand())
{
  // add an (invisible) rubber band to the HUD.
  rubberBand_->setNodeMask(0);
  host_->getOrCreateHUD()->addChild(rubberBand_.get());

  // this callback will allow this object to listen to view events.
  focusDetector_ = new FocusDetector(host_->getFocusManager(), this);
  host_->addEventHandler(focusDetector_.get());

  // install an inset focus tracker for each of the insets.
  simVis::View::Insets insets;
  host_->getInsets(insets);
  for (simVis::View::Insets::iterator i = insets.begin(); i != insets.end(); ++i)
    (*i)->addEventHandler(focusDetector_.get());

  // listen the ViewManager so we can account for any new insets
  // that appear.
  ensureViewListenerInstalled_();
}


InsetViewEventHandler::~InsetViewEventHandler()
{
  if (host_.valid())
  {
    // tear everything down
    host_->getOrCreateHUD()->removeChild(rubberBand_.get());

    // uninstall the focus detector from any insets:
    simVis::View::Insets insets;
    host_->getInsets(insets);
    for (simVis::View::Insets::iterator i = insets.begin(); i != insets.end(); ++i)
      (*i)->removeEventHandler(focusDetector_.get());

    // uninstall the focus detector from the host view:
    host_->removeEventHandler(focusDetector_.get());

    // uninstall the host view listener for this handler:
    if (host_->getViewManager())
      host_->getViewManager()->removeCallback(viewListener_.get());
  }
}

void InsetViewEventHandler::ensureViewListenerInstalled_()
{
  // install hooks to get all the events we need:
  if (!viewListener_.valid() && host_.valid() && host_->getViewManager())
  {
    viewListener_ = new ViewListener(focusDetector_.get());
    host_->getViewManager()->addCallback(viewListener_.get());
  }
}

simVis::View* InsetViewEventHandler::getView()
{
  return host_.get();
}

void InsetViewEventHandler::setAddInsetMode(bool add)
{
  if (addInsetMode_ == add)
    return;

  addInsetMode_ = add;
  if (!addInsetMode_)
    cancelNewInsetAction_();
}

bool InsetViewEventHandler::isAddInsetMode() const
{
  return addInsetMode_;
}

void InsetViewEventHandler::setFocusActions(int mask)
{
  focusActionsMask_ = mask;
}

int InsetViewEventHandler::getFocusActions() const
{
  return focusActionsMask_;
}

bool InsetViewEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (!addInsetMode_)
    return false;

  bool handled = false;

  // Keep the mouse X and Y position values non-negative and inside the view, even when dragging
  const int mouseX = static_cast<int>(osg::clampBetween(ea.getX(), ea.getXmin(), ea.getXmax()));
  const int mouseY = static_cast<int>(osg::clampBetween(ea.getY(), ea.getYmin(), ea.getYmax()));

  osgGA::GUIEventAdapter::EventType e = ea.getEventType();

  // the input mask is such that we might be processing a new inset action:
  bool active = (ea.getButtonMask() == ea.LEFT_MOUSE_BUTTON);

  // start a new inset action?
  if (!newInsetActionInProgress_)
  {
    if (active && e == ea.PUSH)
    {
      beginNewInsetAction_(mouseX, mouseY);
      aa.requestRedraw();
      handled = true;
    }
  }

  // inset action already in progress:
  else
  {
   if (e == ea.RELEASE)
    {
      completeNewInsetAction_(mouseX, mouseY);
      aa.requestRedraw();
      handled = true;
    }
    else if (e == ea.DRAG)
    {
      updateNewInsetAction_(mouseX, mouseY);
      aa.requestRedraw();
      handled = true;
    }
  }

  if (!handled && e == ea.FRAME)
  {
    ensureViewListenerInstalled_();
  }

  return true;
}

void InsetViewEventHandler::beginNewInsetAction_(int mx, int my)
{
  newInsetX0_ = mx;
  newInsetY0_ = my;
  rubberBand_->setNodeMask(~0);
  rubberBand_->setMatrix(osg::Matrix::translate(mx, my, 0));
  newInsetActionInProgress_ = true;
}

void InsetViewEventHandler::updateNewInsetAction_(int mx, int my)
{
  rubberBand_->setMatrix(
    osg::Matrix::scale(mx - newInsetX0_, my - newInsetY0_, 1) *
    osg::Matrix::translate(newInsetX0_, newInsetY0_, 0));
}

void InsetViewEventHandler::completeNewInsetAction_(int mx, int my)
{
  rubberBand_->setNodeMask(0);

  int x = mx > newInsetX0_? newInsetX0_ : mx;
  int y = my > newInsetY0_? newInsetY0_ : my;
  int w = osg::absolute(mx - newInsetX0_);
  int h = osg::absolute(my - newInsetY0_);

  simVis::View* inset = new simVis::View();
  inset->setName(host_->getUniqueInsetName());
  inset->setSceneManager(getView()->getSceneManager());
  if (host_.valid())
    inset->applyManipulatorSettings(*host_);

  const simVis::View::Extents& hostex = host_->getExtents();
  float xr = ((float)x - hostex.x_) / hostex.width_;
  float yr = ((float)y - hostex.y_) / hostex.height_;
  float wr = (float)w / hostex.width_;
  float hr = (float)h / hostex.height_;
  inset->setExtentsAsRatio(xr, yr, wr, hr);

  // Copy over some, but not all, reasonable eye position data
  inset->setViewpoint(host_->getViewpoint(), 0.0);
  if (host_->isOverheadEnabled())
    inset->enableOverheadMode(true);
  if (host_->getCameraTether() != NULL)
    inset->tetherCamera(host_->getCameraTether());

  // Do add after a complete build
  host_->addInset(inset);
  newInsetActionInProgress_ = false;
}

void InsetViewEventHandler::cancelNewInsetAction_()
{
  rubberBand_->setNodeMask(0);
  newInsetActionInProgress_ = false;
}
