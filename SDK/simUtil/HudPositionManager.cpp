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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */
#include <cassert>
#include <string>
#include "osg/Camera"
#include "osg/MatrixTransform"
#include "osg/Vec2d"
#include "osgViewer/ViewerBase"
#include "osgGA/GUIEventHandler"
#include "simUtil/HudPositionManager.h"

namespace simUtil {

HudPositionManager::WindowData::WindowData(const std::string& name)
  : name_(name)
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
  if (positionPct_ == positionPct)
    return;
  positionPct_ = positionPct;
  emitPosition();
}

void HudPositionManager::WindowData::emitPosition()
{
  if (callback_.valid())
    callback_->setPosition(name_, positionPct_);
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

void HudPositionManager::WindowData::setSize(const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx)
{
  minXyPx_ = minXyPx;
  maxXyPx_ = maxXyPx;
}

void HudPositionManager::WindowData::getSize(osg::Vec2d& minXyPx, osg::Vec2d& maxXyPx) const
{
  minXyPx = minXyPx_;
  maxXyPx = maxXyPx_;
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
  assert(reposCallback != NULL);

  auto i = allWindows_.find(name);

  // Create the window if needed
  if (i == allWindows_.end())
  {
    WindowData* newWindow = new WindowData(name);
    newWindow->setRepositionCallback(reposCallback);
    allWindows_[name] = newWindow;
    newWindow->setDefaultPosition(defaultPositionPct);
    newWindow->setPosition(defaultPositionPct);
  }
  else
  {
    // Assertion failure means window was not removed before being added, which is
    // likely a developer error unless a reasonable use case is presented.  There are
    // cases where we will eventually expect that the WindowData* exists, but has
    // no callback, due to being removed or due to being loaded with a position
    // "externally" such as through a settings file.
    assert(i->second->repositionCallback() == NULL);

    // It is not necessarily an error to add a window that exists already.  Just update
    // the reposition callback, but don't bother setting the position, because we already
    // had a position for it in our records.
    // The use case for this is when the window is created due to being loaded from
    // a settings file.  We know a window will exist with this name, and know its
    // position, but it hasn't yet been created.  So we'll have a WindowData created
    // that doesn't have a callback yet.
    i->second->setRepositionCallback(reposCallback);
    i->second->setDefaultPosition(defaultPositionPct);
    i->second->emitPosition();
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
  assert(i->second->repositionCallback() == NULL);
  i->second->setRepositionCallback(NULL);
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

int HudPositionManager::getAllWindows(std::vector<std::string>& names) const
{
  names.clear();
  for (auto i = allWindows_.begin(); i != allWindows_.end(); ++i)
    names.push_back(i->first);
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

RepositionMatrixCallback::RepositionMatrixCallback(osg::MatrixTransform* xform)
  : xform_(xform)
{
}

RepositionMatrixCallback::~RepositionMatrixCallback()
{
}

void RepositionMatrixCallback::setPosition(const std::string& name, const osg::Vec2d& positionPct)
{
  osg::ref_ptr<osg::MatrixTransform> xform;
  if (!xform_.lock(xform))
    return;
  osg::Matrix mat = xform->getMatrix();
  mat.setTrans(osg::Vec3d(positionPct.x(), positionPct.y(), 0.0));
  xform->setMatrix(mat);
}

////////////////////////////////////////////////////////////////////////////////////

/** Responsible for tying in to get window sizes out for positioning */
class RepositionPixelsCallback::ResizeCallback : public osgGA::GUIEventHandler
{
public:
  ResizeCallback(RepositionPixelsCallback* parent)
    : windowSize_(0.0, 0.0),
      parent_(parent)
  {
    // Need to get a reasonable size
  }

  /** Checks for resize events */
  bool virtual handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*)
  {
    // RESIZE does not always emit correctly, especially starting in full screen mode, so use FRAME and always check size
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
    {
      // Cannot rely on getWindowWidth(), need to check viewport
      const osg::View* view = aa.asView();
      if (!view || !view->getCamera() || !view->getCamera()->getViewport())
        return false;
      // Pull the width and height out of the viewport
      const osg::Viewport* vp = view->getCamera()->getViewport();
      const osg::Vec2d newSize(vp->width(), vp->height());
      if (newSize == windowSize_)
        return false;
      windowSize_ = newSize;

      // Get a hard lock on the parent
      osg::ref_ptr<RepositionPixelsCallback> parent;
      if (!parent_.lock(parent))
        return false;

      // For each of the saved positions, update the size and emit setPosition
      for (auto i = parent->savedPositionsPct_.begin(); i != parent->savedPositionsPct_.end(); ++i)
      {
        const osg::Vec2d newPosition(windowSize_.x() * i->second.x(), windowSize_.y() * i->second.y());
        parent->setPositionPx(i->first, newPosition);
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
  // Record the position so that on resize we can re-call setPositionPx correctly
  savedPositionsPct_[name] = positionPct;

  // Get a hard lock on the callback
  osg::ref_ptr<ResizeCallback> resizeCb;
  if (!resizeCb_.lock(resizeCb))
    return;

  // Calculate the pixel position based on the last seen window size and the incoming percentage values
  const auto& windowSize = resizeCb->windowSize();
  osg::Vec2d asPixels(positionPct.x() * windowSize.x(), positionPct.y() * windowSize.y());
  setPositionPx(name, asPixels);
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
