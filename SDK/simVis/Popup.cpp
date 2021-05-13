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
#include "osgEarth/Controls"
#include "osgEarth/LineDrawable"

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


/////////////////////////////////////////////////////////////////////////////////////////////////

EntityPopup2::EntityPopup2()
  : osg::MatrixTransform()
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
}

void EntityPopup2::setPosition(float xPx, float yPx)
{
  osg::Matrix mat = getMatrix();
  mat.setTrans(osg::Vec3d(xPx, yPx, 0.0));
  setMatrix(mat);
}

void EntityPopup2::setTitle(const std::string& content)
{
  titleLabel_->setText(content, osgText::String::ENCODING_UTF8);
  updateLabelPositions_();
}

void EntityPopup2::setContent(const std::string& content)
{
  contentLabel_->setText(content, osgText::String::ENCODING_UTF8);
  updateLabelPositions_();
}

EntityPopup2::~EntityPopup2()
{
}

void EntityPopup2::initGraphics_()
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

void EntityPopup2::updateLabelPositions_()
{
  const osg::BoundingBox& titleBb = titleLabel_->getBoundingBox();
  const osg::BoundingBox& contentBb = contentLabel_->getBoundingBox();

  const float titleHeight = titleBb.yMax() - titleBb.yMin();
  const float titleYPos = -DEFAULT_PADDING - titleHeight;
  titleLabel_->setPosition(osg::Vec3(DEFAULT_PADDING, titleYPos, 0));

  const float contentHeight = contentBb.yMax() - contentBb.yMin();
  const float contentYPos = titleYPos - DEFAULT_PADDING - contentHeight;
  contentLabel_->setPosition(osg::Vec3(DEFAULT_PADDING, contentYPos, 0));

  float width = simCore::sdkMax(titleBb.xMax() - titleBb.xMin(), contentBb.xMax() - contentBb.xMin());
  width += DEFAULT_PADDING * 2;

  // Three pads, on top, bottom, and in between title and content
  float height = titleHeight + contentHeight + (DEFAULT_PADDING * 3);

  // TODO: keep box from going off screen

  // Fix background verts
  (*verts_)[0].set(width, -height, 0); // bot right
  (*verts_)[1].set(width, 0, 0); // top right
  (*verts_)[2].set(0, -height, 0); // bot left
  (*verts_)[3].set(0, 0, 0); // top left
  verts_->dirty();
  background_->dirtyBound();

  outline_->clear();
  outline_->pushVertex((*verts_)[0]);
  outline_->pushVertex((*verts_)[1]);
  outline_->pushVertex((*verts_)[3]);
  outline_->pushVertex((*verts_)[2]);
  outline_->dirty();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

PopupHandler2::PopupHandler2(SceneManager* scene, View* view)
  : scenario_(scene ? scene->getScenario() : nullptr),
  view_(view),
  installed_(false)
{
  init_();
}

PopupHandler2::PopupHandler2(Picker* picker, View* view)
  : picker_(picker),
  view_(view),
  installed_(false)
{
  init_();
}

void PopupHandler2::init_()
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
  popup2_ = new simVis::EntityPopup2;
}

PopupHandler2::~PopupHandler2()
{
}

void PopupHandler2::enable(bool v)
{
  enabled_ = v;
}

void PopupHandler2::clear()
{
  if (currentEntity_.valid())
  {
    currentEntity_ = nullptr;
    if (installed_ && (view_.valid()))
    {
      view_->getOrCreateHUD()->removeChild(popup2_.get());
      installed_ = false;
    }
    entityLocatorRev_.reset();
  }
}

bool PopupHandler2::isEnabled() const
{
  return enabled_;
}

void PopupHandler2::setContentCallback(PopupContentCallback* cb)
{
  contentCallback_ = cb;
}

void PopupHandler2::setLimitVisibility(bool limit)
{
  limitVisibility_ = limit;
}

void PopupHandler2::setShowInCorner(bool showInCorner)
{
  showInCorner_ = showInCorner;
}

void PopupHandler2::setBorderWidth(int borderWidth)
{
  borderWidth_ = borderWidth;
  applySettings_();
}

void PopupHandler2::setBorderColor(const simVis::Color& color)
{
  borderColor_ = color;
  applySettings_();
}

void PopupHandler2::setBackColor(const simVis::Color& color)
{
  backColor_ = color;
  applySettings_();
}

void PopupHandler2::setTitleColor(const simVis::Color& color)
{
  titleColor_ = color;
  applySettings_();
}

void PopupHandler2::setContentColor(const simVis::Color& color)
{
  contentColor_ = color;
  applySettings_();
}

void PopupHandler2::setTitleFontSize(int size)
{
  titleFontSize_ = size;
  applySettings_();
}

void PopupHandler2::setContentFontSize(int size)
{
  contentFontSize_ = size;
  applySettings_();
}

void PopupHandler2::setPadding(int width)
{
  padding_ = width;
  applySettings_();
}

void PopupHandler2::setChildSpacing(int width)
{
  childSpacing_ = width;
  applySettings_();
}

void PopupHandler2::setDuration(int duration)
{
  duration_ = duration;
}

bool PopupHandler2::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
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

void PopupHandler2::updatePopupFromView(simVis::View* currentView)
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

  if (currentEntity_.valid())
  {
    if (!installed_)
    {
      view_->getOrCreateHUD()->addChild(popup2_.get());
      applySettings_();
      showStartTime_ = simCore::getSystemTime();
      installed_ = true;
    }

    popup2_->setTitle(currentEntity_->getEntityName(EntityNode::DISPLAY_NAME, true));

    Locator* locator = currentEntity_->getLocator();
    if (!locator->inSyncWith(entityLocatorRev_))
    {
      auto platform = dynamic_cast<simVis::PlatformNode*>(currentEntity_.get());
      // Prefer the content callback over the entity's method
      if (contentCallback_.valid() && platform != nullptr)
        popup2_->setContent(contentCallback_->createString(platform));
      else
        popup2_->setContent(currentEntity_->popupText());

      locator->sync(entityLocatorRev_);
    }

    popup2_->setPosition(lastMX_, lastMY_);

    return;
  }
}

void PopupHandler2::applySettings_()
{
  // TODO: apply configured settings to the EntityPopup2
}

/////////////////////////////////////////////////////////////////////////////////////////////////

EntityPopup::EntityPopup()
{
  setName("Entity Popup");

  titleLabel_ = new osgEarth::Util::Controls::LabelControl();
  titleLabel_->setName("EntityPopup Title");
  titleLabel_->setForeColor(DEFAULT_TITLE_COLOR);
  osgText::Font* defaultFont = simVis::Registry::instance()->getOrCreateFont("arial.ttf");
  titleLabel_->setFont(defaultFont);
  titleLabel_->setFontSize(simVis::osgFontSize(DEFAULT_TITLE_SIZE));
  titleLabel_->setEncoding(osgText::String::ENCODING_UTF8);
  this->addControl(titleLabel_);

  contentLabel_ = new osgEarth::Util::Controls::LabelControl();
  contentLabel_->setName("EntityPopup Content");
  contentLabel_->setForeColor(DEFAULT_CONTENT_COLOR);
  contentLabel_->setFont(defaultFont);
  contentLabel_->setFontSize(simVis::osgFontSize(DEFAULT_CONTENT_SIZE));
  contentLabel_->setEncoding(osgText::String::ENCODING_UTF8);
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

void EntityPopup::setTitle(const std::string& value)
{
  titleLabel_->setText(value);
}

void EntityPopup::setContent(const std::string& value)
{
  contentLabel_->setText(value);
}

osgEarth::Util::Controls::LabelControl* EntityPopup::titleLabel() const
{
  return titleLabel_;
}

osgEarth::Util::Controls::LabelControl* EntityPopup::contentLabel() const
{
  return contentLabel_;
}

// --------------------------------------------------------------------------

/** Finds the first non-nullptr instance of type T in the node path provided */
template<typename T>
T* findNodeInPath(const osg::NodePath& path)
{
  for (osg::NodePath::const_reverse_iterator i = path.rbegin(); i != path.rend(); ++i)
  {
    if (dynamic_cast<T*>(*i))
      return static_cast<T*>(*i);
  }
  return nullptr;
}

// --------------------------------------------------------------------------

PopupHandler::PopupHandler(SceneManager* scene, View* view)
  : scenario_(scene ? scene->getScenario() : nullptr),
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
  if (currentEntity_.valid())
  {
    currentEntity_ = nullptr;
    if (popup_.valid() && (view_.valid()))
    {
      view_->removeOverlayControl(popup_.get());
    }
    popup_ = nullptr;
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
    // if there is an active entity, but the new entity is different,
    // remove the old pop up.
    if (popup_.valid())
    {
      view_->removeOverlayControl(popup_.get());
    }
    popup_ = nullptr;
    currentEntity_ = entity;
    entityLocatorRev_.reset();
  }

  if (currentEntity_.valid())
  {
    // if we have an active entity, reposition the pop up, creating it if it does
    // not already exist.
    if (!popup_.valid())
    {
      popup_ = new EntityPopup();
      applySettings_();
      popup_->setTitle(currentEntity_->getEntityName(EntityNode::DISPLAY_NAME, true));
      view_->addOverlayControl(popup_.get());
      showStartTime_ = simCore::getSystemTime();
    }

    Locator* locator = currentEntity_->getLocator();
    if (!locator->inSyncWith(entityLocatorRev_))
    {
      auto platform = dynamic_cast<simVis::PlatformNode*>(currentEntity_.get());
      // Prefer the content callback over the entity's method
      if (contentCallback_.valid() && platform != nullptr)
      {
        popup_->setContent(contentCallback_->createString(platform));
      }
      else
      {
        popup_->setContent(currentEntity_->popupText());
      }

      locator->sync(entityLocatorRev_);
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
  popup_->titleLabel()->setFontSize(simVis::osgFontSize(titleFontSize_));
  popup_->contentLabel()->setForeColor(contentColor_);
  popup_->contentLabel()->setFontSize(simVis::osgFontSize(contentFontSize_));
  popup_->setPadding(padding_);
  popup_->setChildSpacing(childSpacing_);
}

}
