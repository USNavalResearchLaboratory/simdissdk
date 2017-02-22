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
 * 2013 - U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */

#ifndef SIMQT_DOCKWIDGET_HH
#define SIMQT_DOCKWIDGET_HH

#include <QDockWidget>
#include <QRect>
#include "simQt/SettingsGroup.h"
#include "simCore/Common/Export.h"

class QAction;
class QMainWindow;
class QTabBar;
class QLabel;
class QToolButton;
class QHBoxLayout;

namespace simQt {

class SearchLineEdit;
class BoundBooleanSetting;

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
class SDKQT_EXPORT DockWidget : public QDockWidget // QDESIGNER_WIDGET_EXPORT
{
  Q_OBJECT;
  // Features that can be set in Qt Designer
  Q_FLAGS(ExtraFeatures);
  Q_PROPERTY(bool Dockable READ isDockable WRITE setDockable);
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
    DockNoTitleStylingHint = 0x0020
  };
  /** Create a flags enumeration */
  Q_DECLARE_FLAGS(ExtraFeatures, ExtraDockFlagHint);

  /**
  * Constructor, for use with QtDesigner.  The title and registryGroup will be
  * non-unique if this constructor is called, so user should update the title
  * and/or the registryGroup when using this constructor
  * @param parent  The parent widget
  */
  DockWidget(QWidget* parent=NULL);

  /**
  * Constructor
  * This updates the QObject objectName, which is formed by registryGroup + title
  * @param title  text to display in dialog header
  * @param settings Where to save/restore window geometry info
  * @param parent  The parent main window
  */
  DockWidget(const QString& title, simQt::Settings* settings, QMainWindow* parent=NULL);

  /** Destructor */
  virtual ~DockWidget();

  /** Override setVisible() to raise() the dock widget when needed */
  virtual void setVisible(bool fl);

  /**
  * Set the widget that the dock widget will display.  Will restore splitter location and
  * column widths from simQt::Settings.
  * @param widget The widget that the dock widget will display
  */
  void setWidget(QWidget* widget);

  /**
  * Get the dockable state of the dock widget, if false, docking is disabled
  * @return bool The dockable state of the dock widget, if false, docking is disabled
  */
  bool isDockable() const;

  /** Retrieves the property for whether search is enabled. */
  bool searchEnabled() const;

  /** Escape key will close the widget or not */
  bool escapeClosesWidget() const;

  /**
   * Retrieve the Is-Dockable toggle action.  You may freely enable/disable or setVisible() on
   * this action.  Changing the visibility or enabled state directly impacts the display in the
   * title bar's right click menu.  This action will only "work" if a valid settings object is
   * passed in at DockWidget construction time.
   */
  QAction* isDockableAction() const;

  /** Retrieves the search field, may be NULL if searching is disabled. */
  SearchLineEdit* searchLineEdit() const;

  /** Adds a widget to the title bar, returning 0 on success */
  int insertTitleBarWidget(int beforeIndex, QWidget* widget);
  /** Adds a widget to the title bar, returning 0 on success */
  int addTitleBarWidget(QWidget* widget);
  /** Retrieves number of widgets in the title bar (not including internal ones) */
  int titleBarWidgetCount() const;

  /** Retrieves the currently set docking flags (additional features to features()) */
  ExtraFeatures extraFeatures() const;

public slots:

  /**
  * Set the widget's dockable state. If false, docking will be disabled
  * @param dockable  true make widget dockable, false keeps it or sets it to floating
  */
  void setDockable(bool dockable);

  /** Changes whether the search field is enabled (off by default) */
  void setSearchEnabled(bool enable);

  /** Sets the visibility of the title bar */
  void setTitleBarVisible(bool show);

  /** Changes whether escape key will close the widget or not */
  void setEscapeClosesWidget(bool escapeCloses);

  /** Retrieves the currently set docking flags (additional features to features()) */
  void setExtraFeatures(ExtraFeatures features);

signals:
  /** Emitted after the dock widget is closed. */
  void closedGui();

protected:
  /** Override keyPressEvent() to handle 'escape' key */
  virtual void keyPressEvent(QKeyEvent* evt);
  /** Override show event to focus the window */
  virtual void showEvent(QShowEvent* evt);

protected slots:
  /** Docks the window (as long as it's dockable) */
  void dock_();
  /** Undocks if necessary and maximizes the window */
  void maximize_();
  /** Undocks if necessary and shows the window unmaximized */
  void restore_();
  /** Undocks the window */
  void undock_();
  /**
  * Update the disableDocking state. If true, docking capability will be disabled, while false will restore
  * the widget to the isDockable state. It will not restore previously docked widgets to their docked location
  * This overrides the effects of setDockable. Note that isDockable will not reflect the overridden state
  * @param dockable  False restores the isDockable state, true disables docking capability and makes window float
  */
  void setDisableDocking_(bool disable);
  /** Need to verify that our docked state is consistent with our allowed areas */
  void verifyDockState_(bool floating);

protected:
  /**
  * Load the settings from simQt::Settings, resizes and moves widget based on settings
  * Called by setWidget, so normally not called by derived classes
  */
  void loadSettings_();

  /**
  * Save position, size, and docked area to simQt::Settings
  * Should be the first call in the destructor of the derived class, before any data models are deleted
  */
  void saveSettings_();

  /** Override closeEvent to emit the closedGui signal */
  virtual void closeEvent(QCloseEvent* event);
  /** Override resizeEvent to update the title bar when maximized */
  virtual void resizeEvent(QResizeEvent* event);
  /** Override resizeEvent to update the title bar when maximized windows move */
  virtual void moveEvent(QMoveEvent* event);
  /** Determine whether the dock widget should respect the disableDocking_ override setting */
  void setRespectDisableDockingSetting_(bool respectDisableDockingSetting);
  /** Update title bar text */
  void updateTitleBarText_();
  /** Update title bar icon */
  void updateTitleBarIcon_();

  /** Points to the settings group for this window */
  simQt::SettingsGroupPtr settings_;
  /** Handle to the global settings */
  simQt::Settings* globalSettings_;

  /** Actions for title bar functionality */
  QAction* dockableAction_;
  QAction* restoreAction_;
  QAction* maximizeAction_;
  QAction* dockAction_;
  QAction* undockAction_;
  QAction* closeAction_;

  /** Parent main window */
  QMainWindow* mainWindow_;

  /** Setting for whether this window can be docked; tied to QSettings */
  BoundBooleanSetting* isDockable_;
  /** Setting to disable docking capability; tied to QSettings */
  BoundBooleanSetting* disableDocking_;

private slots:
  /** Applies the window's icon to the tab bar */
  void fixTabIcon_();
  /** Updates the title bar's enabled/visible states */
  void updateTitleBar_();
  /** Schedules a close of the window using a timer (to avoid memory errors) */
  void closeWindow_();

  /** Changes the window title style based on whether the current focus widget is a child. */
  void changeTitleColorsFromFocusChange_(QWidget* oldFocus, QWidget* newFocus);

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

  /** Returns the current path for simQt::Settings */
  QString path_() const;

  /** Flag to indicate if the dock widget should respect the disableDocking_ setting */
  bool respectDisableDockingSetting_;

  /** Flags true or false depending on whether Escape key will close the widget */
  bool escapeClosesWidget_;

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

  /** Class that caches original icon and provides monochromatic icons matching title color */
  class MonochromeIcon;
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
};

}

#endif
