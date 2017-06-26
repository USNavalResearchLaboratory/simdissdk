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
#include <iostream>
#include <cassert>
#include <QTimer>
#include <QAction>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QTabBar>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "simNotify/Notify.h"
#include "simQt/SearchLineEdit.h"
#include "simQt/BoundSettings.h"
#include "simQt/QtConversion.h"
#include "simQt/DockWidget.h"

namespace simQt {

/** QSettings key for the dockable persistent setting */
static const QString DOCKABLE_SETTING = "DockWidgetDockable";
/** QSettings key for geometry, to restore geometry before main window manages the dock widget */
static const QString DOCK_WIDGET_GEOMETRY = "DockWidgetGeometry";
/** Meta data for the dockable persistent setting */
static const simQt::Settings::MetaData DOCKABLE_METADATA = simQt::Settings::MetaData::makeBoolean(
  true, QObject::tr("Toggles whether the window can be docked into the main window or not"),
  simQt::Settings::PRIVATE);

/// Setting that can be used for disabling all docking all at once
const QString DockWidget::DISABLE_DOCKING_SETTING = "Windows/Disable All Docking";
/// Metadata for DISABLE_DOCKING_SETTING
const simQt::Settings::MetaData DockWidget::DISABLE_DOCKING_METADATA = simQt::Settings::MetaData::makeBoolean(
  false, QObject::tr("Disables docking on all windows. Overrides individual windows' dockable state"),
  simQt::Settings::ADVANCED);

/** Index value for the search widget if it exists */
static const int SEARCH_LAYOUT_INDEX = 2;
/** Default docking flags enables all buttons, but not search */
static const DockWidget::ExtraFeatures DEFAULT_EXTRA_FEATURES(
  DockWidget::DockMaximizeAndRestoreHint |
  DockWidget::DockUndockAndRedockHint |
  DockWidget::DockWidgetCloseOnEscapeKey
);

/**
 * Helper that, given an input icon with transparency, will use that icon as a mask to
 * generate new monochrome icons of the same size.
 */
class DockWidget::MonochromeIcon : public QObject
{
public:
  MonochromeIcon(const QIcon& icon, const QSize& size, QObject* parent)
    : QObject(parent),
      icon_(icon),
      size_(size)
  {
  }

  /** Retrieves the original input icon */
  const QIcon& originalIcon() const
  {
    return icon_;
  }

  /** Retrieve the icon in the given color */
  QIcon icon(const QColor& color)
  {
    const QRgb rgba = color.rgba();
    std::map<QRgb, QIcon>::const_iterator i = colorToIcon_.find(rgba);
    if (i != colorToIcon_.end())
      return i->second;

    // Create then save the icon
    QIcon newIcon = createIcon_(color);
    colorToIcon_[rgba] = newIcon;
    return newIcon;
  }

private:
  /** Given a color, will create an icon of size_ that replaces all colors with input color */
  QIcon createIcon_(const QColor& color) const
  {
    QImage result(size_, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter p(&result);
    const QRect iconRect(0, 0, size_.width(), size_.height());
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.fillRect(iconRect, color);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    icon_.paint(&p, iconRect);
    return QIcon(QPixmap::fromImage(result));
  }

  const QIcon icon_;
  const QSize size_;
  std::map<QRgb, QIcon> colorToIcon_;
};

///////////////////////////////////////////////////////////////

/** Intercept double clicks on the frame.  If undocked, then maximize or restore as appropriate */
class DockWidget::DoubleClickFrame : public QFrame
{
public:
  DoubleClickFrame(DockWidget& dockWidget, QWidget* parent=NULL, Qt::WindowFlags flags=0)
    : QFrame(parent, flags),
      dockWidget_(dockWidget)
  {
  }

protected:
  /** Overridden from QFrame */
  virtual void mouseDoubleClickEvent(QMouseEvent* evt)
  {
    // If it's docked we let Qt deal with the message (i.e. it will undock via Qt mechanisms).
    // If it's floating, we intercept and remap to maximize or restore as appropriate
    if (dockWidget_.isFloating())
    {
      if (dockWidget_.isMaximized_())
        dockWidget_.restore_();
      else
        dockWidget_.maximize_();
      evt->accept();
      // Do not pass on to Qt, else we could be forced into a dock
    }
    else
    {
      // Just pass the event down, which will let us undock (or whatever Qt wants to do)
      QFrame::mouseDoubleClickEvent(evt);
    }
  }

private:
  DockWidget& dockWidget_;
};

///////////////////////////////////////////////////////////////

/** Intercept double clicks on the title bar icon.  Closes window on double click */
class DockWidget::DoubleClickIcon : public QLabel
{
public:
  DoubleClickIcon(DockWidget& dockWidget, QWidget* parent = NULL, Qt::WindowFlags flags = 0)
    : QLabel(parent, flags),
    dockWidget_(dockWidget)
  {
  }

protected:
  /** Overridden from QLabel */
  virtual void mouseDoubleClickEvent(QMouseEvent* evt)
  {
    // If upper left corner is double clicked, close window
    dockWidget_.closeWindow_();
    evt->accept();
  }

private:
  DockWidget& dockWidget_;
};

///////////////////////////////////////////////////////////////

DockWidget::DockWidget(QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(parent, flags),
    globalSettings_(NULL),
    mainWindow_(dynamic_cast<QMainWindow*>(parent))
{
  init_();
}

DockWidget::DockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags),
    globalSettings_(NULL),
    mainWindow_(dynamic_cast<QMainWindow*>(parent))
{
  setObjectName(title);
  init_();
}

DockWidget::DockWidget(const QString& title, simQt::Settings* settings, QMainWindow* parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags),
    globalSettings_(settings),
    mainWindow_(parent)
{
  if (settings)
    settings_.reset(new simQt::SettingsGroup(settings, title));
  setObjectName(title);
  init_();
}

DockWidget::DockWidget(const QString& title, simQt::Settings* settings, QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags),
    globalSettings_(settings),
    mainWindow_(dynamic_cast<QMainWindow*>(parent))
{
  if (settings)
    settings_.reset(new simQt::SettingsGroup(settings, title));
  setObjectName(title);
  init_();
}

DockWidget::~DockWidget()
{
  // do not call saveSettings_() here since there could be race conditions on Qt ownership,
  // but make sure it was called before this destructor
  assert(settingsSaved_);

  // Disconnect is required to avoid focus change from triggering updates to color
  disconnect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(changeTitleColorsFromFocusChange_(QWidget*, QWidget*)));

  delete noTitleBar_;
  noTitleBar_ = NULL;
  delete titleBar_;
  titleBar_ = NULL;
}

void DockWidget::init_()
{
  searchLineEdit_ = NULL;
  titleBarWidgetCount_ = 0;
  extraFeatures_ = DEFAULT_EXTRA_FEATURES;
  settingsSaved_ = (settings_ == NULL);  // Prevent false asserts when the simQt::Settings is not provided in construction
  haveFocus_ = false;
  isDockable_ = true;
  disableAllDocking_ = NULL;

  createStylesheets_();

  // Several circumstances require a fix to the tab icon
  connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(fixTabIcon_()));
  connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(fixTabIcon_()));
  connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(verifyDockState_(bool)));

  setAllowedAreas(Qt::AllDockWidgetAreas);

  // Can-be-docked
  dockableAction_ = new QAction(tr("Dockable"), this);
  dockableAction_->setCheckable(true);
  dockableAction_->setChecked(isDockable_);
  dockableAction_->setToolTip(formatTooltip(tr("Dockable"), tr("Window may be docked to main window")));
  connect(dockableAction_, SIGNAL(toggled(bool)), this, SLOT(setDockable(bool)));

  // Separator
  QAction* sep = new QAction(this);
  sep->setSeparator(true);

  // Maximize
  maximizeAction_ = new QAction(tr("Maximize"), this);
  maximizeAction_->setToolTip(formatTooltip(tr("Maximize"), tr("Expand window to maximum size")));
  maximizeAction_->setIcon(QIcon(":/simQt/images/Maximize.png"));
  connect(maximizeAction_, SIGNAL(triggered()), this, SLOT(maximize_()));

  // Restore
  restoreAction_ = new QAction(tr("Restore"), this);
  restoreAction_->setToolTip(formatTooltip(tr("Restore"), tr("Restore window to original size")));
  restoreAction_->setIcon(QIcon(":/simQt/images/Restore.png"));
  connect(restoreAction_, SIGNAL(triggered()), this, SLOT(restore_()));

  // Dock
  dockAction_ = new QAction(tr("Dock"), this);
  dockAction_->setToolTip(formatTooltip(tr("Dock"), tr("Dock the window to the main window")));
  dockAction_->setIcon(QIcon(":/simQt/images/Dock.png"));
  connect(dockAction_, SIGNAL(triggered()), this, SLOT(dock_()));

  // Undock
  undockAction_ = new QAction(tr("Undock"), this);
  undockAction_->setToolTip(formatTooltip(tr("Undock"), tr("Undock the window from the main window")));
  undockAction_->setIcon(QIcon(":/simQt/images/Undock.png"));
  connect(undockAction_, SIGNAL(triggered()), this, SLOT(undock_()));

  // Close
  closeAction_ = new QAction(tr("Close"), this);
  closeAction_->setToolTip(formatTooltip(tr("Close"), tr("Close the window")));
  closeAction_->setIcon(QIcon(":/simQt/images/Close.png"));
  connect(closeAction_, SIGNAL(triggered()), this, SLOT(closeWindow_()));
  closeAction_->setShortcuts(QKeySequence::Close);

  // Create the monochrome icons for doing focus
  const QSize titleBarIconSize(8, 8);
  maximizeIcon_ = new MonochromeIcon(maximizeAction_->icon(), titleBarIconSize, this);
  restoreIcon_ = new MonochromeIcon(restoreAction_->icon(), titleBarIconSize, this);
  dockIcon_ = new MonochromeIcon(dockAction_->icon(), titleBarIconSize, this);
  undockIcon_ = new MonochromeIcon(undockAction_->icon(), titleBarIconSize, this);
  closeIcon_ = new MonochromeIcon(closeAction_->icon(), titleBarIconSize, this);

  // Create the title bar once all the actions are created
  titleBar_ = createTitleBar_();
  // Create our non-visible title bar widget
  noTitleBar_ = new QWidget();
  noTitleBar_->setMinimumSize(1, 1);

  // Turn on the title bar
  setTitleBarWidget(titleBar_);
  // When floating changes, update the title bar
  connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(updateTitleBar_()));
  // Start with a known good state
  updateTitleBar_();

  // By default use actions() for popup on the title bar
  titleBar_->setContextMenuPolicy(Qt::ActionsContextMenu);
  titleBar_->addAction(dockableAction_);
  titleBar_->addAction(sep);
  titleBar_->addAction(maximizeAction_);
  titleBar_->addAction(restoreAction_);
  titleBar_->addAction(dockAction_);
  titleBar_->addAction(undockAction_);
  titleBar_->addAction(sep);
  titleBar_->addAction(closeAction_);

  connect(this, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)), this, SLOT(updateTitleBar_()));
  connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(changeTitleColorsFromFocusChange_(QWidget*, QWidget*)));

  // Set a consistent focus
  updateTitleBarColors_(false);
}

void DockWidget::createStylesheets_()
{
  const QString ssTemplate =
    "#titleBar {background: rgb(%1,%2,%3); border: 1px solid rgb(%7,%8,%9);} "
    "#titleBarTitle {color: rgb(%4,%5,%6);} "
    ;

  const QPalette& pal = palette();
  const QColor inactiveBackground = pal.color(QPalette::Inactive, QPalette::Highlight);
  inactiveTextColor_ = pal.color(QPalette::Inactive, QPalette::HighlightedText);
  const QColor darkerInactiveBg = inactiveBackground.darker();

  // Get the focus colors
  const QColor focusBackground = pal.color(QPalette::Active, QPalette::Highlight);
  focusTextColor_ = pal.color(QPalette::Active, QPalette::HighlightedText);
  const QColor darkerFocusBg = focusBackground.darker();

  // Create the inactive stylesheet
  inactiveStylesheet_ = ssTemplate
    .arg(inactiveBackground.red()).arg(inactiveBackground.green()).arg(inactiveBackground.blue())
    .arg(inactiveTextColor_.red()).arg(inactiveTextColor_.green()).arg(inactiveTextColor_.blue())
    .arg(darkerInactiveBg.red()).arg(darkerInactiveBg.green()).arg(darkerInactiveBg.blue());

  // Create the focused stylesheet
  focusStylesheet_ = ssTemplate
    .arg(focusBackground.red()).arg(focusBackground.green()).arg(focusBackground.blue())
    .arg(focusTextColor_.red()).arg(focusTextColor_.green()).arg(focusTextColor_.blue())
    .arg(darkerFocusBg.red()).arg(darkerFocusBg.green()).arg(darkerFocusBg.blue());
}

QWidget* DockWidget::createTitleBar_()
{
  // Create the title bar and set its shape and style information
  QFrame* titleBar = new DoubleClickFrame(*this);
  titleBar->setObjectName("titleBar");
  titleBar->setFrameShape(QFrame::StyledPanel);

  // Create the icon holders
  titleBarIcon_ = new DoubleClickIcon(*this);
  titleBarIcon_->setPixmap(windowIcon().pixmap(QSize(16, 16)));
  titleBarIcon_->setScaledContents(true);
  titleBarIcon_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  // Set the titlebar's caption
  titleBarTitle_ = new QLabel();
  titleBarTitle_->setObjectName("titleBarTitle");
  titleBarTitle_->setText(windowTitle());
  titleBarTitle_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // Note a padding of 0 pixels looks bad, especially on Ubuntu 14
  titleBarTitle_->setContentsMargins(4, 0, 0, 0);

  // Create tool buttons for each button that might show on the GUI
  restoreButton_ = newToolButton_(restoreAction_);
  maximizeButton_ = newToolButton_(maximizeAction_);
  dockButton_ = newToolButton_(dockAction_);
  undockButton_ = newToolButton_(undockAction_);
  closeButton_ = newToolButton_(closeAction_);

  // Create the layout
  titleBarLayout_ = new QHBoxLayout();
  titleBarLayout_->setContentsMargins(5, 0, 0, 0);
  titleBarLayout_->setSpacing(1);
  titleBar->setLayout(titleBarLayout_);

  // Add all the widgets to the layout
  titleBarLayout_->addWidget(titleBarIcon_);
  titleBarLayout_->addWidget(titleBarTitle_);
  titleBarLayout_->addWidget(restoreButton_);
  titleBarLayout_->addWidget(maximizeButton_);
  titleBarLayout_->addWidget(dockButton_);
  titleBarLayout_->addWidget(undockButton_);
  titleBarLayout_->addWidget(closeButton_);

  return titleBar;
}

QToolButton* DockWidget::newToolButton_(QAction* defaultAction) const
{
  QToolButton* rv = new QToolButton();
  rv->setFocusPolicy(Qt::NoFocus);
  rv->setDefaultAction(defaultAction);
  rv->setAutoRaise(true);
  rv->setIconSize(QSize(8, 8));
  return rv;
}

void DockWidget::resizeEvent(QResizeEvent* evt)
{
  QDockWidget::resizeEvent(evt);
  // Resizing the window could make us not maximized
  updateTitleBar_();
}

void DockWidget::moveEvent(QMoveEvent* evt)
{
  QDockWidget::moveEvent(evt);
  // Moving the window could change us from maximized to normal
  updateTitleBar_();
}

void DockWidget::setMainWindow(QMainWindow* mainWindow)
{
  if (mainWindow != mainWindow_)
  {
    mainWindow_ = mainWindow;
    updateTitleBar_();
  }
}

void DockWidget::updateTitleBar_()
{
  const bool floating = isFloating();
  const bool maximized = isMaximized_();
  const bool canFloat = features().testFlag(DockWidgetFloatable);
  const bool canClose = features().testFlag(DockWidgetClosable);

  const bool canMaximize = extraFeatures().testFlag(DockMaximizeHint);
  const bool canRestore = extraFeatures().testFlag(DockRestoreHint);
  const bool canUndock = canFloat && extraFeatures().testFlag(DockUndockHint);
  const bool canRedock = extraFeatures().testFlag(DockRedockHint);
  const bool globalCanDock = !(disableAllDocking_ && disableAllDocking_->value());

  // Maximize.  Docked: Visible if can-float;  Undocked: Visible when not maximized
  maximizeAction_->setVisible(canFloat && !maximized && canMaximize);
  maximizeButton_->setVisible(maximizeAction_->isVisible());

  // Restore.  Docked: Hidden;  Undocked: Visible when maximized
  restoreAction_->setVisible(maximized && floating && canRestore);
  restoreButton_->setVisible(restoreAction_->isVisible());

  // Undock.  Docked: Visible if can-float;  Undocked: Hidden
  undockAction_->setVisible(canFloat && !floating && canUndock);
  undockButton_->setVisible(undockAction_->isVisible());

  // Dock.  Docked: Hidden;  Undocked: Visible
  //        Enabled only if Can-Dock is true AND there is a main window specified
  dockAction_->setVisible(floating && canRedock && globalCanDock);
  dockButton_->setVisible(dockAction_->isVisible());
  dockAction_->setEnabled(isDockable_ && mainWindow_);

  // Closeable
  closeAction_->setVisible(canClose);
  closeButton_->setVisible(closeAction_->isVisible());

  // Dockable
  dockableAction_->setVisible(canFloat);

  // Make sure the pixmap and text are correct
  titleBarIcon_->setPixmap(windowIcon().pixmap(QSize(16, 16)));
  titleBarTitle_->setText(windowTitle());

  // Need to make sure icons are right colors too
  updateTitleBarColors_(haveFocus_);
}

void DockWidget::maximize_()
{
  // If we cannot float, then we need to return early
  if (!features().testFlag(DockWidgetFloatable))
    return;
  // If we're not floating, we need to start floating
  if (!isFloating())
  {
    // ... but not before saving our current geometry as "normal"
    normalGeometry_ = geometry();
    setFloating(true);
  }

  // If already maximized, return
  if (isMaximized_())
    return;

  // Save the 'normal' geometry so when we unmaximize we can return to it
  normalGeometry_ = geometry();

  // Set the window dimensions manually to maximize the available geometry
  QDesktopWidget dw;
  setGeometry(dw.availableGeometry(this));

  // Finally update the state of the enable/disable/visibility
  updateTitleBar_();
}

void DockWidget::restore_()
{
  // If we cannot float, then we need to return early
  if (!features().testFlag(DockWidgetFloatable))
    return;
  // If we're not floating, we need to start floating
  if (!isFloating())
  {
    // Grab the geometry before we float, so we don't float into a maximized state
    normalGeometry_ = geometry();
    setFloating(true);
  }

  // We already have a saved decent geometry, restore to it
  setGeometry(normalGeometry_);

  // Finally update the state of the enable/disable/visibility
  updateTitleBar_();
}

void DockWidget::dock_()
{
  // Don't re-dock if it's already docked, OR if the user wants this to be undockable
  if (!isFloating() || !isDockable_)
    return;
  // If the global flag is available and set to disallow, then return
  if (disableAllDocking_ && disableAllDocking_->value())
    return;
  setFloating(false);

  // In some cases, setFloating() may fail to redock.  In these cases, we may need
  // to request a valid dock from the main window.
  if (isFloating() && mainWindow_ != NULL)
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);

  // Finally update the state of the enable/disable/visibility
  updateTitleBar_();
}

void DockWidget::undock_()
{
  if (isFloating() && !features().testFlag(DockWidgetFloatable))
    return;

  // Save the normal geometry state here too, just in case we undock to maximized
  normalGeometry_ = geometry();
  setFloating(true);
  updateTitleBar_();
}

void DockWidget::closeWindow_()
{
  // Fire off a timer to close.  Don't close immediately because this slot might have
  // been called from a popup, which would need to clean up before closing commences.
  // Without this, the window may close with the popup still active, causing a crash
  // as the popup closes later.
  QTimer::singleShot(0, this, SLOT(close()));
}

void DockWidget::fixTabIcon_()
{
  // Break out early if we're floating, or if there's no main window
  if (isFloating() || !mainWindow_)
    return;

  // Return early if this dock widget is not tabified
  QList<QDockWidget*> tabifiedWidgets = mainWindow_->tabifiedDockWidgets(this);
  if (tabifiedWidgets.empty())
    return;

  // Tabified, now set icon to tab
  // First, find all the tab bars, since QMainWindow doesn't provide
  // direct access to the DockArea QTabBar
  QList<QTabBar*> tabBars = mainWindow_->findChildren<QTabBar*>();

  // Locate the tab bar that contains this window, based on the window title
  int index = 0;
  QTabBar* tabBar = findTabWithTitle_(tabBars, windowTitle(), index);
  if (tabBar == NULL)
    return;

  // This title matches ours, set the tab icon
  tabBar->setTabIcon(index, widget()->windowIcon());

  // Here is a special case, the initial tabification, we are making the other widget become tabified as well
  // need to set their tab icon, since there is no other way to alert them they are becoming tabified
  if (tabifiedWidgets.size() == 1)
  {
    // index for other tab is 0 or 1, whichever is not ours
    int newIndex = (index == 1 ? 0 : 1);
    // Set icon from our only other tabified widget
    DockWidget* firstTab = dynamic_cast<DockWidget*>(tabifiedWidgets[0]);
    if (firstTab != NULL && firstTab->windowTitle() == tabBar->tabText(newIndex))
    {
      tabBar->setTabIcon(newIndex, firstTab->widget()->windowIcon());
    }
  }
}

void DockWidget::setTitleBarVisible(bool show)
{
  // if visible, may need to set title bar
  if (show)
  {
    if (titleBarWidget() != titleBar_)
      setTitleBarWidget(titleBar_);
  }
  else
  {
    if (titleBarWidget() != noTitleBar_)
      setTitleBarWidget(noTitleBar_);
    noTitleBar_->hide();
  }

  if (titleBar_->isVisible() != show)
  {
    titleBar_->setVisible(show);
    haveFocus_ = isChildWidget_(QApplication::focusWidget());
    updateTitleBarColors_(haveFocus_);
  }
}

void DockWidget::updateTitleBarText_()
{
  titleBarTitle_->setText(windowTitle());
}

void DockWidget::updateTitleBarIcon_()
{
  titleBarIcon_->setPixmap(windowIcon().pixmap(QSize(16, 16)));
}

void DockWidget::setVisible(bool fl)
{
  // Overridden in order to raise the window (makes tabs active)
  QDockWidget::setVisible(fl);
  if (fl)
    raise();
}

void DockWidget::closeEvent(QCloseEvent* event)
{
  QDockWidget::closeEvent(event);
  emit(closedGui());
}

void DockWidget::setWidget(QWidget* widget)
{
  // Deal with settings -- restore the is-dockable setting
  if (!objectName().isEmpty())
  {
    if (!settings_)
    {
      QSettings settings;
      setDockable(settings.value(path_() + DOCKABLE_SETTING, true).toBool());
    }
    else
    {
      setDockable(settings_->value(path_() + DOCKABLE_SETTING, DOCKABLE_METADATA).toBool());
    }
  }

  QDockWidget::setWidget(widget);
  if (widget == NULL)
    return;
  setWindowIcon(widget->windowIcon());

  // Save the geometry now so that we have some valid value at initialization
  normalGeometry_ = geometry();

  // Call load settings here, since the DockWidget is just a frame around the widget.
  // We call here because settings don't make much sense until there's an underlying widget inside.
  if (settings_)
  {
    // loadSettings_ will pull out the last geometry as needed, and restore floating state
    loadSettings_();
  }
  else
  {
    normalGeometry_ = geometry();
    restoreFloating_(QByteArray());
  }

  // Schedule a fix to the tabs, if it starts up tabified
  if (!isFloating())
    QTimer::singleShot(0, this, SLOT(fixTabIcon_()));
}

bool DockWidget::isDockable() const
{
  return isDockable_;
}

void DockWidget::setDockable(bool dockable)
{
  // Note: Intentionally not doing early-out here because value may match but
  // we may need to do work, since changing disable-all-docking can eventually
  // call this method.

  // Override the dockability flag with the global if needed
  const bool globalDockDisable = (disableAllDocking_ != NULL && disableAllDocking_->value());
  if (globalDockDisable)
    dockable = false;

  // Update settings and QMenu's QAction
  bool emitIt = (dockable != isDockable_);
  isDockable_ = dockable;

  // only set dockable if we can be dockable
  if (dockable)
    setAllowedAreas(Qt::AllDockWidgetAreas);
  else
  {
    // make sure we float in case we are currently docked
    if (!isFloating())
      setFloating(true);
    setAllowedAreas(Qt::NoDockWidgetArea);
  }

  updateTitleBar_();
  if (dockableAction_->isChecked() != dockable)
    dockableAction_->setChecked(dockable);
  if (emitIt)
    emit isDockableChanged(isDockable_);
}

void DockWidget::verifyDockState_(bool floating)
{
  // there are cases where Qt will dock this widget despite the allowedAreas, e.g. restoreState or double clicking on title bar
  if (!floating && allowedAreas() == Qt::NoDockWidgetArea)
    setFloating(true);
}

bool DockWidget::escapeClosesWidget() const
{
  return extraFeatures().testFlag(DockWidgetCloseOnEscapeKey);
}

void DockWidget::setEscapeClosesWidget(bool escapeCloses)
{
  if (escapeCloses)
    extraFeatures_ |= DockWidgetCloseOnEscapeKey;
  else
    extraFeatures_ &= (~DockWidgetCloseOnEscapeKey);
}

QTabBar* DockWidget::findTabWithTitle_(const QList<QTabBar*>& fromBars, const QString& title, int& index) const
{
  Q_FOREACH(QTabBar* tabBar, fromBars)
  {
    // Now search each tab bar for the tab whose title matches ours
    int numTabs = tabBar->count();
    for (index = 0; index < numTabs; index++)
    {
      if (tabBar->tabText(index) == title)
        return tabBar;
    }
  }
  return NULL;
}

QAction* DockWidget::isDockableAction() const
{
  return dockableAction_;
}

bool DockWidget::isMaximized_() const
{
  QDesktopWidget dw;
  return geometry() == dw.availableGeometry(this);
}

bool DockWidget::searchEnabled() const
{
  return searchLineEdit_ != NULL;
}

void DockWidget::setSearchEnabled(bool enable)
{
  if (enable == searchEnabled())
    return;

  // Update the features flag
  if (enable)
    extraFeatures_ |= DockSearchHint;
  else
    extraFeatures_ ^= DockSearchHint;

  // If turning off, destroy the line edit
  if (!enable)
  {
    delete searchLineEdit_;
    searchLineEdit_ = NULL;
    return;
  }

  searchLineEdit_ = new simQt::SearchLineEdit(this);
  searchLineEdit_->setObjectName("DockWidgetSearch");
  searchLineEdit_->setToolTip(tr("Search"));
  // Ensure horizontal policy is preferred
  searchLineEdit_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
  // Without setting a fixed height, the title bar expands a bit.  Choose any tool button for height
  searchLineEdit_->setFixedHeight(restoreButton_->height() + 3); // 3 from experimentation does not cut off descenders
  // Without auto-fill, style sheets for search background color sometimes don't work
  searchLineEdit_->setAutoFillBackground(true);

  // Insert after icon and title, before any action buttons
  titleBarLayout_->insertWidget(SEARCH_LAYOUT_INDEX, searchLineEdit_);
}

simQt::SearchLineEdit* DockWidget::searchLineEdit() const
{
  return searchLineEdit_;
}

int DockWidget::insertTitleBarWidget(int beforeIndex, QWidget* widget)
{
  if (titleBar_ == NULL || titleBar_->layout() == NULL)
    return 1;
  QBoxLayout* layout = dynamic_cast<QBoxLayout*>(titleBar_->layout());
  if (layout == NULL)
    return 1;
  const int numPrev = titleBar_->layout()->count();

  // Calculate the actual index -- offset by icon, title, and maybe search edit if it exists
  const int actualIndex = beforeIndex + (searchLineEdit_ == NULL ? 0 : 1) + SEARCH_LAYOUT_INDEX;
  layout->insertWidget(actualIndex, widget);

  // Add the delta of objects changed in case this results in a "move" (i.e. no items added)
  titleBarWidgetCount_ += titleBar_->layout()->count() - numPrev;
  return 0;
}

int DockWidget::addTitleBarWidget(QWidget* widget)
{
  return insertTitleBarWidget(titleBarWidgetCount(), widget);
}

int DockWidget::titleBarWidgetCount() const
{
  return titleBarWidgetCount_;
}

DockWidget::ExtraFeatures DockWidget::extraFeatures() const
{
  return extraFeatures_;
}

void DockWidget::setExtraFeatures(DockWidget::ExtraFeatures features)
{
  if (extraFeatures_ == features)
    return;

  // DockSearchHint
  const bool showSearch = features.testFlag(DockSearchHint);
  if (extraFeatures_.testFlag(DockSearchHint) != showSearch)
    setSearchEnabled(showSearch);

  // Save extra features now -- code below may depend on it being set.
  const bool wasNoStyleTitle = extraFeatures_.testFlag(DockNoTitleStylingHint);
  extraFeatures_ = features;

  // DockNoTitleStylingHint
  const bool newNoStyleTitle = features.testFlag(DockNoTitleStylingHint);
  if (wasNoStyleTitle != newNoStyleTitle)
  {
    if (newNoStyleTitle)
    {
      // Restore the stylesheet and icons
      titleBar_->setStyleSheet(QString());
      restoreButton_->setIcon(restoreIcon_->originalIcon());
      maximizeButton_->setIcon(maximizeIcon_->originalIcon());
      dockButton_->setIcon(dockIcon_->originalIcon());
      undockButton_->setIcon(undockIcon_->originalIcon());
      closeButton_->setIcon(closeIcon_->originalIcon());
    }
    else
    {
      // Figure out title bar based on focus
      haveFocus_ = isChildWidget_(QApplication::focusWidget());
      updateTitleBarColors_(haveFocus_);
    }
  }

  // Other style hints are handled in the updateTitleBar_() method
  updateTitleBar_();
}

void DockWidget::updateTitleBarColors_(bool haveFocus)
{
  // Do nothing if title styling is off, or if we have the 'no bar' title active
  if (extraFeatures_.testFlag(DockNoTitleStylingHint) || titleBarWidget() == noTitleBar_)
    return;

  // Fix the style sheet
  titleBar_->setStyleSheet(haveFocus ? focusStylesheet_ : inactiveStylesheet_);

  // Set the icon colors for each of the buttons
  const QColor iconColor = (haveFocus ? focusTextColor_ : inactiveTextColor_);
  restoreButton_->setIcon(restoreIcon_->icon(iconColor));
  maximizeButton_->setIcon(maximizeIcon_->icon(iconColor));
  dockButton_->setIcon(dockIcon_->icon(iconColor));
  undockButton_->setIcon(undockIcon_->icon(iconColor));
  closeButton_->setIcon(closeIcon_->icon(iconColor));
}

void DockWidget::changeTitleColorsFromFocusChange_(QWidget* oldFocus, QWidget* newFocus)
{
  // Do nothing if we have no styling
  if (extraFeatures_.testFlag(DockNoTitleStylingHint) || titleBarWidget() == noTitleBar_)
    return;

  // If the newFocus is a child, then we have focus in the dock widget
  const bool haveFocus = isChildWidget_(newFocus);
  // no change means no updates on colors
  if (haveFocus_ == haveFocus)
    return;

  haveFocus_ = haveFocus;
  updateTitleBarColors_(haveFocus_);
}

bool DockWidget::isChildWidget_(const QWidget* widget) const
{
  // Find out whether we're in the parentage for the focused widget
  while (widget != NULL)
  {
    if (widget == this)
      return true;
    widget = widget->parentWidget();
  }
  return false;
}

void DockWidget::keyPressEvent(QKeyEvent *e)
{
  if (escapeClosesWidget())
  {
    // Calls close() if Escape is pressed.
    if (!e->modifiers() && e->key() == Qt::Key_Escape)
      close();
    else
      e->ignore();

    // Qt documentation states that widgets that:
    // "If you reimplement this handler, it is very important that you call the base class implementation if you do not act upon the key"
    // However, qdialog.cpp does not follow this pattern, and that is the class which
    // we are using as a model for this behavior.
  }
  else
  {
    QDockWidget::keyPressEvent(e);
  }
}

void DockWidget::showEvent(QShowEvent* evt)
{
  QDockWidget::showEvent(evt);

  // Queue a raise() to occur AFTER the actual show() finishes, to make window pop up
  QTimer::singleShot(0, this, SLOT(raise()));

  // Do nothing if dock title styling is turned off
  if (extraFeatures_.testFlag(DockNoTitleStylingHint) || titleBarWidget() == noTitleBar_)
    return;
  setFocus();
  activateWindow();  // Covers highlighting when floating
}

void DockWidget::show()
{
  // The following may or may not call showEvent() based on current state
  QDockWidget::show();
  // Only set focus if our title bar widget is used
  if (extraFeatures_.testFlag(DockNoTitleStylingHint) || titleBarWidget() == noTitleBar_)
    return;
  setFocus();
}

void DockWidget::loadSettings_()
{
  // nothing to do if ignoring settings
  if (extraFeatures_.testFlag(DockWidgetIgnoreSettings))
    return;

  // Load any splitters positions or column widths
  if (settings_)
    settings_->loadWidget(widget());

  // Pull out the default geometry
  QVariant widgetGeometry;
  if (settings_)
    widgetGeometry = settings_->value(path_() + DOCK_WIDGET_GEOMETRY, simQt::Settings::MetaData(simQt::Settings::SIZE, QVariant(), "", simQt::Settings::PRIVATE));
  else
  {
    QSettings settings;
    widgetGeometry = settings.value(path_() + DOCK_WIDGET_GEOMETRY);
  }

  // Initialize the bound setting for disable-all-docking
  if (globalSettings_)
  {
    disableAllDocking_ = new simQt::BoundBooleanSetting(this, *globalSettings_, DockWidget::DISABLE_DOCKING_SETTING,
      DockWidget::DISABLE_DOCKING_METADATA);
    connect(disableAllDocking_, SIGNAL(valueChanged(bool)), this, SLOT(setGlobalNotDockableFlag_(bool)));
    setGlobalNotDockableFlag_(disableAllDocking_->value());
  }

  normalGeometry_ = geometry();
  restoreFloating_(widgetGeometry.toByteArray());
}

void DockWidget::restoreFloating_(const QByteArray& geometryBytes)
{
  // Restore the widget from the main window
  if (mainWindow_ == NULL)
  {
    // Must be floatable, because we can't dock without it
    assert(features().testFlag(DockWidgetFloatable));
    if (features().testFlag(DockWidgetFloatable))
    {
      setFloating(true);
      restoreGeometry(geometryBytes);
    }
    return;
  }

  // If ignoring settings, bypass main window. Otherwise give main window first opportunity to restore the position
  if (extraFeatures_.testFlag(DockWidgetIgnoreSettings) || !mainWindow_->restoreDockWidget(this))
  {
    const bool globalNoDocking = (disableAllDocking_ != NULL && disableAllDocking_->value());
    // Restoration failed; new window.  Respect the features() flag to pop up or dock.
    if (features().testFlag(DockWidgetFloatable) || globalNoDocking)
    {
      setFloating(true);
      restoreGeometry(geometryBytes);
    }
    else
    {
      // Need to dock into a place, because floatable is disabled
      mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
    }
  }
  else
  {
#ifndef WIN32
    // On some versions of Gnome, this flag gets set and causes problems where
    // the dock widget, when undocked, will always be in front of other modal
    // always-on-top windows like the file dialog
    setWindowFlags(windowFlags() & ~(Qt::X11BypassWindowManagerHint));
#endif
  }
}

void DockWidget::saveSettings_()
{
  // nothing to do if ignoring settings
  if (extraFeatures_.testFlag(DockWidgetIgnoreSettings))
    return;

  settingsSaved_ = true;

  // Save any splitters positions or column widths
  if (settings_)
  {
    settings_->saveWidget(QDockWidget::widget());
    settings_->setValue(path_() + DOCKABLE_SETTING, dockableAction_->isChecked(), DOCKABLE_METADATA);
    settings_->setValue(path_() + DOCK_WIDGET_GEOMETRY, saveGeometry(), simQt::Settings::MetaData(simQt::Settings::SIZE, QVariant(), "", simQt::Settings::PRIVATE));
  }
  else
  {
    // Save geometry since we can't save the widget (no settings_ pointer)
    QSettings settings;
    settings.setValue(path_() + DOCK_WIDGET_GEOMETRY, saveGeometry());
  }
}

QString DockWidget::path_() const
{
  const QString combined = simQt::WINDOWS_SETTINGS + objectName();
  if (settings_)
    return combined + "/";
  // Handle the "no simQt::Settings" case
  return QString("Private/%1/%2/").arg(windowTitle(), combined);
}

void DockWidget::setGlobalNotDockableFlag_(bool disallowDocking)
{
  if (dockableAction_)
    dockableAction_->setEnabled(!disallowDocking);
  setDockable(isDockable_ && !disallowDocking);
}

}
