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
#include "osgGA/GUIEventAdapter"
#include "osgEarth/EarthManipulator"
#include "simVis/BoxZoomMouseHandler.h"
#include "simVis/EarthManipulator.h"
#include "simVis/View.h"
#include "simVis/NavigationModes.h"

using namespace osgEarth::Util;

namespace simVis {

/** Degrees for minimum pitch (-90 looks straight down) */
static const double MINIMUM_PITCH = -90.0;
/** Degrees for maximum pitch (+90 looks straight up) */
static const double MAXIMUM_PITCH = 85.0; // Cut off the angle a little early to avoid gimbal locks and odd orientations

NavigationMode::NavigationMode()
{
  // configure common settings.

  // actions that will break a tether.
  getBreakTetherActions().push_back(EarthManipulator::ACTION_GOTO);
  getBreakTetherActions().push_back(EarthManipulator::ACTION_PAN);
  getBreakTetherActions().push_back(EarthManipulator::ACTION_EARTH_DRAG);
}

NavigationMode::~NavigationMode()
{
}

NavigationMode::PanOptions::PanOptions()
  : EarthManipulator::ActionOptions()
{
  this->add(EarthManipulator::OPTION_CONTINUOUS, true);
  this->add(EarthManipulator::OPTION_SCALE_Y, -20.0);
  this->add(EarthManipulator::OPTION_SCALE_X, -20.0);
}

NavigationMode::RotateOptions::RotateOptions()
  : EarthManipulator::ActionOptions()
{
  this->add(EarthManipulator::OPTION_CONTINUOUS, true);
  this->add(EarthManipulator::OPTION_SCALE_X, 30.0);
  this->add(EarthManipulator::OPTION_SCALE_Y, -16.0);
}

NavigationMode::ContinuousZoomOptions::ContinuousZoomOptions()
  : EarthManipulator::ActionOptions()
{
  this->add(EarthManipulator::OPTION_CONTINUOUS, true);
  this->add(EarthManipulator::OPTION_SCALE_Y, -6.0);
}

NavigationMode::FixedZoomOptions::FixedZoomOptions()
  : EarthManipulator::ActionOptions()
{
  // should be 0.2, but osgearth seems to drop extra commands
  this->add(EarthManipulator::OPTION_SCALE_Y, 1.0);
}

NavigationMode::IncrementalFixedZoomOptions::IncrementalFixedZoomOptions()
  : EarthManipulator::ActionOptions()
{
  this->add(EarthManipulator::OPTION_SCALE_Y, 0.01);
}

NavigationMode::GoToOptions::GoToOptions()
  : EarthManipulator::ActionOptions()
{
  this->add(EarthManipulator::OPTION_GOTO_RANGE_FACTOR, 1.0);
}

RotatePanNavigationMode::RotatePanNavigationMode(simVis::View* view, bool enableOverhead, bool watchMode)
{
  init_(view, enableOverhead, watchMode);
}

RotatePanNavigationMode::~RotatePanNavigationMode()
{
  if (view_.valid() && boxZoom_.valid())
    view_->removeEventHandler(boxZoom_);
}

void RotatePanNavigationMode::init_(simVis::View* view, bool enableOverhead, bool watchMode)
{
  const bool canRotate = !watchMode && !enableOverhead;
  const bool canZoom = !watchMode;
  view_ = view;

  // left mouse + ctl+shift => box zoom (done with an external event handler)
  if (canZoom && view_.valid())
  {
    EarthManipulator::ActionOptions boxZoomOpts;
    boxZoomOpts.add(EarthManipulator::OPTION_GOTO_RANGE_FACTOR, 1.0);
    boxZoomOpts.add(EarthManipulator::OPTION_DURATION, 1.0);
    boxZoom_ = new BoxZoomMouseHandler(boxZoomOpts);
    // can't use alt + click, since that is stolen by some linux systems for window dragging
    boxZoom_->setModKeyMask(osgGA::GUIEventAdapter::MODKEY_SHIFT | osgGA::GUIEventAdapter::MODKEY_CTRL);
    view_->addEventHandler(boxZoom_);
  }

  // right mouse (or shift left mouse) => globe spin
  bindMouse(EarthManipulator::ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON);
  bindMouse(EarthManipulator::ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT);

  if (enableOverhead)
  {
    // left mouse => continuous pan
    bindMouse(EarthManipulator::ACTION_PAN, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, PanOptions());
    setMinMaxPitch(-90, -90);

    // arrow keys => fixed pan
    bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
    bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
    bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
    bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
  }
  else
  {
    setMinMaxPitch(MINIMUM_PITCH, MAXIMUM_PITCH);
    if (!watchMode)
    {
      // Cannot rotate in watch mode
      assert(canRotate);

      // left mouse => continuous rotate
      bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, RotateOptions());

      // arrow keys => fixed rotate
      bindKey(EarthManipulator::ACTION_ROTATE_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
    }
  }

  // Zooming not permitted in watch mode
  if (canZoom)
  {
    // middle mouse => continuous zoom; Ctl+Alt+Right => continuous zoom
    ContinuousZoomOptions continuousZoomOpts;
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON, 0, continuousZoomOpts);
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_ALT | osgGA::GUIEventAdapter::MODKEY_CTRL, continuousZoomOpts);

    // scroll wheel => fixed zoom
    NavigationMode::FixedZoomOptions fixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, 0, fixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, 0, fixedZoomOpt);

    // scroll wheel + alt => incremental fixed zoom
    NavigationMode::IncrementalFixedZoomOptions incFixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    // bind horizontal scrolling as well, since Qt converts the alt + vertical scroll into a horizontal scroll (and still retains the ALT modifier)
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_RIGHT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_LEFT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
  }

  // both left-double-click and ctrl-left-click center the camera on the mouse pointer
  NavigationMode::GoToOptions goToOpt;
  bindMouseDoubleClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, goToOpt);
  bindMouseClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_CTRL, goToOpt);

  setSingleAxisRotation(true);
}

// ==========================================================================

GlobeSpinNavigationMode::GlobeSpinNavigationMode(bool enableOverhead, bool watchMode)
{
  const bool canRotate = !watchMode && !enableOverhead;
  const bool canZoom = !watchMode;

  // left mouse => globe spin
  bindMouse(EarthManipulator::ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0);

  if (enableOverhead)
  {
    // right mouse (or shift left mouse) => continuous pan
    PanOptions panOpt;
    bindMouse(EarthManipulator::ACTION_PAN, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, panOpt);
    bindMouse(EarthManipulator::ACTION_PAN, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, panOpt);

    setMinMaxPitch(-90, -90);

    // arrow keys => fixed pan
    bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
    bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
    bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
    bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
  }
  else
  {
    setMinMaxPitch(MINIMUM_PITCH, MAXIMUM_PITCH);
    if (!watchMode)
    {
      // Cannot rotate in watch mode
      assert(canRotate);

      // right mouse (or shift left mouse) => continuous rotate
      RotateOptions rotateOpt;
      bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, rotateOpt);
      bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, rotateOpt);


      // arrow keys => fixed rotate
      bindKey(EarthManipulator::ACTION_ROTATE_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
    }
  }

  // Zooming not permitted in watch mode
  if (canZoom)
  {
    // middle mouse => continuous zoom; Ctl+Alt+Right => continuous zoom
    ContinuousZoomOptions continuousZoomOpts;
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON, 0, continuousZoomOpts);
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_ALT | osgGA::GUIEventAdapter::MODKEY_CTRL, continuousZoomOpts);

    // scroll wheel => fixed zoom
    FixedZoomOptions fixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, 0, fixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, 0, fixedZoomOpt);

    // scroll wheel + alt => incremental fixed zoom
    IncrementalFixedZoomOptions incFixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    // bind horizontal scrolling as well, since Qt converts the alt + vertical scroll into a horizontal scroll (and still retains the ALT modifier)
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_RIGHT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_LEFT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
  }

  // both left-double-click and ctrl-left-click center the camera on the mouse pointer
  GoToOptions goToOpt;
  bindMouseDoubleClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, goToOpt);
  bindMouseClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_CTRL, goToOpt);

  setSingleAxisRotation(true);
}

GlobeSpinNavigationMode::~GlobeSpinNavigationMode()
{
}

// ==========================================================================

ZoomNavigationMode::ZoomNavigationMode(bool enableOverhead, bool watchMode)
{
  const bool canRotate = !watchMode && !enableOverhead;
  const bool canZoom = !watchMode;

  if (enableOverhead)
  {
    // shift left mouse => continuous pan
    bindMouse(EarthManipulator::ACTION_PAN, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, PanOptions());
    setMinMaxPitch(-90, -90);

    // arrow keys => fixed pan
    bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
    bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
    bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
    bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
  }
  else
  {
    setMinMaxPitch(MINIMUM_PITCH, MAXIMUM_PITCH);
    if (!watchMode)
    {
      // Cannot rotate in watch mode
      assert(canRotate);

      // shift left mouse => continuous rotate
      bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, RotateOptions());

      // arrow keys => fixed rotate
      bindKey(EarthManipulator::ACTION_ROTATE_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
    }
  }

  // Zooming not permitted in watch mode
  if (canZoom)
  {
    // middle mouse => continuous zoom
    ContinuousZoomOptions contZoomOpt;
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, contZoomOpt);
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON, 0, contZoomOpt);
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, contZoomOpt);

    // scroll wheel => fixed zoom
    FixedZoomOptions fixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, 0, fixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, 0, fixedZoomOpt);

    // scroll wheel + alt => incremental fixed zoom
    IncrementalFixedZoomOptions incFixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    // bind horizontal scrolling as well, since Qt converts the alt + vertical scroll into a horizontal scroll (and still retains the ALT modifier)
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_RIGHT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_LEFT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
  }

  // both left-double-click and ctrl-left-click center the camera on the mouse pointer
  GoToOptions goToOpt;
  bindMouseDoubleClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, goToOpt);
  bindMouseClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_CTRL, goToOpt);

  setSingleAxisRotation(true);
}

ZoomNavigationMode::~ZoomNavigationMode()
{
}

// ==========================================================================

CenterViewNavigationMode::CenterViewNavigationMode(bool enableOverhead, bool watchMode)
{
  const bool canRotate = !watchMode && !enableOverhead;
  const bool canZoom = !watchMode;

  if (enableOverhead)
  {
    // shift left mouse => continuous pan
    bindMouse(EarthManipulator::ACTION_PAN, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, PanOptions());
    setMinMaxPitch(-90, -90);

    // arrow keys => fixed pan
    bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
    bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
    bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
    bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
  }
  else
  {
    setMinMaxPitch(MINIMUM_PITCH, MAXIMUM_PITCH);
    if (!watchMode)
    {
      // Cannot rotate in watch mode
      assert(canRotate);

      // shift left mouse => continuous rotate
      bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, RotateOptions());

      // arrow keys => fixed rotate
      bindKey(EarthManipulator::ACTION_ROTATE_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
      bindKey(EarthManipulator::ACTION_ROTATE_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);
    }
  }

  // Zooming not permitted in watch mode
  if (canZoom)
  {
    // middle mouse => continuous zoom; Ctl+Alt+Right => continuous zoom
    ContinuousZoomOptions continuousZoomOpts;
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON, 0, continuousZoomOpts);
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_ALT | osgGA::GUIEventAdapter::MODKEY_CTRL, continuousZoomOpts);

    // scroll wheel => fixed zoom
    FixedZoomOptions fixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, 0, fixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, 0, fixedZoomOpt);

    // scroll wheel + alt => incremental fixed zoom
    IncrementalFixedZoomOptions incFixedZoomOpt;
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    // bind horizontal scrolling as well, since Qt converts the alt + vertical scroll into a horizontal scroll (and still retains the ALT modifier)
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_RIGHT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_LEFT, osgGA::GUIEventAdapter::MODKEY_ALT, incFixedZoomOpt);
  }

  // left-click, right-click, left-double-click and ctrl-left-click center the camera on the mouse pointer
  GoToOptions goToOpt;
  bindMouseClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, goToOpt);
  bindMouseClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, goToOpt);
  bindMouseDoubleClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, goToOpt);
  bindMouseClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_CTRL, goToOpt);

  setSingleAxisRotation(true);
}

CenterViewNavigationMode::~CenterViewNavigationMode()
{
}

// ==========================================================================

GisNavigationMode::GisNavigationMode(simVis::View* view, bool enableOverhead, bool watchMode)
{
  init_(view, enableOverhead, watchMode);
}

GisNavigationMode::~GisNavigationMode()
{
  if (view_.valid() && boxZoom_.valid())
    view_->removeEventHandler(boxZoom_);
}

void GisNavigationMode::init_(simVis::View* view, bool enableOverhead, bool watchMode)
{
  view_ = view;
  const bool canRotate = !watchMode && !enableOverhead;
  const bool canZoom = !watchMode;

  // left mouse + alt => box zoom (done with an external event handler)
  if (canZoom && view_.valid())
  {
    EarthManipulator::ActionOptions boxZoomOpts;
    boxZoomOpts.add(EarthManipulator::OPTION_GOTO_RANGE_FACTOR, 1.0);
    boxZoomOpts.add(EarthManipulator::OPTION_DURATION, 1.0);
    boxZoom_ = new BoxZoomMouseHandler(boxZoomOpts);
    // can't use alt + click, since that is stolen by some linux systems for window dragging
    boxZoom_->setModKeyMask(osgGA::GUIEventAdapter::MODKEY_SHIFT | osgGA::GUIEventAdapter::MODKEY_CTRL);
    view_->addEventHandler(boxZoom_);
  }

  // Left mouse
  bindMouse(EarthManipulator::ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
  if (canRotate)
    bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT);

  // Note that GOTO will break tether, which will permit zooming, so we can ignore the canZoom
  EarthManipulator::ActionOptions zoomInGoTo;
  zoomInGoTo.add(EarthManipulator::OPTION_GOTO_RANGE_FACTOR, 0.25);
  zoomInGoTo.add(EarthManipulator::OPTION_DURATION, 3.0);
  bindMouseDoubleClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, zoomInGoTo);

  // Right mouse
  if (canZoom)
  {
    EarthManipulator::ActionOptions continuous;
    continuous.add(EarthManipulator::OPTION_CONTINUOUS, 1.0);
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, continuous);
  }
  EarthManipulator::ActionOptions zoomOutGoTo;
  zoomOutGoTo.add(EarthManipulator::OPTION_GOTO_RANGE_FACTOR, 4.0);
  zoomOutGoTo.add(EarthManipulator::OPTION_DURATION, 3.0);
  bindMouseDoubleClick(EarthManipulator::ACTION_GOTO, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, zoomOutGoTo);

  // Middle mouse
  if (canRotate)
  {
    bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON);
  }

  // Scroll wheel
  if (canZoom)
  {
    NavigationMode::FixedZoomOptions wheelDurationScale;
    wheelDurationScale.add(EarthManipulator::OPTION_SCALE_Y, 0.4);
    wheelDurationScale.add(EarthManipulator::OPTION_DURATION, 0.2);
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, 0, wheelDurationScale);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, 0, wheelDurationScale);
  }

  // arrow keys => fixed pan
  bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
  bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
  bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
  bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);

  // alt + arrow keys = move slower
  EarthManipulator::ActionOptions panSlower;
  panSlower.add(EarthManipulator::OPTION_SCALE_X, 0.5);
  panSlower.add(EarthManipulator::OPTION_SCALE_Y, 0.5);
  bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_Left, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);
  bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);
  bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_Up, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);
  bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_Down, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);

  // shift + arrow = rotate around
  if (canRotate)
  {
    bindKey(EarthManipulator::ACTION_ROTATE_LEFT, osgGA::GUIEventAdapter::KEY_Left, osgGA::GUIEventAdapter::MODKEY_SHIFT);
    bindKey(EarthManipulator::ACTION_ROTATE_RIGHT, osgGA::GUIEventAdapter::KEY_Right, osgGA::GUIEventAdapter::MODKEY_SHIFT);
    bindKey(EarthManipulator::ACTION_ROTATE_UP, osgGA::GUIEventAdapter::KEY_Up, osgGA::GUIEventAdapter::MODKEY_SHIFT);
    bindKey(EarthManipulator::ACTION_ROTATE_DOWN, osgGA::GUIEventAdapter::KEY_Down, osgGA::GUIEventAdapter::MODKEY_SHIFT);
  }

  // WASD map to arrow keys
  bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_A, 0);
  bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_D, 0);
  bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_W, 0);
  bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_S, 0);

  // WASD pans slower with alt
  bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_A, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);
  bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_D, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);
  bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_W, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);
  bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_S, osgGA::GUIEventAdapter::MODKEY_ALT, panSlower);

  // shift + WASD = rotate around
  if (canRotate)
  {
    bindKey(EarthManipulator::ACTION_ROTATE_LEFT, osgGA::GUIEventAdapter::KEY_A, osgGA::GUIEventAdapter::MODKEY_SHIFT);
    bindKey(EarthManipulator::ACTION_ROTATE_RIGHT, osgGA::GUIEventAdapter::KEY_D, osgGA::GUIEventAdapter::MODKEY_SHIFT);
    bindKey(EarthManipulator::ACTION_ROTATE_UP, osgGA::GUIEventAdapter::KEY_W, osgGA::GUIEventAdapter::MODKEY_SHIFT);
    bindKey(EarthManipulator::ACTION_ROTATE_DOWN, osgGA::GUIEventAdapter::KEY_S, osgGA::GUIEventAdapter::MODKEY_SHIFT);
  }

  // Set min/max bounds
  if (enableOverhead)
    setMinMaxPitch(-90, -90);
  else
    setMinMaxPitch(MINIMUM_PITCH, MAXIMUM_PITCH);

  setSingleAxisRotation(false);
  setArcViewpointTransitions(true);
  setThrowingEnabled(true);
  setLockAzimuthWhilePanning(false);
}

// ==========================================================================

BuilderNavigationMode::BuilderNavigationMode(bool enableOverhead, bool watchMode)
{
  init_(enableOverhead, watchMode);
}

BuilderNavigationMode::~BuilderNavigationMode()
{
}

void BuilderNavigationMode::init_(bool enableOverhead, bool watchMode)
{
  const bool canRotate = !watchMode && !enableOverhead;
  const bool canZoom = !watchMode;

  // Left mouse
  bindMouse(EarthManipulator::ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
  // Right mouse
  bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON);

  if (canZoom)
  {
    // Scroll wheel
    NavigationMode::FixedZoomOptions wheelDurationScale;
    wheelDurationScale.add(EarthManipulator::OPTION_SCALE_Y, 0.4);
    wheelDurationScale.add(EarthManipulator::OPTION_DURATION, 0.2);
    bindScroll(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN, 0, wheelDurationScale);
    bindScroll(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP, 0, wheelDurationScale);

    // middle mouse => continuous zoom
    ContinuousZoomOptions continuousZoomOpts;
    bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON, 0, continuousZoomOpts);
  }

  if (canRotate)
  {
    // Shift + left mouse == rotate
    bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT);
    // right mouse == rotate
    bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON);
  }

  // arrow keys => fixed pan
  bindKey(EarthManipulator::ACTION_PAN_LEFT, osgGA::GUIEventAdapter::KEY_Left, 0);
  bindKey(EarthManipulator::ACTION_PAN_RIGHT, osgGA::GUIEventAdapter::KEY_Right, 0);
  bindKey(EarthManipulator::ACTION_PAN_UP, osgGA::GUIEventAdapter::KEY_Up, 0);
  bindKey(EarthManipulator::ACTION_PAN_DOWN, osgGA::GUIEventAdapter::KEY_Down, 0);

  if (canZoom)
  {
    // ctrl + up/down = zoom in/out
    bindKey(EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::KEY_Up, osgGA::GUIEventAdapter::MODKEY_CTRL);
    bindKey(EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::KEY_Down, osgGA::GUIEventAdapter::MODKEY_CTRL);
  }

  // ctrl + shift + arrow = rotate around
  if (canRotate)
  {
    const int modkeyMask = osgGA::GUIEventAdapter::MODKEY_CTRL | osgGA::GUIEventAdapter::MODKEY_SHIFT;
    RotateOptions rotateOptions;
    bindKey(EarthManipulator::ACTION_ROTATE_LEFT, osgGA::GUIEventAdapter::KEY_Left, modkeyMask, rotateOptions);
    bindKey(EarthManipulator::ACTION_ROTATE_RIGHT, osgGA::GUIEventAdapter::KEY_Right, modkeyMask, rotateOptions);
    bindKey(EarthManipulator::ACTION_ROTATE_UP, osgGA::GUIEventAdapter::KEY_Up, modkeyMask, rotateOptions);
    bindKey(EarthManipulator::ACTION_ROTATE_DOWN, osgGA::GUIEventAdapter::KEY_Down, modkeyMask, rotateOptions);
  }

  // Set min/max bounds
  if (enableOverhead)
    setMinMaxPitch(-90, -90);
  else
    setMinMaxPitch(MINIMUM_PITCH, MAXIMUM_PITCH);
}

// ==========================================================================

NgtsNavigationMode::NgtsNavigationMode(simVis::View* view, bool enableOverhead, bool watchMode)
  : RotatePanNavigationMode(view, enableOverhead, watchMode)
{
  if (enableOverhead)
    initOverhead_();
  else
    initPerspective_();
}

NgtsNavigationMode::~NgtsNavigationMode()
{
}

void NgtsNavigationMode::initOverhead_()
{
  // Just like normal SIMDIS behavior, except:
  //   Left: Earth Drag
  //   Shift+Left: Continuous Pan
  //   Right: Continuous Zoom
  bindMouse(EarthManipulator::ACTION_ZOOM, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, ContinuousZoomOptions());
  bindMouse(EarthManipulator::ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
  bindMouse(EarthManipulator::ACTION_PAN, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, PanOptions());
}

void NgtsNavigationMode::initPerspective_()
{
  // Just like normal SIMDIS behavior, except:
  //   Left: Earth Drag
  //   Shift+Left: Continuous Rotate
  //   Right: Continuous Rotate
  bindMouse(EarthManipulator::ACTION_EARTH_DRAG, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
  bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::MODKEY_SHIFT, RotateOptions());
  bindMouse(EarthManipulator::ACTION_ROTATE, osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0, RotateOptions());
}

}
