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
#ifndef SIMUTIL_HUDPOSITIONMANAGER_H
#define SIMUTIL_HUDPOSITIONMANAGER_H

#include <map>
#include <optional>
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

/** User-selected placement alignment for a HUD window, particularly for simUtil::HudPositionManager. */
enum class HudWindowAlignment
{
  LEFT_TOP = 0,
  LEFT_CENTER,
  LEFT_BOTTOM,
  CENTER_TOP,
  CENTER_CENTER,
  CENTER_BOTTOM,
  RIGHT_TOP,
  RIGHT_CENTER,
  RIGHT_BOTTOM
};

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

    /**
     * Applies a logical-pixel offset to the window position.
     *
     * HudPositionManager calculates this offset from the selected
     * HudWindowAlignment and the window's current bounds. The offset is
     * applied in addition to the position supplied through setPosition().
     * An offset of (0,0) removes any alignment correction.
     *
     * Implementations that position content in logical pixels should override
     * this method and apply the offset without treating it as a user position
     * change. The default implementation is intentionally a no-op for
     * compatibility with existing RepositionCallback implementations.
     *
     * @param name Name of the HUD window receiving the offset.
     * @param offsetPx Additional logical-pixel translation to apply.
     */
    virtual void setAlignmentOffsetPx(const std::string& name, const osg::Vec2d& offsetPx) = 0;

  protected:
    /** osg::Referenced classes have protected destructors to avoid double delete. */
    virtual ~RepositionCallback() = default;
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

  /**
   * Retrieves the effective alignment for a HUD window. The effective alignment
   * is the user override, if one is set, otherwise the widget default. A value
   * of std::nullopt means no alignment is configured and legacy placement
   * behavior applies.
   * @param name Window or HUD element name to query
   * @return Effective alignment, or nullopt if none is configured or the
   *   window does not exist.
   */
  std::optional<HudWindowAlignment> getAlignment(const std::string& name) const;

  /**
   * Retrieves the user alignment override for a HUD window. This does not return the
   * default alignment when no override exists. This is primarily useful for serialization,
   * as there are few cases where externally this value matters otherwise.
   * @param name Window or HUD element name to query
   * @return User alignment override, or nullopt if none is set or the window does not exist.
   */
  std::optional<HudWindowAlignment> getAlignmentOverride(const std::string& name) const;

  /**
   * Sets or clears the user-selected alignment override for a HUD window.
   * Passing std::nullopt clears the override and causes the window to use its
   * default alignment. If no default alignment is configured, this restores
   * legacy placement behavior. As with setPosition(), alignment may be stored
   * before the window is registered.
   * @param name Existing HUD element name
   * @param alignment User alignment override, or nullopt to clear the override
   * @return 0 on success. It is not an error to pass in a name that does not yet exist.
   */
  int setAlignment(const std::string& name, std::optional<HudWindowAlignment> alignment);

  /**
   * Retrieves the default alignment for a HUD window.
   * @param name Window or HUD element name to query
   * @return Default alignment, or nullopt if none is configured or the window does not exist.
   */
  std::optional<HudWindowAlignment> getDefaultAlignment(const std::string& name) const;

  /**
   * Sets the default alignment for a HUD window. If no user alignment override
   * is active, the new default takes effect immediately. As with setPosition(),
   * default alignment may be stored before the window is registered.
   * @param name Existing or future HUD element name
   * @param alignment Default alignment, or nullopt for legacy behavior
   * @return 0 on success
   */
  int setDefaultAlignment(const std::string& name, std::optional<HudWindowAlignment> alignment);

  /**
   * Clears the user alignment override for a window and reapplies its default alignment.
   * @param name Name of the HUD window to reset
   * @return 0 on success, non-zero if the window does not exist
   */
  int resetAlignment(const std::string& name);

  /** Clears user alignment overrides for all HUD windows. */
  void resetAllAlignments();

  /**
   * Resets both the position and alignment override for a HUD window.
   * @param name Name of the HUD window to reset
   * @return 0 on success, non-zero if the window does not exist
   */
  int resetPlacement(const std::string& name);

  /** Resets both positions and alignment overrides for all HUD windows. */
  void resetAllPlacements();

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

    /** Sets the reposition callback for the window.  May be nullptr. */
    void setRepositionCallback(HudPositionManager::RepositionCallback* callback);
    /** Retrieves the current reposition callback */
    HudPositionManager::RepositionCallback* repositionCallback() const;

    /** Sets the position, alerting the callback. */
    void setPosition(const osg::Vec2d& positionPct);
    /** Sends the current position to the callback. */
    void emitPosition();
    /** Returns true if the position has been explicitly initialized. */
    bool hasPosition() const;
    /** Retrieves position in percentage */
    osg::Vec2d position() const;

    /** Changes the default position, in percentage */
    void setDefaultPosition(const osg::Vec2d& posPct);
    /** Retrieves the default position in percentage */
    osg::Vec2d defaultPosition() const;

    /** Changes the user alignment override. */
    void setAlignment(std::optional<HudWindowAlignment> alignment);
    /** Retrieves the effective alignment. */
    std::optional<HudWindowAlignment> alignment() const;
    /** Retrieves the user alignment override, if one exists. */
    std::optional<HudWindowAlignment> alignmentOverride() const;

    /** Changes the default alignment. */
    void setDefaultAlignment(std::optional<HudWindowAlignment> alignment);
    /** Retrieves the default alignment. */
    std::optional<HudWindowAlignment> defaultAlignment() const;

    /** Clears the user alignment override and reapplies the default alignment. */
    void resetAlignment();
    /** Sends the current alignment-derived offset to the callback. */
    void emitAlignmentOffset();

    /**
     * Changes visual bounds relative to the window's native anchor. If an
     * alignment is selected, sends an updated alignment offset callback.
     */
    void setSize(const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx);
    /** Retrieves size data about the window */
    void getSize(osg::Vec2d& minXyPx, osg::Vec2d& maxXyPx) const;

  protected:
    virtual ~WindowData();

  private:
    /** Calculates the offset implied by the effective alignment and current bounds. */
    osg::Vec2d alignmentOffsetPx_() const;

    const std::string name_;
    osg::Vec2d positionPct_;
    osg::Vec2d defaultPositionPct_;
    osg::Vec2d minXyPx_;
    osg::Vec2d maxXyPx_;
    bool hasPosition_ = false;
    std::optional<HudWindowAlignment> defaultAlignment_;
    std::optional<HudWindowAlignment> alignmentOverride_;
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
  void setPosition(const std::string& name, const osg::Vec2d& positionPct) override;
  /**
   * Stores an additional logical-pixel offset and reapplies the current
   * position, if one has been supplied. The offset persists across viewport
   * resizes and is applied by setPosition().
   */
  void setAlignmentOffsetPx(const std::string& name, const osg::Vec2d& offsetPx) override;

protected:
  /** osg::Referenced classes have protected destructors to avoid double delete. */
  virtual ~RepositionPixelsCallback();

private:
  class ResizeCallback;
  /** Position and alignment-offset data for a HUD window. */
  struct SavedPositionData
  {
    osg::Vec2d positionPct;
    osg::Vec2d alignmentOffsetPx;
    bool hasPosition = false;
  };

  /** Retrieves existing saved data or creates a new record for the given window. */
  SavedPositionData& getOrCreateSavedPosition_(const std::string& name);

  osg::observer_ptr<ResizeCallback> resizeCb_;
  osg::observer_ptr<osg::Node> cbAttachNode_;

  /** Saved positioning data, keyed by HUD window name. */
  std::map<std::string, SavedPositionData> savedPositions_;
};

/** Intended to be used by osg::MatrixTransform users that reposition based on pixel values. */
class SDKUTIL_EXPORT RepositionMatrixPxCallback : public RepositionPixelsCallback
{
public:
  explicit RepositionMatrixPxCallback(osg::MatrixTransform* xform);

  /** Overrides the Reposition callback to set the matrix translate in pixels. */
  void setPositionPx(const std::string& name, const osg::Vec2d& positionPx) override;

protected:
  /** osg::Referenced classes have protected destructors to avoid double delete. */
  virtual ~RepositionMatrixPxCallback();

private:
  osg::observer_ptr<osg::MatrixTransform> xform_;
};

}

#endif /* SIMUTIL_HUDPOSITIONMANAGER_H */
