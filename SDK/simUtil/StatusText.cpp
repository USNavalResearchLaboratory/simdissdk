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
#include <cassert>
#include "simCore/String/TextReplacer.h"
#include "simUtil/HudManager.h"
#include "simVis/View.h"
#include "simUtil/StatusText.h"

namespace simUtil
{

StatusTextNode::StatusTextNode(simCore::TextReplacerPtr textReplacer)
  : textReplacer_(textReplacer)
{
  // Assertion failure means text replacer was not correctly set up
  assert(textReplacer_);

  // Ensure we get an update traversal
  setNumChildrenRequiringUpdateTraversal(1);
}

StatusTextNode::~StatusTextNode()
{
}

void StatusTextNode::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    update_();
  osg::MatrixTransform::traverse(nv);
}

void StatusTextNode::create_(const std::string& status, const osg::Vec4f& color, const std::string& font, double fontSize)
{
  // Create the HUD text
  statusHudText_ = new HudColumnText(0, 0);
  addChild(statusHudText_);
  statusHudText_->update(status, 0.0, 0.0, false, false, simUtil::ALIGN_LEFT, simUtil::ALIGN_BOTTOM, color, font, fontSize);
  statusHudText_->setName("HUD Corner Status Text");
}

void StatusTextNode::update_()
{
  if (statusHudText_.valid())
  {
    if (textReplacer_)
      statusHudText_->setText(textReplacer_->format(statusSpec_));
    else
      statusHudText_->setText(statusSpec_);
  }
}

int StatusTextNode::setStatusSpec(const std::string& statusSpec, const osg::Vec4f& color, double fontSize, const std::string& font)
{
  // Note, don't check statusSpec for change, because other parameters might change
  statusSpec_ = statusSpec;
  // Generate the output string for creation
  const std::string replacerOut = (textReplacer_ ? textReplacer_->format(statusSpec_) : statusSpec_);
  if (!statusHudText_.valid())
    create_(replacerOut, color, font, fontSize);
  else
    statusHudText_->update(replacerOut, statusHudText_->x(), statusHudText_->y(), statusHudText_->isPercentageX(), statusHudText_->isPercentageY(), statusHudText_->hAlignment(), statusHudText_->vAlignment(), color, font, fontSize);
  return 0;
}

//-------------------------------------------------------------------------------------------------------

/** Callback handler for frame updates */
class StatusText::FrameEventHandler : public osgGA::GUIEventHandler
{
public:
  /** Constructor */
  explicit FrameEventHandler(simUtil::StatusText* parent)
  : parent_(parent),
    widthPx_(-1),
    heightPx_(-1)
  {
  }
  /** Handles frame updates and returns false so other handlers can process as well */
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (!parent_.valid())
      return false;
    if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE)
    {
      // Tell the text to resize if the screen size has changed
      const osg::View* view = aa.asView();
      const osg::Camera* camera = (view) ? view->getCamera() : nullptr;
      const osg::Viewport* viewport = (camera) ? camera->getViewport() : nullptr;
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
  : StatusTextNode(textReplacer),
    view_(view),
    frameEventHandler_(nullptr),
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

void StatusText::removeFromView()
{
  if (view_.valid())
  {
    view_->removeEventHandler(frameEventHandler_);
    view_->getOrCreateHUD()->removeChild(this);
    removeChild(statusHudText_.get());
    statusHudText_ = nullptr;
  }
}

// column text implementation requires that all lines are specified with same number of columns
void StatusText::create_(const std::string& status, const osg::Vec4f& color, const std::string& font, double fontSize)
{
  // Completely ignore the base method for creating text
  static const double MARGIN_PERCENT = 0.5;
  if (view_.valid())
  {
    view_->addEventHandler(frameEventHandler_);
    view_->getOrCreateHUD()->addChild(this);
  }

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
