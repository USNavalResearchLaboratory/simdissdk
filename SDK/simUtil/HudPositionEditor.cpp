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
#include <cassert>
#include "osg/Depth"
#include "osg/MatrixTransform"
#include "osgGA/EventVisitor"
#include "osgText/Text"
#include "osgEarth/LineDrawable"
#include "osgEarth/NodeUtils"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simUtil/HudPositionManager.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/HudPositionEditor.h"

namespace simUtil {

/** Color to use for screen dimmining background window */
static const osg::Vec4f SCREEN_BG_DIM_COLOR(0.f, 0.f, 0.f, 0.5f);

/** Width of the outline around a window */
static const float OUTLINE_WIDTH = 3.f;
/** Color of the outline when not selected */
static const osg::Vec4f OUTLINE_DEFAULT_COLOR = simVis::Color::Gray;
/** Stipple of the outline when not selected */
static const GLushort OUTLINE_DEFAULT_STIPPLE = 0xf0f0;
/** Color of the outline when selected */
static const osg::Vec4f OUTLINE_SELECTED_COLOR = simVis::Color::Lime;
/** Stipple of the outline when selected */
static const GLushort OUTLINE_SELECTED_STIPPLE = 0xffff;
/** Factor to apply to stipple of stippled outlines. */
static const GLint OUTLINE_STIPPLE_FACTOR = 3;

/** Pixels of padding between the edge of the window and the anchor. */
static const double BOX_PADDING = 4.0;
/** Background color of a window */
static const osg::Vec4f WINDOW_BG_COLOR(1.f, 1.f, 1.f, 0.25f);
/** Color of the anchor */
static const osg::Vec4f ANCHOR_DIAMOND_COLOR(0.8f, 0.8f, 0.f, 1.f);

/** Font name for the title text */
static const std::string TITLE_FONT = "arialbd.ttf";
/** Size of the title text */
static const float TITLE_POINTSIZE = simVis::osgFontSize(16.f);
/** Color for the title text */
static const osg::Vec4f TITLE_COLOR = simVis::Color::White;

/** Half of the width of the anchor diamond, in pixels */
static const float ANCHOR_HALF_WIDTH = 6.f;

/**
 * Helper class that represents a single window on the screen.  It includes on-screen the
 * bounding box area for the window and the window's name.  It presumes it is working in
 * pixels.  The bounding box is available as well.  (0,0) is the anchor position for the window.
 */
class WindowNodePx : public osg::MatrixTransform
{
public:
  /** Creates the window with the given name and the min/max XYZ values. */
  WindowNodePx(const std::string& name, const osg::Vec3d& minXyz, const osg::Vec3d& maxXyz);

  /** Changes the size of the window frame */
  void updateSize(const osg::Vec3d& minXyz, const osg::Vec3d& maxXyz);

  /**
   * Retrieves the bounding box in window coordinates of the entire area.  This is expanded slightly for
   * buffer and for the control point graphic.
   */
  const osg::BoundingBoxd& boundingBoxPx() const;
  /** Marks the window as "selected", drawing the outline solid and green. */
  void setSelected(bool selected);

protected:
  /** Avoid osg::Referenced double delete problem */
  virtual ~WindowNodePx() {}

private:
  /** Presumes that box is oriented on the Z plane.  Draws the filled background. */
  osg::Geometry* filledBox_(const osg::Vec3d& min, const osg::Vec3d& max, const osg::Vec4f& color) const;
  /** Draws the control point as a filled diamond. */
  osg::Geometry* diamond_(float halfWidth, const osg::Vec4f& color) const;

  /** Removes all children and recreates the geometry based on new size. */
  void recreateGeometry_(const std::string& name, const osg::Vec3d& minXyz, const osg::Vec3d& maxXyz);

  osg::Vec3d minXyz_;
  osg::Vec3d maxXyz_;
  osg::ref_ptr<osgEarth::LineDrawable> outline_;
  osg::BoundingBoxd box_;
  bool selected_;
};

/////////////////////////////////////////////////////////

WindowNodePx::WindowNodePx(const std::string& name, const osg::Vec3d& minXyz, const osg::Vec3d& maxXyz)
  : selected_(false)
{
  // Use osg::Node::setName() for storing the window's name
  setName(name);
  recreateGeometry_(name, minXyz, maxXyz);
}

void WindowNodePx::recreateGeometry_(const std::string& name, const osg::Vec3d& minXyz, const osg::Vec3d& maxXyz)
{
  // Refuse to recreate if all parameters match
  if (minXyz_ == minXyz && maxXyz_ == maxXyz)
    return;

  removeChildren(0, getNumChildren());

  box_.init();
  box_.expandBy(minXyz - osg::Vec3d(BOX_PADDING, BOX_PADDING, 0.));
  box_.expandBy(maxXyz + osg::Vec3d(BOX_PADDING, BOX_PADDING, 0.));
  box_.expandBy(-ANCHOR_HALF_WIDTH - BOX_PADDING, -ANCHOR_HALF_WIDTH - BOX_PADDING, 0.);
  box_.expandBy(ANCHOR_HALF_WIDTH + BOX_PADDING, ANCHOR_HALF_WIDTH + BOX_PADDING, 0.);
  box_.expandBy(-ANCHOR_HALF_WIDTH - BOX_PADDING, ANCHOR_HALF_WIDTH + BOX_PADDING, 0.);
  box_.expandBy(ANCHOR_HALF_WIDTH + BOX_PADDING, -ANCHOR_HALF_WIDTH - BOX_PADDING, 0.);

  // Draw the background, then draw the anchor
  addChild(filledBox_(box_.corner(0), box_.corner(3), WINDOW_BG_COLOR));
  addChild(diamond_(ANCHOR_HALF_WIDTH, ANCHOR_DIAMOND_COLOR));

  // Text draws on top
  osg::ref_ptr<osgText::Text> windowNameText = new osgText::Text;
  windowNameText->setText(name);
  windowNameText->setAlignment(osgText::TextBase::CENTER_CENTER);
  windowNameText->setAxisAlignment(osgText::TextBase::SCREEN);
  windowNameText->setAutoRotateToScreen(true);
  windowNameText->setCharacterSize(TITLE_POINTSIZE);
  windowNameText->setColor(TITLE_COLOR);
  windowNameText->setFont(TITLE_FONT);
  windowNameText->setBackdropColor(osg::Vec4f(0.f, 0.f, 0.f, 1.f));
  windowNameText->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
  windowNameText->setPosition(box_.center());
  addChild(windowNameText.get());

  // Draw the outline on top of the text
  outline_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
  outline_->setLineWidth(OUTLINE_WIDTH);
  outline_->setColor(OUTLINE_DEFAULT_COLOR);
  outline_->setStippleFactor(OUTLINE_STIPPLE_FACTOR);
  outline_->pushVertex(box_.corner(0));
  outline_->pushVertex(box_.corner(1));
  outline_->pushVertex(box_.corner(3));
  outline_->pushVertex(box_.corner(2));
  outline_->setDataVariance(osg::Object::DYNAMIC);
  outline_->finish();
  addChild(outline_.get());

  // Initialize the selection graphics
  setSelected(selected_);

  minXyz_ = minXyz;
  maxXyz_ = maxXyz;
}

void WindowNodePx::updateSize(const osg::Vec3d& minXyz, const osg::Vec3d& maxXyz)
{
  recreateGeometry_(getName(), minXyz, maxXyz);
}

const osg::BoundingBoxd& WindowNodePx::boundingBoxPx() const
{
  return box_;
}

void WindowNodePx::setSelected(bool selected)
{
  // Cache the state of selected for recreation of geometry later, but
  // do not bother testing for changes here because the logic gets too
  // complex on construction, because "not selected" state for graphics
  // includes non-default stipple/color.
  selected_ = selected;
  if (!outline_)
    return;
  if (selected)
  {
    outline_->setStipplePattern(OUTLINE_SELECTED_STIPPLE);
    outline_->setColor(OUTLINE_SELECTED_COLOR);
  }
  else
  {
    outline_->setStipplePattern(OUTLINE_DEFAULT_STIPPLE);
    outline_->setColor(OUTLINE_DEFAULT_COLOR);
  }
}

osg::Geometry* WindowNodePx::filledBox_(const osg::Vec3d& min, const osg::Vec3d& max, const osg::Vec4f& color) const
{
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  osg::Vec4Array* colors = new osg::Vec4Array;
  colors->push_back(color);
  colors->setBinding(osg::Array::BIND_OVERALL);
  geom->setColorArray(colors);
  osg::Vec3Array* verts = new osg::Vec3Array;
  verts->push_back(min);
  verts->push_back(osg::Vec3f(max.x(), min.y(), min.z()));
  verts->push_back(max);
  verts->push_back(osg::Vec3f(min.x(), max.y(), max.z()));
  geom->setVertexArray(verts);
  geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
  return geom.release();
}

osg::Geometry* WindowNodePx::diamond_(float halfWidth, const osg::Vec4f& color) const
{
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  osg::Vec4Array* colors = new osg::Vec4Array;
  colors->push_back(color);
  colors->setBinding(osg::Array::BIND_OVERALL);
  geom->setColorArray(colors);
  osg::Vec3Array* verts = new osg::Vec3Array;
  verts->push_back(osg::Vec3f(0.f, -halfWidth, 0.f));
  verts->push_back(osg::Vec3f(halfWidth, 0.f, 0.f));
  verts->push_back(osg::Vec3f(0.f, halfWidth, 0.f));
  verts->push_back(osg::Vec3f(-halfWidth, 0.f, 0.f));
  geom->setVertexArray(verts);
  geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
  return geom.release();
}

/////////////////////////////////////////////////////////

HudEditorGui::HudEditorGui(simUtil::HudPositionManager* hud)
  : hud_(hud),
    root_(new osg::Group),
    background_(new osg::MatrixTransform),
    widthPx_(1.),
    heightPx_(1.)
{
  setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  setViewMatrix(osg::Matrix::identity());
  setProjectionMatrixAsOrtho2D(0.0, widthPx_ - 1.0, 0.0, heightPx_ - 1.0);
  setClearMask(GL_DEPTH_BUFFER_BIT);
  // Always draw last
  setRenderOrder(osg::Camera::POST_RENDER, std::numeric_limits<int>::max());

  // Setup good HUD stateset
  osg::StateSet* ss = getOrCreateStateSet();
  ss->setMode(GL_BLEND, osg::StateAttribute::ON);
  simVis::setLighting(ss, osg::StateAttribute::OFF);
  ss->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false));
  ss->setRenderBinDetails(0, "TraversalOrderBin");

  // Create the background, which dims the main view
  {
    osg::ref_ptr<osg::Geometry> bgGeom = new osg::Geometry;
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(SCREEN_BG_DIM_COLOR);
    colors->setBinding(osg::Array::BIND_OVERALL);
    bgGeom->setColorArray(colors);
    osg::Vec3Array* verts = new osg::Vec3Array;
    verts->push_back(osg::Vec3f(0.f, 0.f, -1.f));
    verts->push_back(osg::Vec3f(1.f, 0.f, -1.f));
    verts->push_back(osg::Vec3f(1.f, 1.f, -1.f));
    verts->push_back(osg::Vec3f(0.f, 1.f, -1.f));
    bgGeom->setVertexArray(verts);
    bgGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
    background_->addChild(bgGeom);
  }

  addChild(background_.get());
  addChild(root_.get());
  // Add in all the windows
  reset();

  // register for event traversals to catch screen resizes
  ADJUST_EVENT_TRAV_COUNT(this, 1);
}

HudEditorGui::~HudEditorGui()
{
}

void HudEditorGui::reset()
{
  // Remove leftover windows
  root_->removeChildren(0, root_->getNumChildren());
  windows_.clear();

  // Need a valid HUD Position Manager
  osg::ref_ptr<simUtil::HudPositionManager> hud;
  if (!hud_.lock(hud))
    return;

  // Create all the sub-windows
  std::vector<std::string> names;
  hud->getAllWindows(names, true);
  for (auto i = names.begin(); i != names.end(); ++i)
    updatePosition(*i);
}

void HudEditorGui::updatePosition(const std::string& windowName)
{
  osg::ref_ptr<simUtil::HudPositionManager> hud;
  if (!hud_.lock(hud))
    return;

  // Get or create the window pointer
  auto i = windows_.find(windowName);
  WindowNodePx* window = nullptr;
  if (i == windows_.end())
  {
    // Pull out size parameters; they're required on creation
    osg::Vec2d minXyPx;
    osg::Vec2d maxXyPx;
    // This can only fail if we don't have a window
    if (hud->getSize(windowName, minXyPx, maxXyPx) != 0)
      return;

    // Convert to pixels, then create and save the window for later
    const osg::Vec3d minXyz(minXyPx, 0.);
    const osg::Vec3d maxXyz(maxXyPx, 0.);
    window = new WindowNodePx(windowName, minXyz, maxXyz);
    root_->addChild(window);
    windows_[windowName] = window;
  }
  else
    window = i->second.get();

  // Should not be possible to be nullptr here
  assert(window);
  if (!window)
    return;

  // Pull out the position from HUD manager and move our window
  osg::Vec2d posPct;
  if (hud->getPosition(windowName, posPct) == 0)
    movePercent_(window, posPct);
}

int HudEditorGui::updateSize(const std::string& windowName)
{
  // Pull out size parameters from the HUD Position Manager
  osg::ref_ptr<simUtil::HudPositionManager> hud;
  if (!hud_.lock(hud))
    return 1;
  osg::Vec2d minXyPx;
  osg::Vec2d maxXyPx;
  if (hud->getSize(windowName, minXyPx, maxXyPx) != 0)
    return 1;

  // Get our window pointer
  auto i = windows_.find(windowName);
  if (i == windows_.end() || !i->second)
    return 1;
  const osg::Vec3d minXyz(minXyPx, 0.);
  const osg::Vec3d maxXyz(maxXyPx, 0.);
  i->second->updateSize(minXyz, maxXyz);
  return 0;
}

void HudEditorGui::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == osg::NodeVisitor::EVENT_VISITOR)
  {
    // Pull out the View and deal with resize events
    osgGA::EventVisitor* ev = nv.asEventVisitor();
    osgGA::GUIActionAdapter* aa = (ev ? ev->getActionAdapter() : nullptr);
    const osg::View* view = (aa ? aa->asView() : nullptr);
    const osg::Camera* camera = (view ? view->getCamera() : nullptr);
    const osg::Viewport* viewport = (camera ? camera->getViewport() : nullptr);

    // Determine if resize happened (we can't rely on resize events, they don't always include right size)
    if (viewport)
      handleResize_(viewport->width(), viewport->height());
  }
  osg::Camera::traverse(nv);
}

void HudEditorGui::handleResize_(double width, double height)
{
  if (widthPx_ == width && heightPx_ == height)
    return;

  // Save the values, update our projection, and fix the background
  widthPx_ = width;
  heightPx_ = height;
  background_->setMatrix(osg::Matrix::scale(width, height, 1.0));
  setProjectionMatrixAsOrtho2D(0.0, widthPx_ - 1.0, 0.0, heightPx_ - 1.0);
  osg::ref_ptr<simUtil::HudPositionManager> hud;

  // Reposition each window
  if (!hud_.lock(hud))
    return;
  for (auto i = windows_.begin(); i != windows_.end(); ++i)
  {
    osg::Vec2d posPct;
    if (hud->getPosition(i->first, posPct) == 0)
      movePercent_(i->second.get(), posPct);
  }
}

void HudEditorGui::movePercent_(osg::MatrixTransform* xform, const osg::Vec2d& posPct) const
{
  // Convert into pixels since we need that for positioning
  const osg::Vec3d posPx(posPct.x() * widthPx_, posPct.y() * heightPx_, 0.);
  xform->setMatrix(osg::Matrix::translate(posPx));
}

bool HudEditorGui::isVisible() const
{
  return getNodeMask() != 0;
}

void HudEditorGui::setVisible(bool flag)
{
  // Avoid noop
  if (flag == (getNodeMask() != 0))
    return;

  setNodeMask(flag ? ~0 : 0);
  osg::ref_ptr<HudPositionManager> hud;
  if (!flag || !hud_.lock(hud))
    return;

  // Iterate and update each window in case it was created after us
  std::vector<std::string> names;
  // Always hide everything, but only show the active windows
  hud->getAllWindows(names, flag);
  for (auto nameIter = names.begin(); nameIter != names.end(); ++nameIter)
    updatePosition(*nameIter);
}

void HudEditorGui::setSelected(const std::string& name, bool selected)
{
  auto i = windows_.find(name);
  if (i != windows_.end())
    i->second->setSelected(selected);
}

std::string HudEditorGui::intersect(const osg::Vec3d& mousePx, osg::Vec3d& offsetFromAnchorPx) const
{
  offsetFromAnchorPx = osg::Vec3d();
  for (auto i = windows_.begin(); i != windows_.end(); ++i)
  {
    // Figure out the mouse position in matrix coords
    const osg::Vec3d pos = i->second->getMatrix().getTrans();
    const osg::Vec3d mousePxRelative = mousePx - pos;

    // Bounds detection
    if (i->second->boundingBoxPx().contains(mousePxRelative))
    {
      osg::ref_ptr<simUtil::HudPositionManager> hud;
      osg::Vec2d anchorPosPct;
      if (hud_.lock(hud) && hud->getPosition(i->first, anchorPosPct) == 0)
        offsetFromAnchorPx.set(anchorPosPct.x() * widthPx_ - mousePx.x(), anchorPosPct.y() * heightPx_ - mousePx.y(), 0.);
      return i->first;
    }
  }
  return "";
}

/////////////////////////////////////////////////////////

HudEditorMouse::HudEditorMouse(simUtil::HudPositionManager* hud, simUtil::HudEditorGui* gui)
  : hud_(hud),
    gui_(gui),
    widthPx_(1.0),
    heightPx_(1.0)
{
}

int HudEditorMouse::push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Drop mouse commands on the ground
  return inEditorMode_() ? 1 : 0;
}

int HudEditorMouse::release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Ignore if not in editor mode
  if (!inEditorMode_())
    return 0;

  // Process right clicks on windows if we have a callback set
  if (ea.getButton() == ea.RIGHT_MOUSE_BUTTON && !currentSelection_.empty() && callback_)
    callback_->rightClicked(currentSelection_);

  // Eat the right click so it doesn't fall through to SIMDIS
  return 1;
}

int HudEditorMouse::move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (!inEditorMode_())
    return 0;

  // Detect the item under the mouse
  const std::string underMouse = hudUnderMouse_(ea.getX(), ea.getY(), mouseOffsetPx_);
  if (underMouse != currentSelection_)
  {
    // Draw the selection rectangle around "underMouse"
    osg::ref_ptr<simUtil::HudEditorGui> gui;
    if (gui_.lock(gui))
    {
      // Remove the old selection, then add the new selection
      gui->setSelected(currentSelection_, false);
      currentSelection_ = underMouse;
      gui->setSelected(currentSelection_, true);
    }
    else
      currentSelection_ = underMouse;
  }

  return 1;
}

int HudEditorMouse::drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Ignore events unless we're editing
  if (!inEditorMode_())
    return 0;
  // Avoid divide-by-zero or dragging when we don't have a selection
  osg::ref_ptr<simUtil::HudPositionManager> hud;
  if (currentSelection_.empty() || widthPx_ == 0.0 || heightPx_ == 0.0 || !hud_.lock(hud))
    return 1;

  // Tell the HUD to update the position
  const osg::Vec2d newPosPx(ea.getX() + mouseOffsetPx_.x(), ea.getY() + mouseOffsetPx_.y());
  hud->setPosition(currentSelection_, osg::Vec2d(newPosPx.x() / widthPx_, newPosPx.y() / heightPx_));

  // Tell the GUI to update to the HUD values
  osg::ref_ptr<simUtil::HudEditorGui> gui;
  if (gui_.lock(gui))
    gui->updatePosition(currentSelection_);
  return 1;
}

int HudEditorMouse::doubleClick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (!inEditorMode_())
    return 0;
  // On double-click, in editor mode reset the position of the current selection
  if (!currentSelection_.empty())
  {
    osg::ref_ptr<simUtil::HudPositionManager> hud;
    if (hud_.lock(hud))
      hud->resetPosition(currentSelection_);
    osg::ref_ptr<simUtil::HudEditorGui> gui;
    if (gui_.lock(gui))
      gui->updatePosition(currentSelection_);
  }
  // Eat the double-click
  return 1;
}

int HudEditorMouse::scroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Drop mouse commands on the ground
  return inEditorMode_() ? 1 : 0;
}

int HudEditorMouse::frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // We need the viewport for testing size.  Cannot rely on RESIZE events
  const osg::View* view = aa.asView();
  if (!view || !view->getCamera() || !view->getCamera()->getViewport())
    return 0;
  // only use the superhud to update the width and height
  const simVis::View* simView = dynamic_cast<const simVis::View*>(view);
  if (simView && simView->type() != simVis::View::VIEW_SUPERHUD)
    return 0;

  // Save the width and height for future mouse movement calculations
  const osg::Viewport* vp = view->getCamera()->getViewport();
  widthPx_ = vp->width();
  heightPx_ = vp->height();
  return 0;
}

int HudEditorMouse::touchBegan(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // TODO: SIM-15089
  return 0;
}

int HudEditorMouse::touchMoved(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // TODO: SIM-15089
  return 0;
}

int HudEditorMouse::touchEnded(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // TODO: SIM-15089
  return 0;
}

void HudEditorMouse::activate()
{
  // Turn on the HUD GUI
  osg::ref_ptr<simUtil::HudEditorGui> gui;
  if (hud_.valid() && gui_.lock(gui))
    gui_->setVisible(true);
}

void HudEditorMouse::deactivate()
{
  // Turn off the HUD GUI
  osg::ref_ptr<simUtil::HudEditorGui> gui;
  if (hud_.valid() && gui_.lock(gui))
    gui_->setVisible(false);
}

std::string HudEditorMouse::hudUnderMouse_(double xPx, double yPx, osg::Vec3d& mouseOffsetPx) const
{
  // Initialize mouseOffsetPx, return if we can't calculate
  mouseOffsetPx = osg::Vec3d();
  osg::ref_ptr<simUtil::HudPositionManager> hud;
  std::string closest;
  if (widthPx_ == 0.0 || heightPx_ == 0.0 || !hud_.lock(hud))
    return closest;

  // Get all the windows to test
  std::vector<std::string> windows;
  hud->getAllWindows(windows, true);

  // Use AHA (Dynamic Selection) algorithm
  double closestDistanceSq = 50.0 * 50.0;
  for (auto i = windows.begin(); i != windows.end(); ++i)
  {
    // Pull out the position, which comes out in percent but we convert to pixels
    osg::Vec2d pos;
    if (hud->getPosition(*i, pos) != 0)
      continue;

    // Convert it to pixels, since dynamic range picking relies on pixel distance
    pos.x() *= widthPx_;
    pos.y() *= heightPx_;
    const double thisDistSq = osg::square(pos.x() - xPx) + osg::square(pos.y() - yPx);
    // Save the window for later
    if (thisDistSq < closestDistanceSq)
    {
      mouseOffsetPx.set(pos.x() - xPx, pos.y() - yPx, 0.);
      closestDistanceSq = thisDistSq;
      closest = *i;
    }
  }

  // If there was no closest match, fall back to box intersection testing
  if (closest.empty())
  {
    osg::ref_ptr<simUtil::HudEditorGui> gui;
    if (gui_.lock(gui))
      return gui->intersect(osg::Vec3d(xPx, yPx, 0.), mouseOffsetPx);
  }
  return closest;
}

bool HudEditorMouse::inEditorMode_() const
{
  osg::ref_ptr<simUtil::HudEditorGui> gui;
  return hud_.valid() && gui_.lock(gui) && gui->isVisible();
}

void HudEditorMouse::setRightClickCallback(std::shared_ptr<simUtil::HudEditorRightClickCallback> cb)
{
  callback_ = cb;
}

/////////////////////////////////////////////////////////

HudPositionEditor::HudPositionEditor()
  : hud_(new HudPositionManager),
    gui_(new HudEditorGui(hud_.get())),
    mouse_(new HudEditorMouse(hud_.get(), gui_.get()))
{
  // Turn off the GUI by default.  Most use cases need a HUD Position Manager first
  gui_->setVisible(false);
}

HudPositionEditor::~HudPositionEditor()
{
}

void HudPositionEditor::bindAll(osg::Group& hudParent, simUtil::MouseDispatcher& dispatcher, int weight)
{
  hudParent.addChild(gui_.get());
  dispatcher.addManipulator(weight, mouse_);
}

simUtil::HudPositionManager* HudPositionEditor::hud() const
{
  return hud_.get();
}

simUtil::HudEditorGui* HudPositionEditor::gui() const
{
  return gui_.get();
}

std::shared_ptr<simUtil::HudEditorMouse> HudPositionEditor::mouse() const
{
  return mouse_;
}

bool HudPositionEditor::isVisible() const
{
  return gui_->isVisible();
}

void HudPositionEditor::setVisible(bool fl)
{
  // Assertion failure means you forgot to call bindAll or add the HUD GUI to the
  // scene somewhere appropriate.
  assert(gui_->getNumParents() != 0);
  gui_->setVisible(fl);
}

void HudPositionEditor::addWindow(const std::string& name, const osg::Vec2d& defaultPositionPct, HudPositionManager::RepositionCallback* reposCallback)
{
  hud_->addWindow(name, defaultPositionPct, reposCallback);
  gui_->updatePosition(name);
}

int HudPositionEditor::removeWindow(const std::string& name)
{
  const int rv = hud_->removeWindow(name);
  if (rv == 0)
    gui_->reset();
  return rv;
}

int HudPositionEditor::setSize(const std::string& name, const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx)
{
  const int rv = hud_->setSize(name, minXyPx, maxXyPx);
  if (rv == 0)
    gui_->updateSize(name);
  return rv;
}

int HudPositionEditor::setPosition(const std::string& name, const osg::Vec2d& positionPct)
{
  const int rv = hud_->setPosition(name, positionPct);
  if (rv == 0)
    gui_->updatePosition(name);
  return rv;
}

void HudPositionEditor::resetAllPositions()
{
  hud_->resetAllPositions();
  gui_->reset();
}

int HudPositionEditor::resetPosition(const std::string& name)
{
  const int rv = hud_->resetPosition(name);
  if (rv == 0)
    gui_->updatePosition(name);
  return rv;
}

}
