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
#include "simVis/View.h"
#include "simCore/String/TextReplacer.h"
#include "simUtil/HudManager.h"
#include "simUtil/StatusText.h"

namespace simUtil
{

/**
 * Callback handler for frame updates, skips every other frame update
 */
class StatusText::FrameEventHandler : public osgGA::GUIEventHandler
{
public:
  /** Constructor */
  explicit FrameEventHandler(simUtil::StatusText* parent) : parent_(parent)
  {}
  /// Handles frame updates and returns false so other handlers can process as well
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
    {
      parent_->update_();
    }
    return false;
  }
  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }
  /** Return the class name */
  virtual const char* className() const { return "StatusText::FrameEventHandler"; }

protected:
  virtual ~FrameEventHandler(){}
private:
  simUtil::StatusText* parent_;
};


//-------------------------------------------------------------------------------------------------------
StatusText::StatusText(simVis::View* view, simCore::TextReplacerPtr textReplacer, Position pos)
  :view_(view),
  hudManager_(NULL),
  updateEventHandler_(NULL),
  textReplacer_(textReplacer),
  position_(pos)
{
  hudManager_ = new simUtil::HudManager(view);
  updateEventHandler_ = new FrameEventHandler(this);
}

StatusText::~StatusText()
{
  removeFromView();
  delete hudManager_;
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
    {
      create_(replacerOut, color, font, fontSize);
    }
    else
    {
      statusHudText_->update(replacerOut, statusHudText_->x(), statusHudText_->y(), statusHudText_->isPercentageX(), statusHudText_->isPercentageY(), statusHudText_->hAlignment(), statusHudText_->vAlignment(), color, font, fontSize);
    }
  }
  else if (statusSpec.empty() && statusHudText_.valid())
  {
    removeFromView();
  }
  return 0;
}

void StatusText::removeFromView()
{
  if (view_.valid())
  {
    view_.get()->removeEventHandler(updateEventHandler_);
    if (hudManager_ && statusHudText_.valid())
    {
      hudManager_->removeText(statusHudText_.get());
    }
  }
}

// column text implementation requires that all lines are specified with same number of columns
void StatusText::create_(const std::string& status, const osg::Vec4f& color, const std::string& font, double fontSize)
{
  static const double MARGIN_PERCENT = 0.5;
  view_.get()->addEventHandler(updateEventHandler_);

  // each StatusText::Position is implemented using relative/percent positioning
  switch (position_)
  {
  case simUtil::StatusText::LEFT_BOTTOM:
    // ALIGN_BOTTOM positions text at absolute bottom, MARGIN_PERCENT adds a little margin
    statusHudText_ = hudManager_->createColumnText(status, MARGIN_PERCENT, MARGIN_PERCENT, true, simUtil::ALIGN_BOTTOM, color, font, fontSize);
    break;
  case simUtil::StatusText::LEFT_CENTER:
    statusHudText_ = hudManager_->createColumnText(status, MARGIN_PERCENT, 50.0, true, simUtil::ALIGN_CENTER_Y, color, font, fontSize);
    break;
  case simUtil::StatusText::LEFT_TOP:
    statusHudText_ = hudManager_->createColumnText(status, MARGIN_PERCENT, 100.0-MARGIN_PERCENT, true, simUtil::ALIGN_TOP, color, font, fontSize);
    break;
  default:
    assert(0);
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


} // namespace simUtil

