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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgGA/GUIActionAdapter"
#include "osgEarth/LineDrawable"
#include "simVis/View.h"
#include "simVis/Utils.h"
#include "simUtil/ResizeViewManipulator.h"

namespace simUtil {

/// Color for rubber band where we are not highlighting a corner
static const osg::Vec4f BAND_NORMAL_COLOR(1.f, 1.f, 1.f, 0.f);
/// Rubber band will draw a highlight on corners to move
static const osg::Vec4f BAND_HIGHLIGHT_COLOR(0.3f, .8f, 0.3f, 1.f);
/// Width of line for rubber band when dragging
static const float BOLD_WIDTH = 5.f;
/// Width of line for rubber band when not dragging
static const float NORMAL_WIDTH = 5.f;
/// Distance from the edge for picking an edge (vs. picking center)
static const double EDGE_SIZE = 15;
/// Minimum width and height for a viewport when dragging, in pixels
static const double MINIMUM_VIEWPORT_SIZE = 10;

/// Encapsulates a box drawn around the currently selected view
class ResizeViewManipulator::RubberBand
{
public:
  /** Constructor */
  explicit RubberBand(ResizeViewManipulator& manip)
    : manip_(manip)
  {
    initialize_();
    // Add the rubber band to the HUD
    xform_->setNodeMask(0);
    if (manip.hudGroup_.valid())
      manip.hudGroup_->addChild(xform_);
  }

  virtual ~RubberBand()
  {
    osg::Group* parent = xform_->getParent(0);
    if (parent)
      parent->removeChild(xform_);
  }

  /** Places a box around the passed in view; if nullptr hides the box*/
  void attach(simVis::View* view)
  {
    // Hide if nullptr
    if (view == nullptr)
    {
      xform_->setNodeMask(0);
      return;
    }

    // Calculate the absolute position of the viewport
    simVis::View::Extents extents = view->getExtents();
    manip_.toAbsoluteExtents_(*view, extents, nullptr);
    // Move the matrix to over-top, and turn on node mask
    xform_->setMatrix(osg::Matrix::scale(extents.width_, extents.height_, 1) *
      osg::Matrix::translate(extents.x_, extents.y_, 0));

#if OSGEARTH_SOVERSION < 121
    // Avoid osgEarth #1863 by calling finish() here, only if coming out of invisible. Fixed a little before 121 update.
    if (xform_->getNodeMask() == 0)
      line_->finish();
#endif

    xform_->setNodeMask(~0);
  }

  /** Highlight corner */
  void highlightCorner(ResizeViewManipulator::DragPoint corner)
  {
    // Set the highlight corner: highlight all for center mode
    if (corner == ResizeViewManipulator::CENTER)
    {
      for (size_t k = 0; k < 8; ++k)
        line_->setColor(k, BAND_HIGHLIGHT_COLOR);
    }
    else
    {
      size_t cornerNum = static_cast<size_t>(corner);
      // To color the line segments on both sides of the appropriate vertex, set the next vertex's color too
      size_t otherCorner = (cornerNum == 7 ? 0 : cornerNum + 1);
      for (size_t k = 0; k < 8; ++k)
      {
        bool highlight = (k == cornerNum || k == otherCorner);
        line_->setColor(k, (highlight ? BAND_HIGHLIGHT_COLOR : BAND_NORMAL_COLOR));
      }
    }

#if OSGEARTH_SOVERSION < 121
    // Avoid osgEarth #1863 by calling finish() here, but only when visible. Fixed a little before 121 update.
    if (xform_->getNodeMask() != 0)
      line_->finish();
#endif
  }

  /** Set the box to bold */
  void setBold(bool bold)
  {
    line_->setLineWidth(bold ? BOLD_WIDTH : NORMAL_WIDTH);
  }

private:
  void initialize_()
  {
    line_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
    line_->setDataVariance(osg::Object::DYNAMIC);

    line_->pushVertex(osg::Vec3(0.f, 1.f, 0.f));
    line_->pushVertex(osg::Vec3(0.5f, 1.f, 0.f));
    line_->pushVertex(osg::Vec3(1.f, 1.f, 0.f));
    line_->pushVertex(osg::Vec3(1.f, 0.5f, 0.f));
    line_->pushVertex(osg::Vec3(1.f, 0.f, 0.f));
    line_->pushVertex(osg::Vec3(0.5f, 0.f, 0.f));
    line_->pushVertex(osg::Vec3(0.f, 0.f, 0.f));
    line_->pushVertex(osg::Vec3(0.f, 0.5f, 0.f));

    line_->setColor(BAND_NORMAL_COLOR);

    line_->dirty();

    xform_ = new osg::MatrixTransform();
    xform_->addChild(line_.get());

    line_->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, 0);
    line_->getOrCreateStateSet()->setMode(GL_BLEND, 1);
    line_->setCullingActive(false);
  }

  osg::ref_ptr<osg::MatrixTransform> xform_;
  osg::ref_ptr<osgEarth::LineDrawable> line_;
  ResizeViewManipulator& manip_;
};

////////////////////////////////////////////////////////////

ResizeViewManipulator::ResizeViewManipulator(simVis::View* mainView, osg::Group* hudGroup)
  : MouseManipulatorAdapter(),
    enabled_(false),
    mainView_(mainView),
    hudGroup_(hudGroup),
    originalExtents_(0, 0, 100, 100, false),
    dragPoint_(NONE)
{
  rubberBand_ = new RubberBand(*this);
}

ResizeViewManipulator::~ResizeViewManipulator()
{
  delete rubberBand_;
}

void ResizeViewManipulator::setEnabled(bool enabled)
{
  if (enabled == enabled_)
    return;
  enabled_ = enabled;
  activeView_ = nullptr;
  rubberBand_->attach(nullptr);
  setDragPoint_(NONE);
}

bool ResizeViewManipulator::isEnabled() const
{
  return enabled_;
}

int ResizeViewManipulator::push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Only work on primary button
  if (!enabled_ || ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    return 0;

  // Always clear out active view on mouse push
  activeView_ = nullptr;
  osg::ref_ptr<simVis::View> view = static_cast<simVis::View*>(aa.asView());
  // Ignore events from nullptr views and the main view
  if (!view.valid() || view == mainView_.get())
    return 0;

  const osg::Vec2d mousePosition = osg::Vec2d(ea.getX(), ea.getY());
  setDragPoint_(calculateDragPoint_(*view, mousePosition));
  // If calculated to be outside of the rectangle, ignore the point
  if (dragPoint_ == NONE)
    return 0;

  // Save all the data we need to process mouse movement
  activeView_ = view;
  originalExtents_ = activeView_->getExtents();
  anchorMousePosition_ = mousePosition;

  // Tell the rubber band to highlight this view in bold
  rubberBand_->attach(view.get());
  rubberBand_->setBold(true);
  rubberBand_->highlightCorner(dragPoint_);
  return 1;
}

int ResizeViewManipulator::release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Only work on primary button
  if (!enabled_ || ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    return 0;
  // Ignore, if we do not have an active view
  if (!activeView_.valid())
    return 0;

  // Capture the click and clear out the active view
  activeView_ = nullptr;
  rubberBand_->setBold(false);
  rubberBand_->attach(nullptr);
  rubberBand_->highlightCorner(NONE);

  // After release, make sure the highlighting is correct.  We can just call move() here to do that
  move(ea, aa);
  return 1;
}

int ResizeViewManipulator::move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // If we're disabled, do nothing
  if (!enabled_)
    return 0;

  // If we have an active view, then we don't need to do anything here, just return 1 for interception
  if (activeView_.valid())
    return 1;
  // Otherwise we just return 0 -- we don't intercept

  // Figure out what view is under the cursor and highlight it
  osg::ref_ptr<simVis::View> underMouse = static_cast<simVis::View*>(aa.asView());
  // Ignore events from nullptr views and the main view
  if (underMouse == nullptr || underMouse == mainView_.get())
  {
    rubberBand_->attach(nullptr);
    setDragPoint_(NONE);
    return 0;
  }

  rubberBand_->setBold(false);
  osg::Vec2d mouseXY = osg::Vec2d(ea.getX(), ea.getY());
  setDragPoint_(calculateDragPoint_(*underMouse, mouseXY));
  rubberBand_->highlightCorner(dragPoint_);
  rubberBand_->attach(underMouse.get());
  return 0;
}

int ResizeViewManipulator::drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // If we're disabled, do nothing
  if (!enabled_)
    return 0;

  // If we have an active view, then we always intercept
  osg::ref_ptr<simVis::View> view;
  activeView_.lock(view);
  if (!view.valid())
    return 0;

  // Only work on primary button
  if ((ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) == 0)
    return 1;

  // Adjust the corner/side of the view as needed
  osg::Vec2d newPosition = osg::Vec2d(ea.getX(), ea.getY());
  drag_(newPosition);
  // Update the position of the rubber band
  rubberBand_->attach(view.get());
  return 1;
}

void ResizeViewManipulator::drag_(const osg::Vec2d& newXY)
{
  osg::ref_ptr<simVis::View> view;
  activeView_.lock(view);
  if (!view.valid())
    return;

  // Calculate the change in extents
  const osg::Vec2d deltaXY = newXY - anchorMousePosition_;

  // Get the absolute extents
  bool wasRatio = false;
  simVis::View::Extents extents = originalExtents_;
  toAbsoluteExtents_(*view, extents, &wasRatio);

  // Adjust based on the X coordinate
  switch (dragPoint_)
  {
  case CENTER:
    extents.x_ += deltaXY.x();
    break;

  case TOP_RIGHT:
  case RIGHT:
  case BOTTOM_RIGHT:
    extents.width_ = osg::clampAbove(MINIMUM_VIEWPORT_SIZE, extents.width_ + deltaXY.x());
    break;

  case TOP_LEFT:
  case LEFT:
  case BOTTOM_LEFT:
  {
    const double maxX = (extents.x_ + extents.width_ - MINIMUM_VIEWPORT_SIZE);
    const double newX = osg::clampBelow(extents.x_ + deltaXY.x(), maxX);
    extents.width_ -= (newX - extents.x_);
    extents.x_ = newX;
    break;
  }

  default: // top, bottom, none
    break;
  }

  // Adjust based on the Y coordinate
  switch (dragPoint_)
  {
  case CENTER:
    extents.y_ += deltaXY.y();
    break;

  case TOP_LEFT:
  case TOP:
  case TOP_RIGHT:
    extents.height_ = osg::clampAbove(MINIMUM_VIEWPORT_SIZE, extents.height_ + deltaXY.y());
    break;

  case BOTTOM_LEFT:
  case BOTTOM:
  case BOTTOM_RIGHT:
  {
    const double maxY = (extents.y_ + extents.height_ - MINIMUM_VIEWPORT_SIZE);
    const double newY = osg::clampBelow(extents.y_ + deltaXY.y(), maxY);
    extents.height_ -= (newY - extents.y_);
    extents.y_ = newY;
    break;
  }

  default: // left, right, none
    break;
  }

  // Convert back to ratio if needed
  if (wasRatio)
    toRatioExtents_(*view, extents);
  view->setExtents(extents);
}

ResizeViewManipulator::DragPoint ResizeViewManipulator::calculateDragPoint_(const simVis::View& view, const osg::Vec2d& mouseXY) const
{
  // Calculate the absolute extents
  simVis::View::Extents extents = view.getExtents();
  toAbsoluteExtents_(view, extents, nullptr);

  // Test the top side
  if (mouseXY.y() > extents.y_ + extents.height_ - EDGE_SIZE)
  {
    // Must be one of the top, TL, TR
    if (mouseXY.x() < extents.x_ + EDGE_SIZE)
      return TOP_LEFT;
    else if (mouseXY.x() > extents.x_ + extents.width_ - EDGE_SIZE)
      return TOP_RIGHT;
    return TOP;
  }

  // Check the bottom next
  if (mouseXY.y() < extents.y_ + EDGE_SIZE)
  {
    // Must be one of the bottom, BL, BR
    if (mouseXY.x() < extents.x_ + EDGE_SIZE)
      return BOTTOM_LEFT;
    else if (mouseXY.x() > extents.x_ + extents.width_ - EDGE_SIZE)
      return BOTTOM_RIGHT;
    return BOTTOM;
  }

  // Check left/right border
  if (mouseXY.x() < extents.x_ + EDGE_SIZE)
    return LEFT;
  else if (mouseXY.x() > extents.x_ + extents.width_ - EDGE_SIZE)
    return RIGHT;
  return CENTER;
}

void ResizeViewManipulator::toAbsoluteExtents_(const simVis::View& view, simVis::View::Extents& extents, bool* wasRatio) const
{
  // Remember whether it was already a ratio or not
  if (wasRatio)
    *wasRatio = extents.isRatio_;
  // Return early from the is-ratio check
  if (!extents.isRatio_)
    return;

  // Pull out the host's size
  const osg::View* host = view.getHostView();
  if (host == nullptr)
    return;
  const osg::Viewport* rvp = host->getCamera()->getViewport();
  if (rvp == nullptr)
    return;

  // Calculate the absolute pixels
  extents.height_ *= rvp->height();
  extents.y_ *= rvp->height();
  extents.width_ *= rvp->width();
  extents.x_ *= rvp->width();
  extents.isRatio_ = false;
}

void ResizeViewManipulator::toRatioExtents_(const simVis::View& view, simVis::View::Extents& extents) const
{
  // Return from the no-op
  if (extents.isRatio_)
    return;

  // Pull out the host's size
  const osg::View* host = view.getHostView();
  if (host == nullptr)
    return;
  const osg::Viewport* rvp = host->getCamera()->getViewport();
  if (rvp == nullptr)
    return;

  extents.height_ /= rvp->height();
  extents.y_ /= rvp->height();
  extents.width_ /= rvp->width();
  extents.x_ /= rvp->width();
  extents.isRatio_ = true;
}

void ResizeViewManipulator::addListener(ListenerPtr listener)
{
  listeners_.push_back(listener);
}

void ResizeViewManipulator::removeListener(ListenerPtr listener)
{
  listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
}

void ResizeViewManipulator::setDragPoint_(DragPoint dragPoint)
{
  if (dragPoint_ == dragPoint)
    return;
  dragPoint_ = dragPoint;
  for (std::vector<ListenerPtr>::const_iterator i = listeners_.begin(); i != listeners_.end(); ++i)
    (*i)->dragPointChanged(dragPoint_);
}

}
