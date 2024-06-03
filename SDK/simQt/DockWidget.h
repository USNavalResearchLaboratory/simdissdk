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
#ifndef SIMQT_DOCKWIDGET_HH
#define SIMQT_DOCKWIDGET_HH

#include <QDockWidget>
#include <QPointer>
#include <QRect>
#include <QSize>
#include "simCore/Common/Export.h"
#include "simQt/SettingsGroup.h"

class QAction;
class QMainWindow;
class QTabBar;
class QLabel;
class QToolButton;
class QHBoxLayout;

namespace simQt {

class SearchLineEdit;
class BoundBooleanSetting;
class BoundIntegerSetting;

/**
 * DockWidget is a wrapper class around the QDockWidget, to provide a supplemented dock widget.  The
 * DockWidget provides easy access to dockability for the developer (setDockable(), isDockable()) and
 * the end user (dockableAction()).
 *
 * The DockWidget integrates seamlessly with a simQt::Settings to save dockability flags.
 *
 * When tabified, the DockWidget updates the QTabBar with the widget()'s windowIcon(), providing
 * an easier-to-see mechanism for identifying windows.
 *
 * Note that the title must be unique to each DockWidget, since the title is used as the unique objectName,
 * which is necessary for recognizing the instance in the saveState/restoreState, and also for setting
 * the correct icon when tabbed.
 */
class SDKQT_EXPORT DockWidget : public QDockWidget
{
  Q_OBJECT;
  // Features that can be set in Qt Designer
  Q_FLAGS(ExtraFeatures);
  Q_PROPERTY(bool dockable READ isDockable WRITE setDockable);
  Q_PROPERTY(bool searchEnabled READ searchEnabled WRITE setSearchEnabled);
  Q_PROPERTY(bool escapeClosesWidget READ escapeClosesWidget WRITE setEscapeClosesWidget);
  Q_PROPERTY(ExtraFeatures extraFeatures READ extraFeatures WRITE setExtraFeatures);

public:
  /** QSettings key for the disable docking persistent setting */
  static const QString DISABLE_DOCKING_SETTING;
  /** Meta data for the disable docking persistent setting */
  static const simQt::Settings::MetaData DISABLE_DOCKING_METADATA;

  /** Various docking flags, enhancing dockWidgetFeatures */
  enum ExtraDockFlagHint {
    /** Maximize button is visible (resizes floating dock take up entire monitor) */
    DockMaximizeHint = 0x0001,
    /** Restore button is visible (removes maximization) */
    DockRestoreHint = 0x0002,
    /** Restore and Maximize buttons are usable */
    DockMaximizeAndRestoreHint = DockMaximizeHint | DockRestoreHint,
    /** Undock button is visible (removes docked widget from docking area to make it floating) */
    DockUndockHint = 0x0004,
    /** Redock button is visible (puts a floating widget back into its last dock) */
    DockRedockHint = 0x0008,
    /** Undock and redock buttons are usable */
    DockUndockAndRedockHint = DockUndockHint | DockRedockHint,

    /** Search field is shown (equivalent to setSearchEnabled()) */
    DockSearchHint = 0x0010,

    /** Turn off custom Qt styling for title bar; useful for overriding style sheet data */
    DockNoTitleStylingHint = 0x0020,

    /** Escape closes the GUI */
    DockWidgetCloseOnEscapeKey = 0x0040,

    /** Ignore any settings, don't load or save to settings */
    DockWidgetIgnoreSettings = 0x0080
  };
  /** Create a flags enumeration */
  Q_DECLARE_FLAGS(ExtraFeatures, ExtraDockFlagHint);

  /**
   * Create a new Dock Widget.  The title will be non-unique, so be sure to update the title
   * to avoid issues with settings if settings are used.  Variant of polymorphic constructor.
  */
  DockWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());

  /**
   * Create a new Dock Widget.  Variant of polymorphic constructor.
  */
  DockWidget(const QString& title, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());

  /**
   * Create a new Dock Widget.  Variant of polymorphic constructor.
  */
  DockWidget(const QString& title, simQt::Settings* settings, QMainWindow* parent, Qt::WindowFlags flags=Qt::WindowFlags());

  /**
   * Constructor for a new Dock Widget.  Note that this constructor will set the object name based on title parameter.
   * @param title Text to display in dialog header.  Also used as registry key via setObjectName() for saved settings.
   *   If not set, use setObjectName() to define the registry key for settings.
   * @param settings Where to save/restore window geometry information.  If not specified, then settings will be saved
   *   and loaded from a Private section from QSettings.  Note that some features, such as global disable of dockability,
   *   cannot work properly unless this is specified and non-nullptr.
   * @param parent The parent widget.  Recommend this be set to the QMainWindow.  If not set, then
   *   the caller is responsible for calling QMainWindow::addDockWidget() and setFloating() as
   *   needed.  Some features like docking button will not work unless this is the QMainWindow.
   * @param flags Qt window flags
   */
  DockWidget(const QString& title, simQt::Settings* settings, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());

  /** Destructor */
  virtual ~DockWidget();

  /** Override setVisible() to raise() the dock widget when needed */
  virtual void setVisible(bool fl);

  /**
   * Set the widget that the dock widget will display.
   * @param widget The widget that the dock widget will display
   */
  void setWidget(QWidget* widget);

  /*
   * Set a default size for the widget, which serves as a backup if there is no size defined in settings.
   * Must be called before setWidget to take effect.
   * @param defaultSize to define the dock widget's size
   */
  void setDefaultSize(const QSize& defaultSize);

  /**
  * Get the dockable state of the dock widget, if false, docking is disabled. This will always
  * return false if all docking is disabled, regardless of the widget's internal dockable state.
  * @return bool The dockable state of the dock widget, if false, docking is disabled
  */
  bool isDockable() const;

  /** Retrieves the property for whether search is enabled.  Equivalent to checking the flag DockSearchHint. */
  bool searchEnabled() const;

  /** Escape key will close the widget or not.  Equivalent to checking the flag DockWidgetCloseOnEscapeKey. */
  bool escapeClosesWidget() const;

  /**
   * Retrieve the Is-Dockable toggle action.  You may freely enable/disable or setVisible() on
   * this action.  Changing the visibility or enabled state directly impacts the display in the
   * title bar's right click menu.  This action will only "work" if a valid settings object is
   * passed in at DockWidget construction time.
   */
  QAction* isDockableAction() const;

  /** Retrieves the search field, may be nullptr if searching is disabled. */
  SearchLineEdit* searchLineEdit() const;

  /** Adds a widget to the title bar, returning 0 on success */
  int insertTitleBarWidget(int beforeIndex, QWidget* widget);
  /** Adds a widget to the title bar, returning 0 on success */
  int addTitleBarWidget(QWidget* widget);
  /** Retrieves number of widgets in the title bar (not including internal ones) */
  int titleBarWidgetCount() const;

  /**
   * Sets the main window pointer.  The Main Window is required to enable the Dock action in
   * the title bar, and to fix tab icons.  For best results, call this before setWidget().
   * This method is not required if the parent from constructor is a QMainWindow.
   */
  void setMainWindow(QMainWindow* mainWindow);

  /** Retrieves the currently set docking flags (additional features to features()) */
  ExtraFeatures extraFeatures() const;

  /** Provide a custom show() method that also sets focus.  Note that QWidget::show() is not virtual. */
  void show();

  /** Undocks the dock window */
  void undock();

  /** Returns true if all docking is disabled via settings */
  bool allDockingDisabled() const;

  /** Restore layout geometry to default factory state */
  void restoreDefaultLayout();

public Q_SLOTS:

  /**
  * Set the widget's dockable state. If false, docking will be disabled
  * @param dockable  true make widget dockable, false keeps it or sets it to floating
  */
  void setDockable(bool dockable);

  /** Changes whether the search field is enabled (off by default).  Equivalent to setting the DockSearchHint flag. */
  void setSearchEnabled(bool enable);

  /** Sets the visibility of the title bar */
  void setTitleBarVisible(bool show);

  /** Changes whether escape key will close the widget or not.  Equivalent to setting the DockWidgetCloseOnEscapeKey flag. */
  void setEscapeClosesWidget(bool escapeCloses);

  /** Retrieves the currently set docking flags (additional features to features()) */
  void setExtraFeatures(ExtraFeatures features);

  /** Set the point size of the title bar text, which also increases the window icon size proportionately */
  void setTitleBarTextSize(int pointSize);

  /** Set the global settings ptr, which will apply any global settings if ptr is valid */
  void setGlobalSettings(Settings* globalSettings);

Q_SIGNALS:
  /** Emitted after the dock widget is closed. */
  void closedGui();
  /** Emitted when is-dockable flag has changed via setDockable */
  bool isDockableChanged(bool isDockable);

protected Q_SLOTS:
  /** Docks the window (as long as it's dockable) */
  void dock_();
  /** Undocks if necessary and maximizes the window */
  void maximize_();
  /** Undocks if necessary and shows the window unmaximized */
  void restore_();
  /** Undocks the window */
  void undock_();
  /** Update title bar text */
  void updateTitleBarText_();
  /** Update title bar icon */
  void updateTitleBarIcon_();
  /** Need to verify that our docked state is consistent with our allowed areas */
  void verifyDockState_(bool floating);

protected:
  /** Override closeEvent to emit the closedGui signal */
  virtual void closeEvent(QCloseEvent* evt);
  /** Override resizeEvent to update the title bar when maximized */
  virtual void resizeEvent(QResizeEvent* evt);
  /** Override resizeEvent to update the title bar when maximized windows move */
  virtual void moveEvent(QMoveEvent* evt);
  /** Override keyPressEvent() to handle 'escape' key */
  virtual void keyPressEvent(QKeyEvent* evt);
  /** Override show event to focus the window */
  virtual void showEvent(QShowEvent* evt);

  /** Restores the dock widget, placing it in a good position if restoration fails; uses mainWindow_ */
  void restoreFloating_(const QByteArray& geometryBytes);

  /**
   * Save position, size, and docked area to simQt::Settings.
   * Should be the first call in the destructor of the derived class, before any data models are deleted.
   */
  void saveSettings_();

  /** Points to the settings group for this window */
  simQt::SettingsGroupPtr settings_;
  /** Handle to the global settings */
  simQt::Settings* globalSettings_;
  /** Point size of the title bar text */
  int titleBarPointSize_;

  /** Actions for title bar functionality */
  QAction* dockableAction_;
  QAction* restoreAction_;
  QAction* maximizeAction_;
  QAction* dockAction_;
  QAction* undockAction_;
  QAction* closeAction_;

  /** Parent main window */
  QMainWindow* mainWindow_;

private Q_SLOTS:
  /** Applies the window's icon to the tab bar */
  void fixTabIcon_();
  /** Updates the title bar's enabled/visible states */
  void updateTitleBar_();
  /** Schedules a close of the window using a timer (to avoid memory errors) */
  void closeWindow_();

  /** Changes the window title style based on whether the current focus widget is a child. */
  void changeTitleColorsFromFocusChange_(QWidget* oldFocus, QWidget* newFocus);

  /** Enables or disables dockability from the global flag */
  void setGlobalNotDockableFlag_(bool disallowDocking);

  /** Set dock widget border thickness, in pixels */
  void setBorderThickness_(int thickness);

  /** Apply the appropriate global settings from the current globalSettings_ member */
  void applyGlobalSettings_();

private:
  /**
   * Find the QTabBar that contains the DockWidet with this title, fill in its index. This assumes
   * there is only a single QTabBar in the fromBars param that contains this title. If there are
   * multiple QTabBars with this same title, it is possible that the wrong QTabBar will be found.
   * @param fromBars QList of QTabBars to search through
   * @param title QString containing the title to search for
   * @param index Stores index of the returned QTabBar
   * @return QTabBar with the specified title
   */
  QTabBar* findTabWithTitle_(const QList<QTabBar*>& fromBars, const QString& title, int& index) const;
  /**
   * Load the settings from simQt::Settings, resizes and moves widget based on settings
   * Called by setWidget, so normally not called by derived classes
   */
  void loadSettings_();

  /** Returns true when the window is currently maximized */
  bool isMaximized_() const;

  /** Set up the widget */
  void init_();
  /** Creates the title bar, during initialization */
  QWidget* createTitleBar_();
  /** Helper method to create a new tool button assigned with a default action */
  QToolButton* newToolButton_(QAction* defaultAction) const;
  /** Helper to create the stylesheets for the title bar */
  void createStylesheets_();
  /** Updates the title bar colors based on whether we have focus or not; does nothing if DockNoTitleStylingHint is unset */
  void updateTitleBarColors_(bool haveFocus);
  /** Returns true if the given widget is (eventually) a child of "this" */
  bool isChildWidget_(const QWidget* widget) const;
  /** Returns the current path for simQt::Settings, or QSettings if simQt::Settings is unavailable */
  QString path_() const;
  /** Set floating with the specified geometry, falls back to a valid default if geometry is invalid  */
  void setFloatingGeometry_(const QByteArray& geometryBytes);

  /** Install an event filter to capture drag events on the tab button when this widget is docked over or under other dock widgets */
  void installTabEventFilter_(QTabBar* tabBar);
  /** Uninstall the event filter capturing drag events on tab buttons */
  void uninstallTabEventFilter_();

  QToolButton* restoreButton_;
  QToolButton* maximizeButton_;
  QToolButton* dockButton_;
  QToolButton* undockButton_;
  QToolButton* closeButton_;
  QWidget* titleBar_;
  QWidget* noTitleBar_;
  QLabel* titleBarIcon_;
  QLabel* titleBarTitle_;
  QHBoxLayout* titleBarLayout_;
  SearchLineEdit* searchLineEdit_;

  /** Helper class that deals with double click on title bar to maximize */
  class DoubleClickFrame;

  /** Helper class that deals with double click on title bar icon to close */
  class DoubleClickIcon;

  /** Class that caches original icon and provides monochromatic icons matching title color */
  class MonochromeIcon;

  /** Event filter to listen for drag n drop events when the dock widget is tabbed */
  class TabDragDropEventFilter;

  /** Stores the restore icon */
  MonochromeIcon* restoreIcon_;
  /** Stores the maximize icon */
  MonochromeIcon* maximizeIcon_;
  /** Stores the dock icon */
  MonochromeIcon* dockIcon_;
  /** Stores the undock icon */
  MonochromeIcon* undockIcon_;
  /** Stores the close icon */
  MonochromeIcon* closeIcon_;

  /** Save the count of the number of user widgets in titlebar */
  int titleBarWidgetCount_;

  /** Saves the geometry prior to a call to maximize() */
  QRect normalGeometry_;

  /** Various developer-specified flags for the dock widget, similar to but expanding on features() */
  ExtraFeatures extraFeatures_;

  /** Make sure setting are saved before calling the destructor */
  bool settingsSaved_;

  /** Stylesheet to use on the titlebar when focused */
  QString focusStylesheet_;
  /** Stylesheet to use on the titlebar when not focused */
  QString inactiveStylesheet_;

  /** Cached color to use for text and icons in the title bar when we have focus */
  QColor focusTextColor_;
  /** Cached color to use for text and icons in the title bar when we do NOT have focus */
  QColor inactiveTextColor_;
  /** Cache of whether we have focus or not */
  bool haveFocus_;

  /** Flags true when widget is dockable */
  bool isDockable_;

  /** Global flag that can be used to turn off all docking at once. */
  simQt::BoundBooleanSetting* disableAllDocking_;

  /** Setting to update border thickess */
  simQt::BoundIntegerSetting* borderThickness_;

  QPointer<TabDragDropEventFilter> tabDragFilter_;

  /** Default size, fallback if no stored setting size exists */
  QSize defaultSize_;
};

}

#endif
