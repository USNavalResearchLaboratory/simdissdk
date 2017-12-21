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
#ifndef SIMVIS_UI_INSET_VIEW_EVENT_HANDLER_H
#define SIMVIS_UI_INSET_VIEW_EVENT_HANDLER_H

#include "simCore/Common/Common.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "osgGA/GUIEventHandler"
#include "osg/MatrixTransform"

namespace simVis
{

/** Event handler for adding insets using the mouse. */
class SDKVIS_EXPORT CreateInsetEventHandler : public osgGA::GUIEventHandler
{
public:
  /**
  * Constructs a new event handler and attaches it to the specified data object.
  * @param[in ] host View for which to process inset events. This must be
  *    a "top-level" view and not an inset itself.
  */
  explicit CreateInsetEventHandler(simVis::View* host);

  /** Gets the view for which this handler is processing inset events. */
  simVis::View* getView();

  /**
  * Sets the mode for allowing/not allowing for the creation of a inset.
  * @param add True mean allow for the adding of an inset
  */
  void setEnabled(bool enabled);

  /** Returns true when Add-Insert mode is active. */
  bool isEnabled() const;

  // osgGA::GUIEventHandler

  /** Manages dragging for creating insets using the mouse */
  virtual bool handle(const osgGA::GUIEventAdapter& evnt, osgGA::GUIActionAdapter& view);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }
  /** Return the class name */
  virtual const char* className() const { return "CreateInsetEventHandler"; }

protected:
  virtual ~CreateInsetEventHandler();

private:
  bool enabled_;  ///< Allows the user the option of creating an inset
  bool newInsetActionInProgress_;  ///< The user is currently creating an inset
  int newInsetX0_;
  int newInsetY0_;

  osg::observer_ptr<simVis::View> host_;
  osg::ref_ptr<osgGA::GUIEventAdapter> mouseDownEvent_;
  osg::observer_ptr<osg::MatrixTransform> rubberBand_;

  void beginNewInsetAction_(int mx, int my);
  void updateNewInsetAction_(int mx, int my);
  void completeNewInsetAction_(int mx, int my);
  void cancelNewInsetAction_();
};

/**
 * Event handler that detects mouse movement and actions then sets the focus on
 * inset views as appropriate.
 */
class SDKVIS_EXPORT InsetViewEventHandler : public osgGA::GUIEventHandler
{
public:

  /** Various actions that can change the view focus. */
  enum FocusActions
  {
    ACTION_CLICK_SCROLL = 0x1,   // click or scroll in a view to give it focus
    ACTION_HOVER        = 0x2,   // hover the mouse over a view to give it focus
    ACTION_TAB          = 0x4    // TAB key to advance focus to the next inset
  };

  /**
  * Constructs a new event handler and attaches it to the specified data object.
  * @param[in ] host View for which to process inset events. This must be
  *    a "top-level" view and not an inset itself.
  */
  explicit InsetViewEventHandler(simVis::View* host);

  /** Gets the view for which this handler is processing inset events. */
  simVis::View* getView();

  /**
  * Sets a mask of FocusActions values that are active for changing the Inset View focus.
  * @param action_mask mask @see FocusActions
  */
  void setFocusActions(int action_mask);

  /**
  * Gets the mask of FocusActions values that are active for changing the Inset View focus.
  * @return FocusActions mask
  */
  int getFocusActions() const;

#ifdef USE_DEPRECATED_SIMDISSDK_API
  /**
  * @deprecated
  * Sets the mode for allowing/not allowing for the creation of a inset.
  * @param add True mean allow for the adding of an inset
  */
  SDK_DEPRECATE(void setAddInsetMode(bool add), "Use simVis::CreateInsetEventHandler instead.");

  /** @deprecated  Returns true when Add-Insert mode is active. */
  SDK_DEPRECATE(bool isAddInsetMode() const, "Use simVis::CreateInsetEventHandler instead.");
#endif

  // osgGA::GUIEventHandler

  /** Manages dragging for creating insets using the mouse */
  virtual bool handle(const osgGA::GUIEventAdapter& evnt, osgGA::GUIActionAdapter& view);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }
  /** Return the class name */
  virtual const char* className() const { return "InsetViewEventHandler"; }

protected:
  virtual ~InsetViewEventHandler();

private:
  int focusActionsMask_;

  osg::observer_ptr<simVis::View> host_;
  osg::ref_ptr<osgGA::GUIEventHandler> focusDetector_;
  osg::ref_ptr<ViewManager::Callback> viewListener_;

#ifdef USE_DEPRECATED_SIMDISSDK_API
  osg::ref_ptr<CreateInsetEventHandler> createInset_;
#endif

  /** Adds the listener to any views as required */
  void ensureViewListenerInstalled_();
};

} // namespace simVis

#endif // SIMVIS_UI_INSET_VIEW_EVENT_HANDLER_H
