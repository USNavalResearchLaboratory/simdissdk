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
#include <cassert>
#include <string>
#include "osg/Camera"
#include "osg/MatrixTransform"
#include "osg/Vec2d"
#include "osgViewer/ViewerBase"
#include "osgGA/GUIEventHandler"
#include "simVis/DevicePixelRatioUtils.h"
#include "simUtil/HudPositionManager.h"

namespace simUtil {

namespace {

/** Helper function that returns a default set of all possible window alignments (the default) */
const std::set<HudWindowAlignment>& allHudWindowAlignments()
{
  static const std::set<HudWindowAlignment> s_alignments = {
    HudWindowAlignment::LEFT_TOP,
    HudWindowAlignment::LEFT_CENTER,
    HudWindowAlignment::LEFT_BOTTOM,
    HudWindowAlignment::CENTER_TOP,
    HudWindowAlignment::CENTER_CENTER,
    HudWindowAlignment::CENTER_BOTTOM,
    HudWindowAlignment::RIGHT_TOP,
    HudWindowAlignment::RIGHT_CENTER,
    HudWindowAlignment::RIGHT_BOTTOM
  };
  return s_alignments;
}

}

HudPositionManager::WindowData::WindowData(const std::string& name)
  : name_(name),
    allowedAlignments_(allHudWindowAlignments())
{
}

std::string HudPositionManager::WindowData::name() const
{
  return name_;
}

void HudPositionManager::WindowData::setRepositionCallback(HudPositionManager::RepositionCallback* callback)
{
  callback_ = callback;
}

HudPositionManager::RepositionCallback* HudPositionManager::WindowData::repositionCallback() const
{
  return callback_.get();
}

void HudPositionManager::WindowData::setPosition(const osg::Vec2d& positionPct)
{
  // A newly created WindowData starts at (0,0), but that is not necessarily
  // an initialized position. The first setPosition() must always notify.
  if (hasPosition_ && positionPct_ == positionPct)
    return;

  positionPct_ = positionPct;
  hasPosition_ = true;
  emitPosition();
}

void HudPositionManager::WindowData::emitPosition()
{
  if (callback_.valid())
    callback_->setPosition(name_, positionPct_);
}

bool HudPositionManager::WindowData::hasPosition() const
{
  return hasPosition_;
}

osg::Vec2d HudPositionManager::WindowData::position() const
{
  return positionPct_;
}

void HudPositionManager::WindowData::setDefaultPosition(const osg::Vec2d& posPct)
{
  defaultPositionPct_ = posPct;
}

osg::Vec2d HudPositionManager::WindowData::defaultPosition() const
{
  return defaultPositionPct_;
}

int HudPositionManager::WindowData::setAlignment(std::optional<HudWindowAlignment> alignment)
{
  if (alignment && !supportsAlignment(*alignment))
    return 1;

  if (alignmentOverride_ == alignment)
    return 0;

  alignmentOverride_ = alignment;
  emitAlignmentOffset();
  return 0;
}

std::optional<HudWindowAlignment> HudPositionManager::WindowData::alignment() const
{
  if (alignmentOverride_)
    return alignmentOverride_;
  return defaultAlignment_;
}

std::optional<HudWindowAlignment> HudPositionManager::WindowData::alignmentOverride() const
{
  return alignmentOverride_;
}

int HudPositionManager::WindowData::setDefaultAlignment(std::optional<HudWindowAlignment> alignment)
{
  if (alignment && !supportsAlignment(*alignment))
    return 1;

  if (defaultAlignment_ == alignment)
    return 0;

  defaultAlignment_ = alignment;
  // A user override has precedence and should not be disturbed.
  if (!alignmentOverride_)
    emitAlignmentOffset();
  return 0;
}

std::optional<HudWindowAlignment> HudPositionManager::WindowData::defaultAlignment() const
{
  return defaultAlignment_;
}

void HudPositionManager::WindowData::resetAlignment()
{
  if (!alignmentOverride_)
    return;

  alignmentOverride_.reset();
  emitAlignmentOffset();
}

void HudPositionManager::WindowData::emitAlignmentOffset()
{
  if (callback_.valid())
    callback_->setAlignmentOffsetPx(name_, alignmentOffsetPx_());
}

void HudPositionManager::WindowData::setSize(const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx)
{
  if (minXyPx_ == minXyPx && maxXyPx_ == maxXyPx)
    return;

  minXyPx_ = minXyPx;
  maxXyPx_ = maxXyPx;
  emitAlignmentOffset();
}

osg::Vec2d HudPositionManager::WindowData::alignmentOffsetPx_() const
{
  const std::optional<HudWindowAlignment> effectiveAlignment = alignment();
  if (!effectiveAlignment)
    return osg::Vec2d();

  const double centerX = (minXyPx_.x() + maxXyPx_.x()) * 0.5;
  const double centerY = (minXyPx_.y() + maxXyPx_.y()) * 0.5;

  switch (*effectiveAlignment)
  {
  case HudWindowAlignment::LEFT_TOP:
    return osg::Vec2d(-minXyPx_.x(), -maxXyPx_.y());

  case HudWindowAlignment::LEFT_CENTER:
    return osg::Vec2d(-minXyPx_.x(), -centerY);

  case HudWindowAlignment::LEFT_BOTTOM:
    return osg::Vec2d(-minXyPx_.x(), -minXyPx_.y());

  case HudWindowAlignment::CENTER_TOP:
    return osg::Vec2d(-centerX, -maxXyPx_.y());

  case HudWindowAlignment::CENTER_CENTER:
    return osg::Vec2d(-centerX, -centerY);

  case HudWindowAlignment::CENTER_BOTTOM:
    return osg::Vec2d(-centerX, -minXyPx_.y());

  case HudWindowAlignment::RIGHT_TOP:
    return osg::Vec2d(-maxXyPx_.x(), -maxXyPx_.y());

  case HudWindowAlignment::RIGHT_CENTER:
    return osg::Vec2d(-maxXyPx_.x(), -centerY);

  case HudWindowAlignment::RIGHT_BOTTOM:
    return osg::Vec2d(-maxXyPx_.x(), -minXyPx_.y());
  }

  // Unexpected alignment flag
  assert(0);
  return osg::Vec2d();
}

void HudPositionManager::WindowData::getSize(osg::Vec2d& minXyPx, osg::Vec2d& maxXyPx) const
{
  minXyPx = minXyPx_;
  maxXyPx = maxXyPx_;
}

int HudPositionManager::WindowData::setAllowedAlignments(const std::set<HudWindowAlignment>& alignments)
{
  // Must allow something, ignore nonsense input
  if (alignments.empty())
    return 1;
  if (alignments == allowedAlignments_)
    return 0;

  // Hyper-vigilant on the allowed alignments
  const auto& allAlignments = allHudWindowAlignments();
  for (const auto alignment : alignments)
  {
    if (allAlignments.find(alignment) == allAlignments.end())
      return 1;
  }

  // Validate both values, not merely the current effective alignment. A user
  // might reset an override later and reveal the default alignment.
  if (defaultAlignment_ && alignments.find(*defaultAlignment_) == alignments.end())
    return 1;
  if (alignmentOverride_ && alignments.find(*alignmentOverride_) == alignments.end())
    return 1;

  allowedAlignments_ = alignments;
  return 0;
}

const std::set<HudWindowAlignment>& HudPositionManager::WindowData::allowedAlignments() const
{
  return allowedAlignments_;
}

bool HudPositionManager::WindowData::supportsAlignment(HudWindowAlignment alignment) const
{
  return allowedAlignments_.find(alignment) != allowedAlignments_.end();
}

HudPositionManager::WindowData::~WindowData()
{
}

////////////////////////////////////////////////////////////////////////////////////

HudPositionManager::HudPositionManager()
{
}

HudPositionManager::~HudPositionManager()
{
  // Auto-deletes ref_ptr
  allWindows_.clear();
}

void HudPositionManager::addWindow(const std::string& name, const osg::Vec2d& defaultPositionPct, RepositionCallback* reposCallback)
{
  // Assertion failure means other asserts in the code will fail due to unexpected precondition
  assert(reposCallback != nullptr);

  auto i = allWindows_.find(name);

  // Create the window if needed
  if (i == allWindows_.end())
  {
    WindowData* newWindow = new WindowData(name);
    newWindow->setRepositionCallback(reposCallback);
    allWindows_[name] = newWindow;
    newWindow->setDefaultPosition(defaultPositionPct);
    newWindow->setPosition(defaultPositionPct);
    newWindow->emitAlignmentOffset();
  }
  else
  {
    // Assertion failure means window was not removed before being added, which is
    // likely a developer error unless a reasonable use case is presented. Several
    // functions can create windows e.g. from settings or mementos, but they won't
    // be setting reposition callbacks in those cases.
    assert(i->second->repositionCallback() == nullptr);

    // Existing data may have been loaded by setPosition(), setAlignment(), or
    // a memento restore before the actual HUD window was constructed.
    i->second->setRepositionCallback(reposCallback);
    i->second->setDefaultPosition(defaultPositionPct);

    if (i->second->hasPosition())
      i->second->emitPosition();
    else
      i->second->setPosition(defaultPositionPct);
    i->second->emitAlignmentOffset();
  }
}

int HudPositionManager::removeWindow(const std::string& name)
{
  // Do not actually remove the record of the window, because it might come back and
  // would need to know its old position.
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return 1;
  // Assertion failure means removal of a window that was already removed
  assert(i->second->repositionCallback() != nullptr);
  i->second->setRepositionCallback(nullptr);
  return 0;
}

int HudPositionManager::getPosition(const std::string& name, osg::Vec2d& positionPct) const
{
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return 1;
  positionPct = i->second->position();
  return 0;
}

int HudPositionManager::setPosition(const std::string& name, const osg::Vec2d& positionPct)
{
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
  {
    // Cache the position for the future, for when the window actually is created.
    WindowData* newWindow = new WindowData(name);
    allWindows_[name] = newWindow;
    newWindow->setPosition(positionPct);
    return 0;
  }
  i->second->setPosition(positionPct);
  return 0;
}

std::optional<HudWindowAlignment> HudPositionManager::getAlignment(const std::string& name) const
{
  const auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return {};
  return i->second->alignment();
}

std::optional<HudWindowAlignment> HudPositionManager::getAlignmentOverride(const std::string& name) const
{
  const auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return {};
  return i->second->alignmentOverride();
}

int HudPositionManager::setAlignment(const std::string& name, std::optional<HudWindowAlignment> alignment)
{
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
  {
    osg::ref_ptr<WindowData> newWindow = new WindowData(name);
    const int rv = newWindow->setAlignment(alignment);
    if (rv != 0)
      return rv;
    allWindows_[name] = newWindow.get();
    return 0;
  }

  return i->second->setAlignment(alignment);
}

std::optional<HudWindowAlignment> HudPositionManager::getDefaultAlignment(const std::string& name) const
{
  const auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return {};
  return i->second->defaultAlignment();
}

int HudPositionManager::setDefaultAlignment(const std::string& name, std::optional<HudWindowAlignment> alignment)
{
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
  {
    osg::ref_ptr<WindowData> newWindow = new WindowData(name);
    const int rv = newWindow->setDefaultAlignment(alignment);
    if (rv != 0)
      return rv;
    allWindows_[name] = newWindow.get();
    return 0;
  }

  return i->second->setDefaultAlignment(alignment);
}

std::set<HudWindowAlignment> HudPositionManager::getAllowedAlignments(const std::string& name) const
{
  const auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return {};
  return i->second->allowedAlignments();
}

int HudPositionManager::setAllowedAlignments(const std::string& name, const std::set<HudWindowAlignment>& alignments)
{
  auto i = allWindows_.find(name);
  if (i != allWindows_.end())
    return i->second->setAllowedAlignments(alignments);

  osg::ref_ptr<WindowData> newWindow = new WindowData(name);
  const int rv = newWindow->setAllowedAlignments(alignments);
  if (rv != 0)
    return rv;
  allWindows_[name] = newWindow.get();
  return 0;
}

int HudPositionManager::resetAlignment(const std::string& name)
{
  const auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return 1;

  i->second->resetAlignment();
  return 0;
}

void HudPositionManager::resetAllAlignments()
{
  for (auto i = allWindows_.begin(); i != allWindows_.end(); ++i)
    i->second->resetAlignment();
}

int HudPositionManager::resetPlacement(const std::string& name)
{
  const auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return 1;

  i->second->setPosition(i->second->defaultPosition());
  i->second->resetAlignment();
  return 0;
}

void HudPositionManager::resetAllPlacements()
{
  for (auto i = allWindows_.begin(); i != allWindows_.end(); ++i)
  {
    i->second->setPosition(i->second->defaultPosition());
    i->second->resetAlignment();
  }
}

int HudPositionManager::resetPosition(const std::string& name)
{
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return 1;
  i->second->setPosition(i->second->defaultPosition());
  return 0;
}

void HudPositionManager::resetAllPositions()
{
  for (auto i = allWindows_.begin(); i != allWindows_.end(); ++i)
    i->second->setPosition(i->second->defaultPosition());
}

int HudPositionManager::getAllWindows(std::vector<std::string>& names, bool activeOnly) const
{
  names.clear();
  for (auto i = allWindows_.begin(); i != allWindows_.end(); ++i)
  {
    // Active callbacks have a reposition callback
    if (!activeOnly || i->second->repositionCallback() != nullptr)
      names.push_back(i->first);
  }
  return 0;
}

int HudPositionManager::getSize(const std::string& name, osg::Vec2d& minXyPx, osg::Vec2d& maxXyPx) const
{
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return 1;
  i->second->getSize(minXyPx, maxXyPx);
  return 0;
}

int HudPositionManager::setSize(const std::string& name, const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx)
{
  auto i = allWindows_.find(name);
  if (i == allWindows_.end())
    return 1;
  // Note that this call may trigger a callback to owner
  i->second->setSize(minXyPx, maxXyPx);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////

/** Responsible for tying in to get window sizes out for positioning */
class RepositionPixelsCallback::ResizeCallback : public osgGA::GUIEventHandler
{
public:
  explicit ResizeCallback(RepositionPixelsCallback* parent)
    : windowSize_(0.0, 0.0),
      parent_(parent)
  {
    // Need to get a reasonable size
  }

  /** Checks for resize events */
  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*) override
  {
    // RESIZE does not always emit correctly, especially starting in full screen mode, so use FRAME and always check size
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
    {
      // Cannot rely on getWindowWidth(), need to check viewport
      const osg::View* view = aa.asView();
      if (!view || !view->getCamera() || !view->getCamera()->getViewport())
        return false;

      // Pull the width and height out of the viewport and convert to logical coordinates
      const osg::Viewport* vp = view->getCamera()->getViewport();
      const osg::Vec2d newSize(simVis::DevicePixelRatioUtils::toLogical(vp->width()),
        simVis::DevicePixelRatioUtils::toLogical(vp->height()));
      if (newSize == windowSize_)
        return false;
      windowSize_ = newSize;

      // Get a hard lock on the parent
      osg::ref_ptr<RepositionPixelsCallback> parent;
      if (!parent_.lock(parent))
        return false;

      // For each saved position, update the pixel coordinates for the new viewport size.
      for (const auto& [name, savedPosition] : parent->savedPositions_)
      {
        if (!savedPosition.hasPosition)
          continue;

        const osg::Vec2d newPosition(
          windowSize_.x() * savedPosition.positionPct.x() + savedPosition.alignmentOffsetPx.x(),
          windowSize_.y() * savedPosition.positionPct.y() + savedPosition.alignmentOffsetPx.y());
        parent->setPositionPx(name, newPosition);
      }
    }
    return false;
  }

  /** Retrieves the last window size seen */
  osg::Vec2d windowSize() const
  {
    return windowSize_;
  }

private:
  osg::Vec2d windowSize_;
  osg::observer_ptr<RepositionPixelsCallback> parent_;
};

RepositionPixelsCallback::RepositionPixelsCallback(osg::Node* node)
  : cbAttachNode_(node)
{
  if (node)
  {
    resizeCb_ = new ResizeCallback(this);
    cbAttachNode_->addEventCallback(resizeCb_.get());
  }
}

RepositionPixelsCallback::~RepositionPixelsCallback()
{
  // Remove the event callback
  osg::ref_ptr<osg::Node> cbAttach;
  if (cbAttachNode_.lock(cbAttach) && resizeCb_.valid())
    cbAttach->removeEventCallback(resizeCb_.get());
}

void RepositionPixelsCallback::setPosition(const std::string& name, const osg::Vec2d& positionPct)
{
  SavedPositionData& savedPosition = getOrCreateSavedPosition_(name);
  savedPosition.positionPct = positionPct;
  savedPosition.hasPosition = true;

  // Get a hard lock on the callback
  osg::ref_ptr<ResizeCallback> resizeCb;
  if (!resizeCb_.lock(resizeCb))
    return;

  // Calculate the pixel position based on the last seen window size, and apply the
  // manager-provided logical-pixel alignment offset.
  const auto& windowSize = resizeCb->windowSize();
  const osg::Vec2d asPixels(
    positionPct.x() * windowSize.x() + savedPosition.alignmentOffsetPx.x(),
    positionPct.y() * windowSize.y() + savedPosition.alignmentOffsetPx.y());
  setPositionPx(name, asPixels);
}

void RepositionPixelsCallback::setAlignmentOffsetPx(const std::string& name, const osg::Vec2d& offsetPx)
{
  SavedPositionData& savedPosition = getOrCreateSavedPosition_(name);
  if (savedPosition.alignmentOffsetPx == offsetPx)
    return;

  savedPosition.alignmentOffsetPx = offsetPx;

  // If no position has been supplied yet, retain the offset. A later
  // setPosition() call will apply it.
  if (!savedPosition.hasPosition)
    return;

  osg::ref_ptr<ResizeCallback> resizeCb;
  if (!resizeCb_.lock(resizeCb))
    return;

  const auto& windowSize = resizeCb->windowSize();
  const osg::Vec2d positionPx(
    savedPosition.positionPct.x() * windowSize.x() + offsetPx.x(),
    savedPosition.positionPct.y() * windowSize.y() + offsetPx.y());

  setPositionPx(name, positionPx);
}

////////////////////////////////////////////////////////////////////////////////////

RepositionMatrixPxCallback::RepositionMatrixPxCallback(osg::MatrixTransform* xform)
  : RepositionPixelsCallback(xform),
    xform_(xform)
{
}

RepositionMatrixPxCallback::~RepositionMatrixPxCallback()
{
}

RepositionPixelsCallback::SavedPositionData& RepositionPixelsCallback::getOrCreateSavedPosition_(const std::string& name)
{
  auto i = savedPositions_.find(name);
  if (i == savedPositions_.end())
  {
    const auto result = savedPositions_.insert({ name, SavedPositionData() });
    i = result.first;
  }
  return i->second;
}

void RepositionMatrixPxCallback::setPositionPx(const std::string& name, const osg::Vec2d& positionPx)
{
  osg::ref_ptr<osg::MatrixTransform> xform;
  if (!xform_.lock(xform))
    return;
  osg::Matrix mat = xform->getMatrix();
  mat.setTrans(osg::Vec3d(positionPx.x(), positionPx.y(), 0.0));
  xform->setMatrix(mat);
}

}
