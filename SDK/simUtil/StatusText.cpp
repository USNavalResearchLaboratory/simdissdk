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
#include <cassert>
#include "simCore/String/TextReplacer.h"
#include "simUtil/HudManager.h"
#include "simVis/View.h"
#include "simUtil/StatusText.h"

namespace simUtil
{

/** Callback handler for frame updates */
class StatusText::FrameEventHandler : public osgGA::GUIEventHandler
{
public:
  /** Constructor */
  explicit FrameEventHandler(simUtil::StatusText* parent) : parent_(parent) {}
  /** Handles frame updates and returns false so other handlers can process as well */
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (!parent_.valid())
      return false;
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
    {
      // Update the text each frame
      parent_->update_();
    }

    else if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE)
    {
      // Tell the text to resize if the screen size has changed
      const osg::View* view = aa.asView();
      const osg::Camera* camera = (view) ? view->getCamera() : NULL;
      const osg::Viewport* viewport = (camera) ? camera->getViewport() : NULL;
      if (viewport)
      {
        const int widthPx = static_cast<int>(viewport->width());
        const int heightPx = static_cast<int>(viewport->height());
        if (widthPx != widthPx_ || heightPx != heightPx_)
        {
          widthPx_ = widthPx;
          heightPx_ = heightPx;
          parent_->resize_(widthPx_, heightPx_);
        }
      }
    }
    return false;
  }
  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }
  /** Return the class name */
  virtual const char* className() const { return "StatusText::FrameEventHandler"; }

protected:
  /** Protect osg::Referenced-derived destructor */
  virtual ~FrameEventHandler(){}

private:
  osg::observer_ptr<simUtil::StatusText> parent_;
  int widthPx_;
  int heightPx_;
};

//-------------------------------------------------------------------------------------------------------

StatusText::StatusText(simVis::View* view, simCore::TextReplacerPtr textReplacer, Position pos)
  : view_(view),
  frameEventHandler_(NULL),
  textReplacer_(textReplacer),
  position_(pos)
{
  const osg::Viewport* vp = view->getCamera()->getViewport();
  windowWidthPx_ = static_cast<int>(vp->width());
  windowHeightPx_ = static_cast<int>(vp->height());

  frameEventHandler_ = new FrameEventHandler(this);
}

StatusText::~StatusText()
{
  removeFromView();
}


int StatusText::setStatusSpec(const std::string& statusSpec, const osg::Vec4f& color, double fontSize, const std::string& font)
{
  if (view_.valid() && textReplacer_ && !statusSpec.empty())
  {
    // verify the spec string parses before creating the status display
    std::string replacerOut = textReplacer_->format(statusSpec);
    if (replacerOut.empty())
      return 1;

    statusSpec_ = statusSpec;
    if (!statusHudText_.valid())
      create_(replacerOut, color, font, fontSize);
    else
      statusHudText_->update(replacerOut, statusHudText_->x(), statusHudText_->y(), statusHudText_->isPercentageX(), statusHudText_->isPercentageY(), statusHudText_->hAlignment(), statusHudText_->vAlignment(), color, font, fontSize);
  }
  else if (statusSpec.empty() && statusHudText_.valid())
    removeFromView();
  return 0;
}

void StatusText::removeFromView()
{
  if (view_.valid())
  {
    view_->removeEventHandler(frameEventHandler_);
    view_->getOrCreateHUD()->removeChild(this);
    removeChild(statusHudText_.get());
    statusHudText_ = NULL;
  }
}

// column text implementation requires that all lines are specified with same number of columns
void StatusText::create_(const std::string& status, const osg::Vec4f& color, const std::string& font, double fontSize)
{
  static const double MARGIN_PERCENT = 0.5;
  view_->addEventHandler(frameEventHandler_);
  view_->getOrCreateHUD()->addChild(this);

  // Create the HUD text
  statusHudText_ = new HudColumnText(windowWidthPx_, windowHeightPx_);
  addChild(statusHudText_);

  // each StatusText::Position is implemented using relative/percent positioning
  switch (position_)
  {
  case simUtil::StatusText::LEFT_BOTTOM:
    // ALIGN_BOTTOM positions text at absolute bottom, MARGIN_PERCENT adds a little margin
    statusHudText_->update(status, MARGIN_PERCENT, MARGIN_PERCENT, true, true, simUtil::ALIGN_LEFT, simUtil::ALIGN_BOTTOM, color, font, fontSize);
    break;
  case simUtil::StatusText::LEFT_CENTER:
    statusHudText_->update(status, MARGIN_PERCENT, 50.0, true, true, simUtil::ALIGN_LEFT, simUtil::ALIGN_CENTER_Y, color, font, fontSize);
    break;
  case simUtil::StatusText::LEFT_TOP:
    statusHudText_->update(status, MARGIN_PERCENT, 100.0 - MARGIN_PERCENT, true, true, simUtil::ALIGN_LEFT, simUtil::ALIGN_TOP, color, font, fontSize);
    break;
  default:
    assert(0); // Unknown position
  }

  statusHudText_->setName("HUD Corner Status Text");
}

// this routine services the frame update
void StatusText::update_()
{
  std::string replacerOut;
  assert(textReplacer_);
  // only updating if there is a change in the text
  replacerOut = textReplacer_->format(statusSpec_);
  if (!replacerOut.empty() && statusHudText_.valid())
  {
    statusHudText_->setText(replacerOut);
  }
}

void StatusText::resize_(int widthPx, int heightPx)
{
  if (windowWidthPx_ == widthPx && windowHeightPx_ == heightPx)
    return;

  windowWidthPx_ = widthPx;
  windowHeightPx_ = heightPx;
  if (statusHudText_.valid())
    statusHudText_->resize(windowWidthPx_, windowHeightPx_);
}

} // namespace simUtil

