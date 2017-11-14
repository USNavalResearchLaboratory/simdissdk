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
#ifndef SIMVIS_POPUPS_H
#define SIMVIS_POPUPS_H

/**@file
* Pop-ups are little transient text windows that "pop up" when the user
* mouses over a platform.
*/
#include "simCore/Common/Common.h"
#include "simVis/Platform.h"
#include "simVis/Types.h"

#include "osg/Geometry"
#include "osg/MatrixTransform"
#include "osgGA/GUIEventHandler"
#include "osgEarth/Revisioning"
#include "osgEarthSymbology/Symbol"
#include "osgEarthUtil/Controls"

namespace simVis
{
  class PlatformNode;
  class SceneManager;
  class ScenarioManager;
  class View;
  class Picker;

  /**
  * A platform mouse-over pop up control. Using the PopupHandler, you can
  * display this pop up when hovering the mouse over a platform in the scene.
  */
  class SDKVIS_EXPORT PlatformPopup : public osgEarth::Util::Controls::VBox
  {
  public:
    /**
    * Constructs a new platform pop up control
    */
    PlatformPopup();

    /**
    * Sets the title text of the pop up
    * @param str Text string
    */
    void setTitle(const std::string& str);

    /**
    * Sets the context text of the pop up that appears under the title
    * @param str Text string
    */
    void setContent(const std::string& str);

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }
    /** Return the class name */
    virtual const char* className() const { return "PlatformPopup"; }

    /** Retrieve the label for the title */
    osgEarth::Util::Controls::LabelControl* titleLabel() const;
    /** Retrieve the label for the content */
    osgEarth::Util::Controls::LabelControl* contentLabel() const;

  protected:
    /// osg::Referenced-derived
    virtual ~PlatformPopup() {}

  private:
    osgEarth::Util::Controls::LabelControl* titleLabel_;
    osgEarth::Util::Controls::LabelControl* contentLabel_;
  };

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
    explicit PopupHandler(SceneManager* scene, View* view = NULL);

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
    void setShowInCorner(bool showInCorner);

    /// Updates popup, depending on if mouse is over a platform in the current view
    void updatePopupFromView(simVis::View* currentView);

  public: // GUIEventHandler interface
    /// internal - override handle() to detect when we draw the popup
    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

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
    osg::observer_ptr<ScenarioManager> scenario_;             ///< ref to the scenario for finding platforms
    osg::observer_ptr<Picker>          picker_;               ///< Picker class that finds platforms
    osg::ref_ptr<PlatformNode>         currentPlatform_;      ///< keep track of current platform
    osg::ref_ptr<PlatformPopup>        popup_;                ///< the popup display
    osg::ref_ptr<PopupContentCallback> contentCallback_;      ///< callback for filling in platform's data in popup
    osg::observer_ptr<View>            view_;                 ///< view where popup should draw
    osgEarth::Revision                 platformLocatorRev_;   ///< current platform's locator revision
    float                              lastMX_;               ///< last stored mouse X position, 0 is left side
    float                              lastMY_;               ///< last stored mouse Y position, 0 is bottom
    bool                               mouseDirty_;           ///< flag indicating if mouse was moved
    bool                               enabled_;              ///< flag indicating if popup should draw
    bool                               showInCorner_;         ///< flag indicating if popup should display in lower right corner, otherwise displays at mouse
    bool                               limitVisibility_;      ///< flag indicating if popup should only display for a limited time (as defined by duration_)

    int                                borderWidth_;          ///< Width of the border in pixels; 0 to turn off
    simVis::Color                      borderColor_;          ///< RGBA color for the outline of popup
    simVis::Color                      backColor_;            ///< RGBA color for background
    simVis::Color                      titleColor_;           ///< RGBA color for the title text
    simVis::Color                      contentColor_;         ///< RGBA color for the content text
    int                                titleFontSize_;        ///< Title font size in points
    int                                contentFontSize_;      ///< Content font size in points
    int                                padding_;              ///< Space between text and border
    int                                childSpacing_;         ///< Space between title and content
    int                                duration_;             ///< Duration in seconds popup should remain visible once shown
    double                             showStartTime_;        ///< Time popup started being shown
  };

} // namespace simVis

#endif // SIMVIS_POPUPS_H

