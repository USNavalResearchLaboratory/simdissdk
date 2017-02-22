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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMUTIL_PLATFORMPOPUPMANIPULATOR_H
#define SIMUTIL_PLATFORMPOPUPMANIPULATOR_H

#include "osg/ref_ptr"
#include "osgGA/GUIEventHandler"
#include "simVis/Popup.h"
#include "simVis/View.h"
#include "simUtil/MouseManipulator.h"

namespace simUtil
{

/**
 * Event handler that converts a simVis::PopupHandler to a MouseManipulator.
 * Passes the mouse move events from the MouseDispatcher to the simVis::PopupHandler.
 */
class SDKUTIL_EXPORT PlatformPopupManipulator : public MouseManipulatorAdapter
{
public:
  /**
  * Constructs a new simVis::PopupHandler and registers an event handler with the supplied view
  * @param scene Scene manager under which this object operates
  * @param view where the popup should be displayed
  */
  PlatformPopupManipulator(simVis::SceneManager& scene, simVis::View& view);
  virtual ~PlatformPopupManipulator();

  /// set whether pop-ups are enabled (or not)
  void enable(bool v = true);
  /// get current enable status
  bool isEnabled() const;
  /// remove the current pop up, if any
  void clear();
  /// set whether to show popup in the lower right corner of the view
  void setShowInCorner(bool showInCorner);

  /// Retrieve the non-NULL pointer to the popup handler
  simVis::PopupHandler* popupHandler() const;

public: // MouseManipulator interface
  // internal - override
  virtual int move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

private:
  /// timer class to listen for FRAME events to update the popup
  class FrameTimer;

  /// updates the popupHandler to check the last mouse view
  void updatePopupHandler_();

  osg::ref_ptr<simVis::PopupHandler>    popupHandler_;   ///< manages the popup graphic
  osg::observer_ptr<simVis::View>       lastMouseView_;  ///< last view from the mouse move event
  osg::observer_ptr<simVis::View>       drawView_;       ///< view to use for registering our frame timer
  osg::ref_ptr<osgGA::GUIEventHandler>  frameTimer_;     ///< updates on FRAME events
};

} // namespace simUtil

#endif // SIMUTIL_PLATFORMPOPUPMANIPULATOR_H

