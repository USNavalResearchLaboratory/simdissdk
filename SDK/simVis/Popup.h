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
#ifndef SIMVIS_POPUPS_H
#define SIMVIS_POPUPS_H

/**@file
* Pop-ups are little transient text windows that "pop up" when the user
* mouses over a platform.
*/
#include "simCore/Common/Common.h"
#include "simVis/Types.h"

#include "osg/Geometry"
#include "osg/MatrixTransform"
#include "osgText/Text"
#include "osgGA/GUIEventHandler"
#include "osgEarth/LineDrawable"
#include "osgEarth/Revisioning"
#include "osgEarth/Symbol"

namespace simVis
{
class PlatformNode;
class SceneManager;
class ScenarioManager;
class View;
class Picker;

/**
* Callback for the user to create custom Pop up content.
*/
struct PopupContentCallback : public osg::Referenced
{
  /** Retrieves a string to display to end user for the PlatformNode provided */
  virtual std::string createString(PlatformNode* platform) =0;

protected:
  /// osg::Referenced-derived
  virtual ~PopupContentCallback() {}
};

//////////////////////////////////////////////////////////////////////////////////////////////////

/// Define a screen location to display a popup
enum class PopupLocation
{
  OVER_ENTITY = 0,
  UPPER_LEFT,
  LOWER_LEFT,
  UPPER_RIGHT,
  LOWER_RIGHT
};

/**
* A entity mouse-over pop up control. Using the PopupHandler, you can
* display this pop up when hovering the mouse over a entity in the scene.
*/
class SDKVIS_EXPORT EntityPopup : public osg::MatrixTransform
{
public:
  EntityPopup();

  /// Set the position (in pixels) of the popup. No-op if showing the popup in the corner
  void setPosition(float xPx, float yPx);

  /// Set the title text
  void setTitle(const std::string& content);
  /// Set the content text
  void setContent(const std::string& content);

  /// Retrieve a pointer to the title label
  osgText::Text* titleLabel() const;
  /// Retrieve a pointer to the content label
  osgText::Text* contentLabel() const;

  /// Sets the width of the popup border; set to 0 to turn off
  void setBorderWidth(float borderWidth);
  /// Sets the border color for the popup
  void setBorderColor(const simVis::Color& color);
  /// Sets the background color for the popup
  void setBackgroundColor(const simVis::Color& color);
  /// Sets the width between text and border
  void setPadding(int paddingPx);
  /// Sets the width between title and content
  void setChildSpacing(int spacingPx);
  /// Sets the maximum width of the title and content
  void setMaxWidth(int widthPx);

  /// Sets to show popup in the lower right corner of the view
  SDK_DEPRECATE(void setShowInCorner(bool showInCorner),
    "Use setPopupLocation instead.");
  /// Define the location to display the popup
  void setPopupLocation(PopupLocation location);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }
  /** Return the class name */
  virtual const char* className() const { return "EntityPopup"; }

protected:
  virtual ~EntityPopup();

private:
  /** Responsible for getting window sizes for positioning */
  class WindowResizeHandler : public osgGA::GUIEventHandler
  {
  public:
    explicit WindowResizeHandler(EntityPopup* parent);

    /** Checks for resize events */
    bool virtual handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*) override;
    /** Retrieves the last window size seen */
    osg::Vec2f windowSize() const;

  private:
    osg::Vec2f windowSize_;
    osg::observer_ptr<EntityPopup> parent_;
  };

  /** Initialize the background and outline graphics */
  void initGraphics_();
  /** Update the label positions within the popup */
  void updateLabelPositions_();
  /** Position the popup in the bottom right corner */
  void positionInCorner_();

  osg::ref_ptr<WindowResizeHandler> resizeHandler_;
  osg::ref_ptr<osg::Vec3Array> verts_;
  osg::ref_ptr<osg::Geometry> background_;
  osg::ref_ptr<osgEarth::LineDrawable> outline_;
  osg::ref_ptr<osgText::Text> titleLabel_;
  osg::ref_ptr<osgText::Text> contentLabel_;

  int paddingPx_; ///< Padding (in pixels) between the edge of the popup and the labels
  int spacingPx_; ///< Vertical spacing (in pixels) between title and content labels
  float widthPx_; ///< Width (in pixels) of the popup based on current content
  float heightPx_; ///< Height (in pixels) of the popup based on current content
  PopupLocation location_; ///< Location on screen to display popup
};

/**
* Event handler that checks for mouse-over on platforms and generates pop-ups. The popup will display in the view provided in the constructor,
* otherwise popup will display in the current view the mouse inhabits. Can set the popup to display either at mouse coordinates or in
* the lower right corner
*/
class SDKVIS_EXPORT PopupHandler : public osgGA::GUIEventHandler
{
public:
  /**
    * Constructs a new popup manager, using the Picker instance supplied.  If using
    * an RTT picker, this is more efficient than using the Scene Manager intersection.
    * @param picker Render-to-texture picker instance that provides item under mouse
    * @param view View on which to show the popup
    */
  PopupHandler(Picker* picker, View* view);

  /**
    * Constructs a new pop up manager and attaches it to a scene manager, using the
    * Scenario Manager's find<>() intersection method for picking.
    *
    * This constructor uses a less efficient method for picking.  Consider using the
    * constructor that uses the simVis::Picker.  This constructor may be removed in
    * a future version of the SIMDIS SDK.
    *
    * @param scene Scene under which this object operates
    * @param view View on which to show the popup
    */
  explicit PopupHandler(SceneManager* scene, View* view = nullptr);

  /// set whether pop-ups are enabled (or not)
  void enable(bool v = true);
  /// get current enable status
  bool isEnabled() const;
  /// remove the current pop up, if any
  void clear();

  /// Sets the width of the popup border; set to 0 to turn off
  void setBorderWidth(int borderWidth);
  /// Sets the border color for the popup
  void setBorderColor(const simVis::Color& color);
  /// Sets the background color for the popup
  void setBackColor(const simVis::Color& color);
  /// Sets the title color for the popup
  void setTitleColor(const simVis::Color& color);
  /// Sets the content color for the popup
  void setContentColor(const simVis::Color& color);
  /// Sets the width of the font for the title
  void setTitleFontSize(int size);
  /// Sets the size of the font for the content
  void setContentFontSize(int size);
  /// Sets the width between text and border
  void setPadding(int width);
  /// Sets the width between title and content
  void setChildSpacing(int width);
  /// Sets the maximum width of the title and content
  void setMaxWidth(int widthPx);

  /**
  * Sets a custom callback that will be used to generate the string
  * that goes in the pop up.
  * @param callback Callback that will generate content
  */
  void setContentCallback(PopupContentCallback* callback);

  /// Returns current content callback
  PopupContentCallback* contentCallback() const { return contentCallback_.get(); }

  /// Sets the duration popup should remain visible after being shown, in seconds
  void setDuration(int duration);

  /// Sets whether to display for a limited time once shown, or keep showing until some other state change affects visibility
  void setLimitVisibility(bool limit);

  /// Sets to show popup in the lower right corner of the view
  SDK_DEPRECATE(void setShowInCorner(bool showInCorner),
    "Use setPopupLocation instead.");
  /// Define the location to display the popup
  void setPopupLocation(PopupLocation location);

  /// Updates popup, depending on if mouse is over a platform in the current view
  void updatePopupFromView(simVis::View* currentView);

public: // GUIEventHandler interface
  /// internal - override handle() to detect when we draw the popup
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "PopupHandler"; }

private: // methods
  virtual ~PopupHandler(); // reference counted object

  /** Syncs the popup to our internal settings */
  void applySettings_();

  /** Initializes variables to default. */
  void init_();

private:
  osg::observer_ptr<ScenarioManager> scenario_; ///< ref to the scenario for finding platforms
  osg::observer_ptr<Picker> picker_; ///< Picker class that finds platforms
  osg::observer_ptr<EntityNode> currentEntity_; ///< keep track of current entity
  osg::ref_ptr<PopupContentCallback> contentCallback_; ///< callback for filling in platform's data in popup
  osg::ref_ptr<EntityPopup> popup_; ///< managed EntityPopup
  osg::observer_ptr<View> view_; ///< view where popup should draw
  osgEarth::Revision entityLocatorRev_; ///< current platform's locator revision
  float lastMX_; ///< last stored mouse X position, 0 is left side
  float lastMY_; ///< last stored mouse Y position, 0 is bottom
  bool mouseDirty_; ///< flag indicating if mouse was moved
  bool enabled_; ///< flag indicating if popup should draw
  PopupLocation location_;
  bool limitVisibility_; ///< flag indicating if popup should only display for a limited time (as defined by duration_)
  int borderWidth_; ///< Width of the border in pixels; 0 to turn off
  simVis::Color borderColor_; ///< RGBA color for the outline of popup
  simVis::Color backColor_; ///< RGBA color for background
  simVis::Color titleColor_; ///< RGBA color for the title text
  simVis::Color contentColor_; ///< RGBA color for the content text
  int titleFontSize_; ///< Title font size in points
  int contentFontSize_; ///< Content font size in points
  int padding_; ///< Space between text and border
  int childSpacing_; ///< Space between title and content
  int duration_; ///< Duration in seconds popup should remain visible once shown
  int maxWidth_; ///< Maximum width in pixels of the popup title and content
  double showStartTime_; ///< Time popup started being shown
  bool installed_; ///< True when the EntityPopup is installed in the view
};

} // namespace simVis

#endif // SIMVIS_POPUPS_H

