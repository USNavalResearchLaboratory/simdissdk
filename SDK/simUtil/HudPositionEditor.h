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
#ifndef SIMUTIL_HUDPOSITIONEDITOR_H
#define SIMUTIL_HUDPOSITIONEDITOR_H

#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "osg/Camera"
#include "simCore/Common/Export.h"
#include "simUtil/HudPositionManager.h"
#include "simUtil/MouseManipulator.h"

namespace simUtil {

class MouseDispatcher;
class WindowNodePx;

/**
 * GUI controls for a HUD Editor.  This is intended to be used in tandem with the
 * HudPositionMouseManipulator.  This is the graphical portion of the HudPositionManager
 * and draws windows on-screen with titles on them.
 */
class SDKUTIL_EXPORT HudEditorGui : public osg::Camera
{
public:
  /** Constructs the HUD Editor GUI to reflect windows in the HUD Position Manager. */
  explicit HudEditorGui(simUtil::HudPositionManager* hud);

  /** Returns true if this GUI is visible (non-zero nodemask) */
  bool isVisible() const;
  /** Changes the visibility (node mask) of the GUI.  When going from invisible to visible, positions are updated. */
  void setVisible(bool fl);

  /** Removes all windows and rebuilds them from scratch. */
  void reset();

  /**
   * Updates the position of a single window, pulling the values from the HUD Position Manager.
   * If the window does not exist in our records but does exist in the HUD Position Manager,
   * then the window is created and sized and placed appropriately.
   */
  void updatePosition(const std::string& windowName);
  /** Updates the size of an existing window.  Unlike updatePosition(), does not create windows. */
  int updateSize(const std::string& windowName);

  /** Sets the flag for whether a particular window is selected or not.  Window must exist. */
  void setSelected(const std::string& name, bool selected);

  /**
   * Does a box intersection with given mouse coordinate, returning the window under mouse.
   * @param mousePx Mouse position in pixels
   * @param offsetFromAnchorPx If a window is found, the is the delta value from the anchor
   *   position of that window relative to mouse.  This is useful for doing dragging without
   *   forcing the anchor to the mouse's location.
   * @return Empty string for no intersection, else name of the first window to intersect mouse.
   */
  std::string intersect(const osg::Vec3d& mousePx, osg::Vec3d& offsetFromAnchorPx) const;

  /** Override traverse() to detect screen resizes in FRAME events */
  virtual void traverse(osg::NodeVisitor& nv);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }
  /** Return the class name */
  virtual const char* className() const { return "HudEditorGui"; }

protected:
  /** Prevent double delete from ref_ptr */
  virtual ~HudEditorGui();

private:
  /** Detects resize of screen and repositions and resizes widgets as needed */
  void handleResize_(double width, double height);
  /** Helper method to move a matrix transform to a particular translation in percentage coords */
  void movePercent_(osg::MatrixTransform* xform, const osg::Vec2d& posPct) const;

  /** Points to the HUD manager that the GUI is mirroring */
  osg::observer_ptr<simUtil::HudPositionManager> hud_;

  /** Root node for the various windows */
  osg::ref_ptr<osg::Group> root_;
  /** Background that obscures the main scene by darkening. */
  osg::ref_ptr<osg::MatrixTransform> background_;
  /** Each window pointer, sorted by name */
  std::map<std::string, osg::ref_ptr<WindowNodePx> > windows_;

  /** Most recent screen width (in pixels) */
  double widthPx_;
  /** Most recent screen height (in pixels) */
  double heightPx_;
};

//////////////////////////////////////////////////////////////////////

/**
 * Right click callback interface. Install an implementation of this callback via
 * HudEditorMouse::setRightClickCallback() to receive notifications when a HUD
 * Editor window is right clicked.
 */
class SDKUTIL_EXPORT HudEditorRightClickCallback
{
public:
  virtual ~HudEditorRightClickCallback() {}

  /** Called when a window named windowName is right clicked. */
  virtual void rightClicked(const std::string& windowName) = 0;
};

//////////////////////////////////////////////////////////////////////

/**
 * Mouse Manipulator that ties into a simUtil::MouseDispatcher, which is responsible
 * for intercepting mouse events for use with a HudPositionManager and a HudEditorGui.
 *
 * The mouse is active whenever the HUD GUI is displayed.  When it's displayed, every
 * mouse event gets intercepted (and not passed on) by this manipulator.  This prevents
 * users from moving the underlying OSG/osgEarth scene while the editor is shown.
 */
class SDKUTIL_EXPORT HudEditorMouse : public simUtil::MouseManipulator
{
public:
  /** Constructs a mouse manipulator on the HUD and GUI provided. */
  HudEditorMouse(simUtil::HudPositionManager* hud, simUtil::HudEditorGui* gui);

  /** Mouse button pushed, returns non-zero on handled */
  virtual int push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Mouse button released, returns non-zero on handled */
  virtual int release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Mouse being moved, returns non-zero on handled */
  virtual int move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Mouse being dragged, returns non-zero on handled */
  virtual int drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Mouse button double clicked, returns non-zero on handled */
  virtual int doubleClick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Mouse wheel scrolled, returns non-zero on handled */
  virtual int scroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Frame event, returns non-zero on handled */
  virtual int frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

  // From MouseManipulator
  virtual void activate();
  virtual void deactivate();

  /** Set a pointer to the right click callback to be used by the mouse manipulator. */
  void setRightClickCallback(std::shared_ptr<simUtil::HudEditorRightClickCallback> cb);

private:
  /** Returns the HUD window under the mouse */
  std::string hudUnderMouse_(double xPx, double yPx, osg::Vec3d& mouseOffsetPx) const;
  /** Returns true if in editor mode */
  bool inEditorMode_() const;

  osg::observer_ptr<simUtil::HudPositionManager> hud_;
  osg::observer_ptr<simUtil::HudEditorGui> gui_;
  std::shared_ptr<simUtil::HudEditorRightClickCallback> callback_;
  double widthPx_;
  double heightPx_;
  std::string currentSelection_;
  osg::Vec3d mouseOffsetPx_;
};

//////////////////////////////////////////////////////////////////////

/**
 * Convenience class that ties together the HUD, GUI, and Mouse.  This class serves as a facade
 * for the three classes and manages their interactions appropriately.  The HUD Editor starts
 * as not visible, use setVisible() to show it.
 */
class SDKUTIL_EXPORT HudPositionEditor
{
public:
  /** Constructs the HUD, GUI, and Mouse controls.  GUI is not visible by default. */
  HudPositionEditor();
  /** Destroys all resources. */
  virtual ~HudPositionEditor();

  /** Adds the Editor GUI to hudParent and adds the mouse to the dispatcher. */
  void bindAll(osg::Group& hudParent, simUtil::MouseDispatcher& dispatcher, int weight=-100);

  /** Retrieves the HUD pointer */
  simUtil::HudPositionManager* hud() const;
  /** Retrieves the GUI pointer */
  simUtil::HudEditorGui* gui() const;
  /** Retrieves the Mouse pointer */
  std::shared_ptr<simUtil::HudEditorMouse> mouse() const;

  /** Returns true if visible */
  bool isVisible() const;
  /** Show or hide the HUD Editor */
  void setVisible(bool visible);

  /** Adds a window to the HUD Position Manager */
  void addWindow(const std::string& name, const osg::Vec2d& defaultPositionPct, HudPositionManager::RepositionCallback* reposCallback);
  /** Removes a window from the HUD Position Manager */
  int removeWindow(const std::string& name);
  /** Resizes a window in the HUD Position Manager */
  int setSize(const std::string& name, const osg::Vec2d& minXyPx, const osg::Vec2d& maxXyPx);

  /** Changes a single window's position */
  int setPosition(const std::string& name, const osg::Vec2d& positionPct);
  /** Resets all windows to default locations */
  void resetAllPositions();
  /** Resets the position of a single window */
  int resetPosition(const std::string& name);

private:
  osg::ref_ptr<simUtil::HudPositionManager> hud_;
  osg::ref_ptr<simUtil::HudEditorGui> gui_;
  std::shared_ptr<simUtil::HudEditorMouse> mouse_;
};

}

#endif /* SIMUTIL_HUDPOSITIONEDITOR_H */
