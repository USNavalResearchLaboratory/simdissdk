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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#include "simVis/Popup.h"
#include "simVis/View.h"
#include "simUtil/PlatformPopupManipulator.h"

namespace simUtil
{

/// Class for updating the popup on FRAME event
class PlatformPopupManipulator::FrameTimer : public osgGA::GUIEventHandler
{
public:
  /** Constructor */
  explicit FrameTimer(PlatformPopupManipulator* parent)
    :parent_(parent)
  {}

  /** implement the handle method to update on FRAME events */
  virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (ea.getEventType() == ea.FRAME)
      parent_->updatePopupHandler_();
    return false;
  }

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "PlatformPopupManipulator::FrameTimer"; }

private:
  PlatformPopupManipulator* parent_;
};


// --------------------------------------------------------------------------
PlatformPopupManipulator::PlatformPopupManipulator(simVis::Picker& picker, simVis::View& view)
 : popupHandler_(new simVis::PopupHandler(&picker, &view))
{
  drawView_ = &view;
  frameTimer_ = new FrameTimer(this);
  drawView_->addEventHandler(frameTimer_);
}

PlatformPopupManipulator::PlatformPopupManipulator(simVis::SceneManager& scene, simVis::View& view)
 : popupHandler_(new simVis::PopupHandler(&scene, &view))
{
  drawView_ = &view;
  frameTimer_ = new FrameTimer(this);
  drawView_->addEventHandler(frameTimer_);
}

PlatformPopupManipulator::~PlatformPopupManipulator()
{
  if (drawView_.valid())
    drawView_->removeEventHandler(frameTimer_);
}

void PlatformPopupManipulator::enable(bool v)
{
  popupHandler_->enable(v);
}

void PlatformPopupManipulator::clear()
{
  popupHandler_->clear();
}

bool PlatformPopupManipulator::isEnabled() const
{
  return popupHandler_->isEnabled();
}

void PlatformPopupManipulator::setShowInCorner(bool showInCorner)
{
  popupHandler_->setShowInCorner(showInCorner);
}

simVis::PopupHandler* PlatformPopupManipulator::popupHandler() const
{
  return popupHandler_.get();
}

int PlatformPopupManipulator::move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Pass the move event to the popupHandler_, who reinterprets as needed to update its mouse position
  const int rv = popupHandler_->handle(ea, aa) ? 1 : 0;
  // Now update the last mouse view if it has changed
  osg::observer_ptr<simVis::View> currentView = static_cast<simVis::View*>(aa.asView());
  if (lastMouseView_ != currentView)
    lastMouseView_ = currentView;
  return rv;
}

void PlatformPopupManipulator::updatePopupHandler_()
{
  if (lastMouseView_.valid())
    popupHandler_->updatePopupFromView(lastMouseView_.get());
}

}

