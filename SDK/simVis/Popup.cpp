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
#include <sstream>
#include <iomanip>

#include "osg/Geode"
#include "osg/NodeCallback"
#include "osg/NodeVisitor"
#include "osgDB/Registry"
#include "osgDB/ReaderWriter"
#include "osgText/Text"
#include "osgUtil/IntersectionVisitor"
#include "osgUtil/LineSegmentIntersector"
#include "osgViewer/View"
#include "osgEarthUtil/Controls"

#include "simCore/Calc/Angle.h"
#include "simCore/String/Constants.h"
#include "simCore/Time/Utils.h"

#include "simVis/Locator.h"
#include "simVis/Picker.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simVis/Popup.h"

#define LC "simVis::Popup "

// --------------------------------------------------------------------------

namespace simVis
{

static const int DEFAULT_BORDER_WIDTH = 2;
static const simVis::Color DEFAULT_BORDER_COLOR(1, 1, 0, 1);
static const simVis::Color DEFAULT_BACK_COLOR(0, 0, 0, 0.5);
static const simVis::Color DEFAULT_TITLE_COLOR(.9, .9, 0, 1);
static const simVis::Color DEFAULT_CONTENT_COLOR(.9, .9, .9, 1);
static const int DEFAULT_TITLE_SIZE = 16;
static const int DEFAULT_CONTENT_SIZE = 14;
static const int DEFAULT_PADDING = 10;
static const int DEFAULT_SPACING = 4;


PlatformPopup::PlatformPopup()
{
  setName("Platform Popup");

  titleLabel_ = new osgEarth::Util::Controls::LabelControl();
  titleLabel_->setName("PlatformPopup Title");
  titleLabel_->setForeColor(DEFAULT_TITLE_COLOR);
  osgText::Font* defaultFont = simVis::Registry::instance()->getOrCreateFont("arial.ttf");
  titleLabel_->setFont(defaultFont);
  titleLabel_->setFontSize(DEFAULT_TITLE_SIZE);
  this->addControl(titleLabel_);

  contentLabel_ = new osgEarth::Util::Controls::LabelControl();
  contentLabel_->setName("PlatformPopup Content");
  contentLabel_->setForeColor(DEFAULT_CONTENT_COLOR);
  contentLabel_->setFont(defaultFont);
  contentLabel_->setFontSize(DEFAULT_CONTENT_SIZE);
  this->addControl(contentLabel_);

  // add yellow border
  this->setBorderColor(DEFAULT_BORDER_COLOR);
  this->setBorderWidth(DEFAULT_BORDER_WIDTH);

  this->setBackColor(DEFAULT_BACK_COLOR);
  this->setPadding(DEFAULT_PADDING);
  this->setChildSpacing(DEFAULT_SPACING);
  this->setAbsorbEvents(false);

  this->getOrCreateStateSet()->setRenderBinDetails(20, "RenderBin");
}

void PlatformPopup::setTitle(const std::string& value)
{
  titleLabel_->setText(value);
}

void PlatformPopup::setContent(const std::string& value)
{
  contentLabel_->setText(value);
}

osgEarth::Util::Controls::LabelControl* PlatformPopup::titleLabel() const
{
  return titleLabel_;
}

osgEarth::Util::Controls::LabelControl* PlatformPopup::contentLabel() const
{
  return contentLabel_;
}

// --------------------------------------------------------------------------

/** Finds the first non-NULL instance of type T in the node path provided */
template<typename T>
T* findNodeInPath(const osg::NodePath& path)
{
  for (osg::NodePath::const_reverse_iterator i = path.rbegin(); i != path.rend(); ++i)
  {
    if (dynamic_cast<T*>(*i))
      return static_cast<T*>(*i);
  }
  return NULL;
}

// --------------------------------------------------------------------------

PopupHandler::PopupHandler(SceneManager* scene, View* view)
  : scenario_(scene ? scene->getScenario() : NULL),
    view_(view)
{
  init_();
}

PopupHandler::PopupHandler(Picker* picker, View* view)
  : picker_(picker),
    view_(view)
{
  init_();
}

void PopupHandler::init_()
{
  lastMX_ = 0.0f;
  lastMY_ = 0.0f;
  mouseDirty_ = false;
  enabled_ = true;
  showInCorner_ = false;
  limitVisibility_ = true;
  borderWidth_ = DEFAULT_BORDER_WIDTH;
  borderColor_ = DEFAULT_BORDER_COLOR;
  backColor_ = DEFAULT_BACK_COLOR;
  titleColor_ = DEFAULT_TITLE_COLOR;
  contentColor_ = DEFAULT_CONTENT_COLOR;
  titleFontSize_ = DEFAULT_TITLE_SIZE;
  contentFontSize_ = DEFAULT_CONTENT_SIZE;
  padding_ = DEFAULT_PADDING;
  childSpacing_ = DEFAULT_SPACING;
  duration_ = 5;
}

PopupHandler::~PopupHandler()
{
}

void PopupHandler::enable(bool v)
{
  enabled_ = v;
}

void PopupHandler::clear()
{
  if (currentPlatform_.valid())
  {
    currentPlatform_ = NULL;
    if (popup_.valid() && (view_.valid()))
    {
      view_->removeOverlayControl(popup_.get());
    }
    popup_ = NULL;
    platformLocatorRev_.reset();
  }
}

bool PopupHandler::isEnabled() const
{
  return enabled_;
}

void PopupHandler::setContentCallback(PopupContentCallback* cb)
{
  contentCallback_ = cb;
}

void PopupHandler::setLimitVisibility(bool limit)
{
  limitVisibility_ = limit;
}

void PopupHandler::setShowInCorner(bool showInCorner)
{
  showInCorner_ = showInCorner;
}

void PopupHandler::setBorderWidth(int borderWidth)
{
  borderWidth_ = borderWidth;
  applySettings_();
}

void PopupHandler::setBorderColor(const simVis::Color& color)
{
  borderColor_ = color;
  applySettings_();
}

void PopupHandler::setBackColor(const simVis::Color& color)
{
  backColor_ = color;
  applySettings_();
}

void PopupHandler::setTitleColor(const simVis::Color& color)
{
  titleColor_ = color;
  applySettings_();
}

void PopupHandler::setContentColor(const simVis::Color& color)
{
  contentColor_ = color;
  applySettings_();
}

void PopupHandler::setTitleFontSize(int size)
{
  titleFontSize_ = size;
  applySettings_();
}

void PopupHandler::setContentFontSize(int size)
{
  contentFontSize_ = size;
  applySettings_();
}

void PopupHandler::setPadding(int width)
{
  padding_ = width;
  applySettings_();
}

void PopupHandler::setChildSpacing(int width)
{
  childSpacing_ = width;
  applySettings_();
}

void PopupHandler::setDuration(int duration)
{
  duration_ = duration;
}

bool PopupHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
  if (!enabled_)
    return false;

  // This only fires for the view associated with addEventHandler()
  if (ea.getEventType() == ea.MOVE)
  {
    lastMX_ = ea.getX();
    lastMY_ = ea.getY();
    mouseDirty_ = true;
    aa.requestRedraw();
  }

  if (ea.getEventType() == ea.FRAME)
  {
    // If you're using this with insets, you may need to artificially trigger
    // handle() calls on MOVE events in other insets to get the mouse to time out.

    // In the case of not limiting visibility, and if we're using the RTT picker code
    // (which has better performance), AND if we're showing in the corner (don't need
    // mouse coords), then always dirty the mouse.  This helps SDK examples.
    if (!limitVisibility_ && showInCorner_ && picker_.valid())
      mouseDirty_ = true;

    osg::observer_ptr<View> currentView = static_cast<View*>(aa.asView());
    updatePopupFromView(currentView.get());
  }

  return false;
}

void PopupHandler::updatePopupFromView(simVis::View* currentView)
{
  if (limitVisibility_ && popup_.valid() && !mouseDirty_)
  {
    double curTime = simCore::getSystemTime();
    if (curTime - showStartTime_ > static_cast<double>(duration_))
    {
      clear();
      return;
    }
  }

  // only create a pop up if the user moves the mouse (not if something wanders
  // into the path of the mouse pointer).
  if (!popup_.valid() && !mouseDirty_)
    return;

  mouseDirty_ = false;

  // get the interface to this particular view if view is not valid
  if (!view_.valid())
    view_ = currentView;

  // get a safe handler on the observer
  osg::ref_ptr<PlatformNode> platform;
  osg::ref_ptr<Picker> picker;
  if (picker_.lock(picker))
    platform = picker_->pickedPlatform();
  else
  {
    osg::ref_ptr<ScenarioManager> scenarioSafe;
    // intersect the scenario graph, looking for PlatformModelNodes, need to also traverse PlatformNode to get to PlatformModelNode
    if (scenario_.lock(scenarioSafe))
      platform = scenarioSafe->find<PlatformNode>(currentView, lastMX_, lastMY_, PlatformNode::getMask() | PlatformModelNode::getMask());
  }

  if (!platform)
  {
    clear();
    return;
  }

  if (!currentPlatform_.valid())
  {
    // if there is no current platform, assign one.
    currentPlatform_ = platform;
    platformLocatorRev_.reset();
  }

  else if (currentPlatform_.valid() && platform != currentPlatform_.get())
  {
    // if there is an active platform, but the new platform is different,
    // remove the old pop up.
    if (popup_.valid())
    {
      view_->removeOverlayControl(popup_.get());
    }
    popup_ = NULL;
    currentPlatform_ = platform;
    platformLocatorRev_.reset();
  }

  if (currentPlatform_.valid())
  {
    // if we have an active platform, reposition the pop up, creating it if it does
    // not already exist.
    if (!popup_.valid())
    {
      popup_ = new PlatformPopup();
      applySettings_();
      popup_->setTitle(currentPlatform_->getEntityName(EntityNode::DISPLAY_NAME, true));
      view_->addOverlayControl(popup_.get());
      showStartTime_ = simCore::getSystemTime();
    }

    Locator* locator = currentPlatform_->getLocator();
    if (!locator->inSyncWith(platformLocatorRev_))
    {
      if (contentCallback_.valid())
      {
        popup_->setContent(contentCallback_->createString(currentPlatform_.get()));
      }
      else
      {
        popup_->setContent(currentPlatform_->popupText());
      }

      locator->sync(platformLocatorRev_);
    }

    // if using main view, then show popup on lower right, otherwise use mouse position
    if (showInCorner_)
    {
      popup_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
      popup_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
      popup_->setX(0.0f);
      popup_->setY(0.0f);
      popup_->setMargin(osgEarth::Util::Controls::Control::SIDE_BOTTOM, 10.0f);
      popup_->setMargin(osgEarth::Util::Controls::Control::SIDE_RIGHT, 10.0f);
    }
    else
    {
      // Calculate size of popup
      osg::Vec2f popupSize;
      osgEarth::Util::Controls::ControlContext cx;
      popup_->calcSize(cx, popupSize);
      const osg::Viewport* viewport = view_->getCamera()->getViewport();
      // Constants measured in pixels with the origin at the top left
      const float buffer = 20.f;
      // Farthest right the popup can go
      const float maxX = viewport->width() - buffer - popupSize[0];
      // Farthest down the popup can go
      const float maxY = viewport->height() - buffer - popupSize[1];
      // Left edge of the popup if it is not too close to the left side of the screen
      const float x = (lastMX_ > buffer) ? lastMX_ : buffer;
      // Bottom of the popup if it is not too close to the bottom of the screen
      const float y = (viewport->height() - lastMY_ > buffer) ? viewport->height() - lastMY_ : buffer;
      popup_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_LEFT);
      popup_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_TOP);
      popup_->setX((x < maxX) ? x : maxX);
      popup_->setY((y < maxY) ? y : maxY);
      popup_->setMargin(osgEarth::Util::Controls::Control::SIDE_BOTTOM, 0.0f);
      popup_->setMargin(osgEarth::Util::Controls::Control::SIDE_RIGHT, 0.0f);
    }
  }
}

void PopupHandler::applySettings_()
{
  if (!popup_.valid())
    return;
  popup_->setBorderWidth(borderWidth_);
  popup_->setBorderColor(borderColor_);
  popup_->setBackColor(backColor_);
  popup_->titleLabel()->setForeColor(titleColor_);
  popup_->titleLabel()->setFontSize(titleFontSize_);
  popup_->contentLabel()->setForeColor(contentColor_);
  popup_->contentLabel()->setFontSize(contentFontSize_);
  popup_->setPadding(padding_);
  popup_->setChildSpacing(childSpacing_);
}

}
