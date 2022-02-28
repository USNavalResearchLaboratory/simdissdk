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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
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
#include "osg/PolygonMode"
#include "osgDB/Registry"
#include "osgDB/ReaderWriter"
#include "osgText/Text"
#include "osgUtil/IntersectionVisitor"
#include "osgUtil/LineSegmentIntersector"
#include "osgViewer/View"
#include "simCore/Calc/Angle.h"
#include "simCore/String/Constants.h"
#include "simCore/Time/Utils.h"
#include "simVis/CustomRendering.h"
#include "simVis/Locator.h"
#include "simVis/Picker.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simVis/Popup.h"

#undef LC
#define LC "simVis::Popup "

// --------------------------------------------------------------------------

namespace simVis
{

static const int DEFAULT_BORDER_WIDTH = 2;
static const simVis::Color DEFAULT_BORDER_COLOR(1, 1, 0, 1); // yellow
static const simVis::Color DEFAULT_BACK_COLOR(0, 0, 0, 0.5); // semi-transparent black
static const simVis::Color DEFAULT_TITLE_COLOR(.9, .9, 0, 1); // yellow
static const simVis::Color DEFAULT_CONTENT_COLOR(.9, .9, .9, 1); // white
static const int DEFAULT_TITLE_SIZE = 13;
static const int DEFAULT_CONTENT_SIZE = 11;
static const int DEFAULT_PADDING = 10;
static const int DEFAULT_SPACING = 4;
static const std::string DEFAULT_FONT = "arial.ttf";
static const float BUFFER_PX = 20.f; // Minimum buffer between edge of screen and popup


/////////////////////////////////////////////////////////////////////////////////////////////////

EntityPopup::WindowResizeHandler::WindowResizeHandler(EntityPopup* parent)
  : windowSize_(0.0, 0.0),
  parent_(parent)
{
}

bool EntityPopup::WindowResizeHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*)
{
  // RESIZE does not always emit correctly, especially starting in full screen mode, so use FRAME and always check size
  if (ea.getEventType() != osgGA::GUIEventAdapter::FRAME)
    return false;

  // Cannot rely on getWindowWidth(), need to check viewport
  const osg::View* view = aa.asView();
  if (!view || !view->getCamera() || !view->getCamera()->getViewport())
    return false;
  // Pull the width and height out of the viewport
  const osg::Viewport* vp = view->getCamera()->getViewport();
  const osg::Vec2f newSize(vp->width(), vp->height());
  if (newSize == windowSize_)
    return false;
  windowSize_ = newSize;

  // Get a hard lock on the parent
  osg::ref_ptr<EntityPopup> parent;
  // Update parent location if showing in corner
  if (parent_.lock(parent) && parent_->showInCorner_)
    parent_->positionInCorner_();

  return false;
}

osg::Vec2f EntityPopup::WindowResizeHandler::windowSize() const
{
  return windowSize_;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

EntityPopup::EntityPopup()
  : osg::MatrixTransform(),
  paddingPx_(DEFAULT_PADDING),
  spacingPx_(DEFAULT_SPACING),
  widthPx_(0.f),
  heightPx_(0.f),
  showInCorner_(false)
{
  setDataVariance(osg::Object::DYNAMIC);
  initGraphics_();

  titleLabel_ = new osgText::Text;
  titleLabel_->setDataVariance(osg::Object::DYNAMIC);
  titleLabel_->setName("EntityPopup Title");
  titleLabel_->setColor(DEFAULT_TITLE_COLOR);
  titleLabel_->setFont(DEFAULT_FONT);
  titleLabel_->setCharacterSize(simVis::osgFontSize(DEFAULT_TITLE_SIZE));
  titleLabel_->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
  addChild(titleLabel_);

  contentLabel_ = new osgText::Text;
  contentLabel_->setDataVariance(osg::Object::DYNAMIC);
  contentLabel_->setName("EntityPopup Content");
  contentLabel_->setColor(DEFAULT_CONTENT_COLOR);
  contentLabel_->setFont(DEFAULT_FONT);
  contentLabel_->setCharacterSize(simVis::osgFontSize(DEFAULT_CONTENT_SIZE));
  contentLabel_->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
  addChild(contentLabel_);

  // Set stateset values for the background box: Fill front-face, blend
  osg::StateSet* stateSet = getOrCreateStateSet();
  stateSet->setAttributeAndModes(new osg::PolygonMode(osg::PolygonMode::FRONT, osg::PolygonMode::FILL));
  stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);

  resizeHandler_ = new WindowResizeHandler(this);
  addEventCallback(resizeHandler_.get());
}

EntityPopup::~EntityPopup()
{
}

void EntityPopup::setPosition(float xPx, float yPx)
{
  if (showInCorner_)
    return;

  // Keep box from going off screen
  const auto& windowSize = resizeHandler_->windowSize();

  // Farthest right the popup can go
  const float maxX = windowSize.x() - BUFFER_PX - widthPx_;
  // Farthest up the popup can go
  const float maxY = windowSize.y() - BUFFER_PX;
  // Limit the X position to keep the entirety of the popup on screen horizontally
  const float x = simCore::sdkMin(simCore::sdkMax(xPx, BUFFER_PX), maxX);
  // Limit the Y position to keep the entirety of the popup on screen vertically
  const float y = simCore::sdkMin(simCore::sdkMax(yPx, BUFFER_PX + heightPx_), maxY);

  osg::Matrix mat = getMatrix();
  mat.setTrans(osg::Vec3d(x, y, 0.0));
  setMatrix(mat);
}

void EntityPopup::setTitle(const std::string& content)
{
  titleLabel_->setText(content, osgText::String::ENCODING_UTF8);
  updateLabelPositions_();
}

void EntityPopup::setContent(const std::string& content)
{
  contentLabel_->setText(content, osgText::String::ENCODING_UTF8);
  updateLabelPositions_();
}

osgText::Text* EntityPopup::titleLabel() const
{
  return titleLabel_.get();
}

osgText::Text* EntityPopup::contentLabel() const
{
  return contentLabel_.get();
}

void EntityPopup::setBorderWidth(float borderWidth)
{
  outline_->setLineWidth(borderWidth);
}

void EntityPopup::setBorderColor(const simVis::Color& color)
{
  outline_->setColor(color);
}

void EntityPopup::setBackgroundColor(const simVis::Color& color)
{
  osg::Vec4Array* backgroundColor = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  backgroundColor->push_back(color);
  background_->setColorArray(backgroundColor);
}

void EntityPopup::setPadding(int width)
{
  if (paddingPx_ == width)
    return;
  paddingPx_ = width;
  updateLabelPositions_();
}

void EntityPopup::setChildSpacing(int width)
{
  if (spacingPx_ == width)
    return;
  spacingPx_ = width;
  updateLabelPositions_();
}

void EntityPopup::setShowInCorner(bool showInCorner)
{
  if (showInCorner_ == showInCorner)
    return;
  showInCorner_ = showInCorner;
  // Note: turning off showInCorner_ will require mouse movement to correctly position
  positionInCorner_();
}

void EntityPopup::initGraphics_()
{
  // Set up vertices
  verts_ = new osg::Vec3Array();
  verts_->setDataVariance(osg::Object::DYNAMIC);
  verts_->push_back(osg::Vec3());
  verts_->push_back(osg::Vec3());
  verts_->push_back(osg::Vec3());
  verts_->push_back(osg::Vec3());
  verts_->dirty();

  // Create background geometry
  background_ = new osg::Geometry;
  background_->setName("EntityPopup Background");
  background_->setDataVariance(osg::Object::DYNAMIC);
  background_->setVertexArray(verts_.get());
  background_->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, verts_->size()));
  osg::Vec4Array* backgroundColor = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  backgroundColor->push_back(DEFAULT_BACK_COLOR);
  background_->setColorArray(backgroundColor);

  // Create outline geometry
  outline_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
  outline_->setDataVariance(osg::Object::DYNAMIC);
  outline_->setLineWidth(DEFAULT_BORDER_WIDTH);
  outline_->setColor(DEFAULT_BORDER_COLOR);

  addChild(background_.get());
  addChild(outline_.get());
}

void EntityPopup::updateLabelPositions_()
{
  const osg::BoundingBox& titleBb = titleLabel_->getBoundingBox();
  const osg::BoundingBox& contentBb = contentLabel_->getBoundingBox();

  const float titleHeight = titleBb.yMax() - titleBb.yMin();
  const float titleYPos = -paddingPx_ - titleHeight;
  titleLabel_->setPosition(osg::Vec3(paddingPx_, titleYPos, 0));

  const float contentHeight = contentBb.yMax() - contentBb.yMin();
  const float contentYPos = titleYPos - spacingPx_ - contentHeight;
  contentLabel_->setPosition(osg::Vec3(paddingPx_, contentYPos, 0));

  widthPx_ = simCore::sdkMax(titleBb.xMax() - titleBb.xMin(), contentBb.xMax() - contentBb.xMin());
  widthPx_ += paddingPx_ * 2;

  // Two pads (top and bottom) and spacing between title and content
  heightPx_ = titleHeight + contentHeight + (paddingPx_ * 2) + spacingPx_;

  // Fix background verts
  (*verts_)[0].set(widthPx_, -heightPx_, 0); // bot right
  (*verts_)[1].set(widthPx_, 0, 0); // top right
  (*verts_)[2].set(0, -heightPx_, 0); // bot left
  (*verts_)[3].set(0, 0, 0); // top left
  verts_->dirty();
  background_->dirtyBound();

  outline_->clear();
  outline_->pushVertex((*verts_)[0]);
  outline_->pushVertex((*verts_)[1]);
  outline_->pushVertex((*verts_)[3]);
  outline_->pushVertex((*verts_)[2]);
  outline_->dirty();

  // Fix the position in the corner to account for the newly changed sizes
  if (showInCorner_)
    positionInCorner_();
}

void EntityPopup::positionInCorner_()
{
  const float xPos = resizeHandler_->windowSize().x() - BUFFER_PX - widthPx_;
  const float yPos = BUFFER_PX + heightPx_;
  osg::Matrix mat = getMatrix();
  mat.setTrans(osg::Vec3d(xPos, yPos, 0.0));
  setMatrix(mat);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

PopupHandler::PopupHandler(SceneManager* scene, View* view)
  : scenario_(scene ? scene->getScenario() : nullptr),
  view_(view),
  installed_(false)
{
  init_();
}

PopupHandler::PopupHandler(Picker* picker, View* view)
  : picker_(picker),
  view_(view),
  installed_(false)
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
  popup_ = new simVis::EntityPopup;
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
  if (currentEntity_.valid())
  {
    currentEntity_ = nullptr;
    if (installed_ && (view_.valid()))
    {
      view_->getOrCreateHUD()->removeChild(popup_.get());
      installed_ = false;
    }
    entityLocatorRev_.reset();
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
  popup_->setShowInCorner(showInCorner_);
}

void PopupHandler::setBorderWidth(int borderWidth)
{
  if (borderWidth_ == borderWidth)
    return;
  borderWidth_ = borderWidth;
  applySettings_();
}

void PopupHandler::setBorderColor(const simVis::Color& color)
{
  if (borderColor_ == color)
    return;
  borderColor_ = color;
  applySettings_();
}

void PopupHandler::setBackColor(const simVis::Color& color)
{
  if (backColor_ == color)
    return;
  backColor_ = color;
  applySettings_();
}

void PopupHandler::setTitleColor(const simVis::Color& color)
{
  if (titleColor_ == color)
    return;
  titleColor_ = color;
  applySettings_();
}

void PopupHandler::setContentColor(const simVis::Color& color)
{
  if (contentColor_ == color)
    return;
  contentColor_ = color;
  applySettings_();
}

void PopupHandler::setTitleFontSize(int size)
{
  if (titleFontSize_ == size)
    return;
  titleFontSize_ = size;
  applySettings_();
}

void PopupHandler::setContentFontSize(int size)
{
  if (contentFontSize_ == size)
    return;
  contentFontSize_ = size;
  applySettings_();
}

void PopupHandler::setPadding(int width)
{
  if (padding_ == width)
    return;
  padding_ = width;
  applySettings_();
}

void PopupHandler::setChildSpacing(int width)
{
  if (childSpacing_ == width)
    return;
  childSpacing_ = width;
  applySettings_();
}

void PopupHandler::setDuration(int duration)
{
  duration_ = duration;
}

bool PopupHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
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
  if (limitVisibility_ && installed_ && !mouseDirty_)
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
  if (!installed_ && !mouseDirty_)
    return;

  mouseDirty_ = false;

  // get the interface to this particular view if view is not valid
  if (!view_.valid())
    view_ = currentView;

  // get a safe handler on the observer
  osg::ref_ptr<EntityNode> entity;
  osg::ref_ptr<Picker> picker;
  if (picker_.lock(picker))
    entity = picker_->pickedEntity();
  else
  {
    osg::ref_ptr<ScenarioManager> scenarioSafe;
    // intersect the scenario graph, looking for PlatformModelNodes, need to also traverse PlatformNode to get to PlatformModelNode
    if (scenario_.lock(scenarioSafe))
      entity = scenarioSafe->find<PlatformNode>(currentView, lastMX_, lastMY_, PlatformNode::getMask() | PlatformModelNode::getMask());
  }

  if (!entity)
  {
    clear();
    return;
  }

  if (!currentEntity_.valid())
  {
    // if there is no current entity, assign one.
    currentEntity_ = entity;
    entityLocatorRev_.reset();
  }

  else if (currentEntity_.valid() && entity != currentEntity_.get())
  {
    currentEntity_ = entity;
    entityLocatorRev_.reset();
  }

  if (!currentEntity_.valid())
    return;

  if (!installed_)
  {
    view_->getOrCreateHUD()->addChild(popup_.get());
    applySettings_();
    showStartTime_ = simCore::getSystemTime();
    installed_ = true;
  }

  popup_->setTitle(currentEntity_->getEntityName(EntityNode::DISPLAY_NAME, true));

  Locator* locator = currentEntity_->getLocator();
  if (!locator->inSyncWith(entityLocatorRev_))
  {
    auto platform = dynamic_cast<simVis::PlatformNode*>(currentEntity_.get());
    // Prefer the content callback over the entity's method
    if (contentCallback_.valid() && platform != nullptr)
      popup_->setContent(contentCallback_->createString(platform));
    else
      popup_->setContent(currentEntity_->popupText());

    locator->sync(entityLocatorRev_);
  }

  if (!showInCorner_)
    popup_->setPosition(lastMX_, lastMY_);
}

void PopupHandler::applySettings_()
{
  if (!popup_.valid() || !popup_->titleLabel() || !popup_->contentLabel())
    return;
  popup_->setBorderWidth(borderWidth_);
  popup_->setBorderColor(borderColor_);
  popup_->setBackgroundColor(backColor_);
  popup_->titleLabel()->setColor(titleColor_);
  popup_->titleLabel()->setCharacterSize(simVis::osgFontSize(titleFontSize_));
  popup_->contentLabel()->setColor(contentColor_);
  popup_->contentLabel()->setCharacterSize(simVis::osgFontSize(contentFontSize_));
  popup_->setPadding(padding_);
  popup_->setChildSpacing(childSpacing_);
}

}
