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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
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
 : popupHandler_(new simVis::PopupHandler(&picker, &view)),
  popupHandler2_(new simVis::PopupHandler2(&picker, &view)),
  usePopupHandler2_(false)
{
  drawView_ = &view;
  frameTimer_ = new FrameTimer(this);
  drawView_->addEventHandler(frameTimer_);
}

PlatformPopupManipulator::PlatformPopupManipulator(simVis::SceneManager& scene, simVis::View& view)
 : popupHandler_(new simVis::PopupHandler(&scene, &view)),
  usePopupHandler2_(false)
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
  if (usePopupHandler2_ && popupHandler2_)
    popupHandler2_->enable(v);
  else
    popupHandler_->enable(v);
}

void PlatformPopupManipulator::clear()
{
  if (usePopupHandler2_ && popupHandler2_)
    popupHandler2_->clear();
  else
    popupHandler_->clear();
}

bool PlatformPopupManipulator::isEnabled() const
{
  if (usePopupHandler2_ && popupHandler2_)
    return popupHandler2_->isEnabled();
  return popupHandler_->isEnabled();
}

void PlatformPopupManipulator::setShowInCorner(bool showInCorner)
{
  if (usePopupHandler2_ && popupHandler2_)
    popupHandler2_->setShowInCorner(showInCorner);
  else
    popupHandler_->setShowInCorner(showInCorner);
}

simVis::PopupHandler* PlatformPopupManipulator::popupHandler() const
{
  return popupHandler_.get();
}

simVis::PopupHandler2* PlatformPopupManipulator::popupHandler2() const
{
  return popupHandler2_.get();
}

void PlatformPopupManipulator::setUsePopupHandler2()
{
  usePopupHandler2_ = true;
}

int PlatformPopupManipulator::move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Pass the move event to the popupHandler_, who reinterprets as needed to update its mouse position
  int rv = 0;
  if (usePopupHandler2_ && popupHandler2_)
    rv = popupHandler2_->handle(ea, aa) ? 1 : 0;
  else
    rv = popupHandler_->handle(ea, aa) ? 1 : 0;
  // Now update the last mouse view if it has changed
  osg::observer_ptr<simVis::View> currentView = static_cast<simVis::View*>(aa.asView());
  if (lastMouseView_ != currentView)
    lastMouseView_ = currentView;
  return rv;
}

void PlatformPopupManipulator::updatePopupHandler_()
{
  if (lastMouseView_.valid())
  {
    if (usePopupHandler2_ && popupHandler2_)
      popupHandler2_->updatePopupFromView(lastMouseView_.get());
    else
      popupHandler_->updatePopupFromView(lastMouseView_.get());
  }
}

}

