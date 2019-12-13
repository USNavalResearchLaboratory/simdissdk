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
#ifndef SIMUTIL_HUDPOSITIONMANAGER_H
#define SIMUTIL_HUDPOSITIONMANAGER_H

#include <map>
#include <string>
#include <vector>
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osg/Referenced"
#include "osg/Vec2d"
#include "simCore/Common/Export.h"

namespace osg {
  class MatrixTransform;
  class Node;
}
namespace osgViewer { class ViewerBase; }

namespace simUtil {

/**
 * Responsible for centralizing the management of window positioning in a HUD system.  Relies
 * on a callback system to reposition child windows.  All percentages are in the range from 0.0 to
 * 1.0.  Position (0.0, 0.0) is in lower-left; (1.0, 1.0) is in the upper-right.
 *
 * This class works on an inversion of control.  It is responsible for telling windows where to
 * position using the RepositionCallback.  Windows that register can supply a default position, but
 * ultimately it is the interaction with this class that determines the position sent in the callback.
 * In other words, a caller into this class can reposition your window using setPosition(), which
 * triggers your RepositionCallback, which triggers the actual update to position.  If position updates
 * are handled outside this flow, then they might be reverted on the next call to setPosition().
 * Therefore, always use HudPositionManager::setPosition() to move your top level widget, relying
 * on the values from the callback scheme.
 *
 * Conceptually the state of the HUD position manager can be serialized and reloaded at a later point.
 * Modification tools such as a mouse editor can be written to manipulate positions in real-time.
 */
class SDKUTIL_EXPORT HudPositionManager : public osg::Referenced
{
public:
  HudPositionManager();

  /** Callback to be defined by windows that has them respond to position changes. */
  class RepositionCallback : public osg::Referenced
  {
  public:
    /** Reposition the window with the given X/Y percentage positions. */
    virtual void setPosition(const std::string& name, const osg::Vec2d& positionPct) = 0;

  protected:
    /** osg::Referenced classes have protected destructors to avoid double delete. */
    virtual ~RepositionCallback() {}
  };

  /**
   * Adds a window with default position.  Adds a reposition callback that gets called when pos needs to update.
   * Use this method once your window (HUD item) is defined and you have a reposition callback.  It is an error
   * to call this method without a reposition callback.  The HUD Position Manager will notify you of correct
   * positioning through the reposition callback.  The correct position may not be the default position that you
   * pass in.  In most cases, you will get a call into this reposition callback before this method finishes.
   * @param name Unique name for your window.
   * @param defaultPositionPct Position for the window that you want to use, in percentages.  If no position was
   *    previously specified for this window from setPosition(), then this position is used.  If a position was
   *    previously specified using setPosition(), then the setPosition()-based position is used.  In either case,
   *    you are notified through the repositioning callback.
   * @param reposCallback Callback to execute when the HUD manager needs to reposition your window.  In most
   *    cases, you will get a callback before this method returns.  This callback is stored in a ref_ptr<> and
   *    is cleaned out after a call to removeWindow().
   */
  void addWindow(const std::string& name, const osg::Vec2d& defaultPositionPct, RepositionCallback* reposCallback);

  /**
   * Removes information about a window.  Position data is retained for future use.  The RepositionCallback
   * supplied in the addWindow() call is cleared out and will no longer be executed, but users can still
   * call getPosition() and setPosition() on the window name.  Window position data is retained in case
   * a window is recreated, or in case of a need to serialize after window destruction.
   * @param name Name of the window to remove.
   * @return 0 on success, non-zero if window does not exist.
   */
  int removeWindow(const std::string& name);

  /** Retrieves the current position for the window with given name.  Returns 0 on success, non-zero on error. */
  int getPosition(const std::string& name, osg::Vec2d& positionPct) const;

  /**
   * Changes the position for the window with given name, calling its RepositionCallback.
   * If the window with the given name is not yet created, the position is saved for later.
   * This position data overrides the default position in the addWindow() call.
   * @param name Name of a window that exists or may exist in the future
   * @param positionPct Position of the window, in range [0,1]
   * @return 0 on success.  It is not an error to pass in a name that does not yet exist.
   */
  int setPosition(const std::string& name, const osg::Vec2d& positionPct);

  /** Resets the given window name to its default position. */
  int resetPosition(const std::string& name);
  /** Resets the position of all windows. */
  void resetAllPositions();

  /** Retrieves a list of all named windows that this GUI knows about.  Returns 0 on success, non-zero on error. */
  int getAllWindows(std::vector<std::string>& names, bool activeOnly) const;

  /** Retrieves the expected size of the window, in pixels (not percentages).  Returns 0 on success, non-zero on error. */
  int getSize(const std::string& name, osg::Vec2d& minXyPx, osg::Vec2d& maxXyPx) const;
  /** Updates the expected size of the window, in pixels (not percentages).  Returns 0 on success, non-zero on error. */
  int setSize(const std::string& name, const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx);

protected:
  /** osg::Referenced classes have protected destructors to avoid double delete. */
  virtual ~HudPositionManager();

private:
  /** Provides a container for information about a particular window. */
  class WindowData : public osg::Referenced
  {
  public:
    explicit WindowData(const std::string& name);

    /** Retrieves the (immutable) name */
    std::string name() const;

    /** Sets the reposition callback for the window.  May be NULL. */
    void setRepositionCallback(HudPositionManager::RepositionCallback* callback);
    /** Retrieves the current reposition callback */
    HudPositionManager::RepositionCallback* repositionCallback() const;

    /** Sets the position, alerting the callback. */
    void setPosition(const osg::Vec2d& positionPct);
    /** Sends the current position to the callback. */
    void emitPosition();
    /** Retrieves position in percentage */
    osg::Vec2d position() const;
    /** Changes the default position, in percentage */
    void setDefaultPosition(const osg::Vec2d& posPct);
    /** Retrieves the default position in percentage */
    osg::Vec2d defaultPosition() const;

    /** Changes size data, which is just metadata about the window */
    void setSize(const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx);
    /** Retrieves size data about the window */
    void getSize(osg::Vec2d& minXyPx, osg::Vec2d& maxXyPx) const;

  protected:
    virtual ~WindowData();

  private:
    const std::string name_;
    osg::Vec2d positionPct_;
    osg::Vec2d defaultPositionPct_;
    osg::Vec2d minXyPx_;
    osg::Vec2d maxXyPx_;
    osg::ref_ptr<HudPositionManager::RepositionCallback> callback_;
  };
  std::map<std::string, osg::ref_ptr<WindowData> > allWindows_;
};

/** Converts pixels to percentages and monitors for window resizes. */
class SDKUTIL_EXPORT RepositionPixelsCallback : public HudPositionManager::RepositionCallback
{
public:
  explicit RepositionPixelsCallback(osg::Node* node);

  /** Template method for derived instances to implement to receive position values in pixels. */
  virtual void setPositionPx(const std::string& name, const osg::Vec2d& positionPx) = 0;

  /** Overrides RepositionCallback to convert the percentages to pixels. */
  virtual void setPosition(const std::string& name, const osg::Vec2d& positionPct);

protected:
  /** osg::Referenced classes have protected destructors to avoid double delete. */
  virtual ~RepositionPixelsCallback();

private:
  class ResizeCallback;
  osg::observer_ptr<ResizeCallback> resizeCb_;
  osg::observer_ptr<osg::Node> cbAttachNode_;
  std::map<std::string, osg::Vec2d> savedPositionsPct_;
};

/** Intended to be used by osg::MatrixTransform users that reposition based on pixel values. */
class SDKUTIL_EXPORT RepositionMatrixPxCallback : public RepositionPixelsCallback
{
public:
  explicit RepositionMatrixPxCallback(osg::MatrixTransform* xform);

  /** Overrides the Reposition callback to set the matrix translate in pixels. */
  virtual void setPositionPx(const std::string& name, const osg::Vec2d& positionPx);

protected:
  /** osg::Referenced classes have protected destructors to avoid double delete. */
  virtual ~RepositionMatrixPxCallback();

private:
  osg::observer_ptr<osg::MatrixTransform> xform_;
};

}

#endif /* SIMUTIL_HUDPOSITIONMANAGER_H */
