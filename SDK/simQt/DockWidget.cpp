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
#include <iostream>
#include <cassert>
#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QScreen>
#include <QStyleHints>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include "simNotify/Notify.h"
#include "simQt/SearchLineEdit.h"
#include "simQt/BoundSettings.h"
#include "simQt/QtFormatting.h"
#include "simQt/QtUtils.h"
#include "simQt/DockWidget.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#include <QDesktopWidget>
#endif

namespace simQt {

/** QSettings key for the dockable persistent setting */
static const QString DOCKABLE_SETTING = "DockWidgetDockable";
/** QSettings key for geometry, to restore geometry before main window manages the dock widget */
static const QString DOCK_WIDGET_GEOMETRY = "DockWidgetGeometry";
/** QSettings key for un-maximized geometry, so the widget can restore to the last known un-maximized state if it is maximized */
static const QString DOCK_WIDGET_UNMAX_GEOMETRY = "DockWidgetUnmaximizedGeometry";
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

/// Setting that can be used to change dock widget border size
const QString DOCK_BORDER_THICKNESS = "Windows/Undocked Border Thickness";
/// Metadata for DOCK_BORDER_THICKNESS
const simQt::Settings::MetaData DOCK_BORDER_METADATA = simQt::Settings::MetaData::makeInteger(
  3, QObject::tr("Set border thickness of dock widgets, in pixels"),
  simQt::Settings::ADVANCED, 1, 10);

/** Index value for the search widget if it exists */
static const int SEARCH_LAYOUT_INDEX = 2;
/** Default docking flags enables all buttons, but not search */
static const DockWidget::ExtraFeatures DEFAULT_EXTRA_FEATURES(
  DockWidget::DockMaximizeAndRestoreHint |
  DockWidget::DockUndockAndRedockHint |
  DockWidget::DockWidgetCloseOnEscapeKey
);

/** Amount of rounding around the edges for Dock Widget; 8 and 11 work well on Win11 */
static constexpr int ROUND_RADIUS_PX = 8;

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
  explicit DoubleClickFrame(DockWidget& dockWidget, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags())
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
  DoubleClickIcon(DockWidget& dockWidget, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags())
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

class DockWidget::TabDragDropEventFilter : public QObject
{
public:
  TabDragDropEventFilter(DockWidget& dockWidget)
    : QObject(),
      dockWidget_(dockWidget),
      tabBar_(nullptr)
  {
  }

  void setTabBar(QTabBar* tabBar)
  {
    tabBar_ = tabBar;
    if (tabBar_)
      tabBar_->installEventFilter(this);
  }

  bool eventFilter(QObject* object, QEvent* event)
  {
    if (object != tabBar_ || !tabBar_)
      return false;

    if (event->type() == QEvent::DragEnter)
    {
      QDragEnterEvent* dragEvt = dynamic_cast<QDragEnterEvent*>(event);
      if (!dragEvt)
        return false;

      // Don't interfere with moving the dock widget between tabs
      if (dragEvt->dropAction() == Qt::MoveAction)
        return false;

      bool evtConsumed = false;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      const int tabIndex = tabBar_->tabAt(dragEvt->pos());
#else
      const int tabIndex = tabBar_->tabAt(dragEvt->position().toPoint());
#endif
      QString tabName = tabBar_->tabText(tabIndex);

      if (tabIndex < 0)
      {
        // Drags over empty portions of the tab bar should be ignored
        dragEvt->setDropAction(Qt::IgnoreAction);
      }
      else if (tabName == dockWidget_.windowTitle())
      {
        // Set the event ignored, but only to determine if the dock widget can accept it. The event is accepted later anyway
        dragEvt->ignore();
        dockWidget_.dragEnterEvent(dragEvt);
        if (dragEvt->isAccepted())
        {
          evtConsumed = true;
        }
        else
        {
          // Drop action needs to be set to ignore so that the drag image correctly shows a block icon to show it can't currently be dropped
          dragEvt->setDropAction(Qt::IgnoreAction);
        }
      }

      // Drag enter event needs to be accepted in order to receive drag move events later
      dragEvt->accept();

      prevTab_ = tabName;
      return evtConsumed;
    }

    else if (event->type() == QEvent::DragMove)
    {
      // Most of the time only drag enter matters. Here, because the events are on a tab bar,
      // we need to listen to moves in case the drag moves from one tab to another
      QDragMoveEvent* dragEvt = dynamic_cast<QDragMoveEvent*>(event);
      if (!dragEvt)
        return false;

      // Don't interfere with moving the dock widget between tabs
      if (dragEvt->dropAction() == Qt::MoveAction)
        return false;

      QString thisTitle = dockWidget_.windowTitle();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      const int mouseIndex = tabBar_->tabAt(dragEvt->pos());
#else
      const int mouseIndex = tabBar_->tabAt(dragEvt->position().toPoint());
#endif
      QString mouseTitle = tabBar_->tabText(mouseIndex);

      // Nothing to do if the mouse isn't over this widget's tab or the move remained over the same tab
      if (prevTab_ == thisTitle || mouseTitle != thisTitle)
      {
        prevTab_ = mouseTitle;
        if (mouseIndex < 0)
          dragEvt->setDropAction(Qt::IgnoreAction);
        return false;
      }

      // Construct a drag enter event from the drag move and pass it to our dock widget
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      QDragEnterEvent simulatedEvent(dragEvt->pos(), dragEvt->possibleActions(), dragEvt->mimeData(), dragEvt->mouseButtons(), dragEvt->keyboardModifiers());
#else
      QDragEnterEvent simulatedEvent(dragEvt->position().toPoint(), dragEvt->possibleActions(), dragEvt->mimeData(), dragEvt->buttons(), dragEvt->modifiers());
#endif
      simulatedEvent.ignore();
      dockWidget_.dragEnterEvent(&simulatedEvent);

      prevTab_ = mouseTitle;
      dragEvt->setAccepted(simulatedEvent.isAccepted());
      return simulatedEvent.isAccepted();
    }
    else if (event->type() == QEvent::DragLeave)
    {
      prevTab_ = "";
    }
    else if (event->type() == QEvent::Drop)
    {
      QDropEvent* dropEvt = dynamic_cast<QDropEvent*>(event);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      if (!dropEvt || tabBar_->tabText(tabBar_->tabAt(dropEvt->pos())) != dockWidget_.windowTitle())
#else
      if (!dropEvt || tabBar_->tabText(tabBar_->tabAt(dropEvt->position().toPoint())) != dockWidget_.windowTitle())
#endif
        return false;

      // Don't interfere with moving the dock widget between tabs
      if (dropEvt->dropAction() == Qt::MoveAction)
        return false;

      dockWidget_.dropEvent(dropEvt);
      prevTab_ = "";
      return dropEvt->isAccepted();
    }

    return false;
  }

  void uninstall(QMainWindow* mainWindow)
  {
    // remove event filter from previous tab bar, if it still exists
    if (mainWindow && tabBar_)
    {
      QList<QTabBar*> tabBars = mainWindow->findChildren<QTabBar*>();
      for (auto tabBar : tabBars)
      {
        if (tabBar == tabBar_)
        {
          tabBar_->removeEventFilter(this);
          break;
        }
      }
    }
    tabBar_ = nullptr;
  }

private:
  DockWidget& dockWidget_;
  QPointer<QTabBar> tabBar_;
  QString prevTab_;
};

///////////////////////////////////////////////////////////////

// Adapted from https://doc.qt.io/qt-5/qtwidgets-widgets-elidedlabel-example.html and
// https://stackoverflow.com/questions/73684307
class DockWidget::ElidedTitleLabel : public QFrame
{
public:
  explicit ElidedTitleLabel(QWidget* parent = nullptr)
    : QFrame(parent)
  {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumSize(40, 6);
  }

  void setText(const QString& txt)
  {
    content_ = txt;
    update();
  }

  void setElideMode(Qt::TextElideMode elideMode)
  {
    elideMode_ = elideMode;
    update();
  }

  const QString& text() const
  {
    return content_;
  }

  QSize sizeHint() const override
  {
    const auto& margins = contentsMargins();
    const QSize marginSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    const auto& metrics = fontMetrics();
    return marginSize + QSize{ metrics.averageCharWidth() * 4, metrics.height() };
  }

protected:
  virtual void paintEvent(QPaintEvent* evt) override
  {
    QFrame::paintEvent(evt);

    QPainter painter(this);
    const QFontMetrics& fontMetrics = painter.fontMetrics();

    const auto& margins = contentsMargins();
    const QString& elidedLine = fontMetrics.elidedText(content_, elideMode_, width() - margins.left() - margins.right());
    painter.drawText(QPoint(margins.left(), fontMetrics.ascent() + margins.bottom()), elidedLine);
  }

private:
  QString content_;
  Qt::TextElideMode elideMode_ = Qt::ElideRight;
};

///////////////////////////////////////////////////////////////

inline
bool pointOnScreen(const QPoint& point)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  const auto& screens = QGuiApplication::screens();
  for (const auto& screen : screens)
  {
    if (screen->availableGeometry().contains(point))
      return true;
  }
  return false;
#else
  return QGuiApplication::screenAt(point);
#endif
}

inline
void ensureVisible(simQt::DockWidget& dockWidget, const QWidget* parentWidget)
{
  // Docked widgets will always be visible
  if (!dockWidget.isFloating())
    return;

  // Dock widgets should always have a title; the no-title display is 1x1
  auto* title = dockWidget.titleBarWidget();
  if (!title)
    return; // unexpected

  const auto& titlePos = title->mapToGlobal(title->pos());
  const QRect titleRect(titlePos, title->size());
  // Each corner of the title should be on the screen
  if (pointOnScreen(titleRect.topLeft()) && pointOnScreen(titleRect.topRight()) &&
    pointOnScreen(titleRect.bottomLeft()) && pointOnScreen(titleRect.bottomRight()))
    return;

  QtUtils::centerWidgetOnParent(dockWidget, parentWidget);
}

///////////////////////////////////////////////////////////////

DockWidget::DockWidget(QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(parent, flags),
    globalSettings_(nullptr),
    mainWindow_(dynamic_cast<QMainWindow*>(parent))
{
  init_();
}

DockWidget::DockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags)
  : QDockWidget(title, parent, flags),
    globalSettings_(nullptr),
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

  uninstallTabEventFilter_();
  delete tabDragFilter_;
  tabDragFilter_ = nullptr;

  delete noTitleBar_;
  noTitleBar_ = nullptr;
  delete titleBar_;
  titleBar_ = nullptr;
}

void DockWidget::init_()
{
  // QTBUG-140207: Appears to be mitigated by turning off animations on the main window. This is
  // a central location in which to do that. Likely also related to QTBUG-141718, QTBUG-141350.
#if QT_VERSION > QT_VERSION_CHECK(6,8,4) && QT_VERSION < QT_VERSION_CHECK(6,10,1)
  if (mainWindow_)
    mainWindow_->setAnimated(false);
#endif

  // SIM-17647: the event filter cannot be a child of the tab bar, must persist for the life of the widget
  // This is because after unloading plug-ins, the QTabBar might still reference the event filter after it has been destroyed
  tabDragFilter_ = new TabDragDropEventFilter(*this);

  // default title bar text size to application text size
  titleBarPointSize_ = QApplication::font().pointSize();
  searchLineEdit_ = nullptr;
  titleBarWidgetCount_ = 0;
  extraFeatures_ = DEFAULT_EXTRA_FEATURES;
  settingsSaved_ = (settings_ == nullptr);  // Prevent false asserts when the simQt::Settings is not provided in construction
  haveFocus_ = false;
  isDockable_ = true;
  disableAllDocking_ = nullptr;
  borderThickness_ = nullptr;

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
  // Widget needs a layout, else QWidget::sizeHint() returns (-1,-1), which adversely
  // affects the size of the OpenGL widget in SIMDIS in fullscreen mode.
  QHBoxLayout* noLayout = new QHBoxLayout;
  noLayout->setContentsMargins(0, 0, 0, 0);
  noTitleBar_->setLayout(noLayout);
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
  connect(this, SIGNAL(windowTitleChanged(QString)), this, SLOT(updateTitleBarText_()));
  connect(this, SIGNAL(windowIconChanged(QIcon)), this, SLOT(updateTitleBarIcon_()));

  // Set a consistent focus
  updateTitleBarColors_(false);
}

void DockWidget::createStylesheets_()
{
  const QString ssTemplate =
    "#titleBar {"
    "   background: %1;"
    "   border: 1px solid %3;"
    "   border-top-left-radius: %4px;"    // Top-left corner
    "   border-top-right-radius: %4px;"   // Top-right corner
    "   border-bottom-left-radius: 0px;" // Bottom-left corner (square)
    "   border-bottom-right-radius: 0px;"// Bottom-right corner (square)
    "} "
    "#titleBarTitle {color: %2;} "
    ;

  const bool lightMode = !simQt::QtUtils::isDarkTheme();

  const QColor inactiveBackground = lightMode ? QColor("#e0e0e0") : QColor("#3C3C3C"); // Light gray vs dark gray
  inactiveTextColor_ = lightMode ? QColor("#404040") : Qt::white; // Darker gray or white
  const QColor darkerInactiveBg = QColor("#d0d0d0");

  // Get the focus colors
  const QColor focusBackground = lightMode ? QColor("#d8d8d8") : QColor("#0078D7"); // Lighter gray or blue
  focusTextColor_ = lightMode ? QColor("#202020") : Qt::white; // Darkest gray or white
  const QColor darkerFocusBg = QColor("#b0b0b0");

  // Create the inactive stylesheet
  inactiveStylesheet_ = ssTemplate
    .arg(inactiveBackground.name())
    .arg(inactiveTextColor_.name())
    .arg(darkerInactiveBg.name())
    .arg(ROUND_RADIUS_PX);

  // Create the focused stylesheet
  focusStylesheet_ = ssTemplate
    .arg(focusBackground.name())
    .arg(focusTextColor_.name())
    .arg(darkerFocusBg.name())
    .arg(ROUND_RADIUS_PX);
}

QWidget* DockWidget::createTitleBar_()
{
  // Create the title bar and set its shape and style information
  QFrame* titleBar = new DoubleClickFrame(*this);
  titleBar->setObjectName("titleBar");

  // Create the icon holders
  titleBarIcon_ = new DoubleClickIcon(*this);
  titleBarIcon_->setObjectName("titleBarIcon");
  titleBarIcon_->setPixmap(windowIcon().pixmap(QSize(16, 16)));
  titleBarIcon_->setScaledContents(true);
  titleBarIcon_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  // Set the title bar's caption
  titleBarTitle_ = new ElidedTitleLabel();
  titleBarTitle_->setObjectName("titleBarTitle");
  titleBarTitle_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  // Note a padding of 0 pixels looks bad, especially on Ubuntu 14
  titleBarTitle_->setContentsMargins(4, 0, 0, 0);

  updateTitleBarText_(); // Calls titleBarTitle_->setText() in a consistent manner

  // Create tool buttons for each button that might show on the GUI
  restoreButton_ = newToolButton_(restoreAction_);
  maximizeButton_ = newToolButton_(maximizeAction_);
  dockButton_ = newToolButton_(dockAction_);
  undockButton_ = newToolButton_(undockAction_);
  closeButton_ = newToolButton_(closeAction_);

  // Style the tool buttons
  const QString buttonStyle =
    "QToolButton {"
    "   background-color: transparent;"
    "   border: none;"
    "   padding: 2px;"
    "}"
    "QToolButton:hover {"
    "   background-color: rgba(0, 0, 0, 0.1);"
    "}";
  restoreButton_->setStyleSheet(buttonStyle);
  maximizeButton_->setStyleSheet(buttonStyle);
  dockButton_->setStyleSheet(buttonStyle);
  undockButton_->setStyleSheet(buttonStyle);
  closeButton_->setStyleSheet(buttonStyle);

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

  // Update the window mask for rounded edges
  if (floating)
  {
    const QRect rect(0, 0, width(), height());
    QPainterPath path;
    path.addRoundedRect(rect, ROUND_RADIUS_PX, ROUND_RADIUS_PX);
    QBitmap mask(rect.size());
    mask.clear();
    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::color1); // Required to fill the path
    painter.setPen(Qt::NoPen);    // Ensure no outline is drawn
    painter.drawPath(path);
    painter.end();
    setMask(mask);
  }
  else
  {
    // Do not use rounded edges
    setMask(QRegion());
  }

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
  updateTitleBarIcon_();
  updateTitleBarText_(); // Calls titleBarTitle_->setText() in a consistent manner

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
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  QDesktopWidget dw;
  setGeometry(dw.availableGeometry(this));
#else
  auto* currentScreen = screen();
  if (currentScreen)
    setGeometry(currentScreen->availableGeometry());
#endif

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
  if (isFloating() && mainWindow_ != nullptr)
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);

  // Finally update the state of the enable/disable/visibility
  updateTitleBar_();
}

void DockWidget::undock()
{
  undock_();
}

void DockWidget::undock_()
{
  if (isFloating() || !features().testFlag(DockWidgetFloatable))
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
  // always uninstall tab event filter in case widget went from tabbed to floating
  uninstallTabEventFilter_();
  // Break out early if we're floating, or if there's no main window
  if (isFloating() || !mainWindow_)
    return;

  // Return early if this dock widget is not tabified
  QList<QDockWidget*> tabifiedWidgets = mainWindow_->tabifiedDockWidgets(this);
  if (tabifiedWidgets.empty())
    return;

  // Tabified, now set icon to tab
  // First, find all the tab bars, since QMainWindow doesn't provide
  // direct access to the DockArea QTabBar, making sure to only get direct children of the main window
  QList<QTabBar*> tabBars = mainWindow_->findChildren<QTabBar*>(QString(), Qt::FindDirectChildrenOnly);

  // Locate the tab bar that contains this window, based on the window title
  int index = 0;
  QTabBar* tabBar = findTabWithTitle_(tabBars, windowTitle(), index);
  if (tabBar == nullptr)
    return;

  // This title matches ours, set the tab icon
  tabBar->setTabIcon(index, widget()->windowIcon());

  tabBar->setAcceptDrops(true);
  installTabEventFilter_(tabBar);

  // Here is a special case, the initial tabification, we are making the other widget become tabified as well
  // need to set their tab icon, since there is no other way to alert them they are becoming tabified
  if (tabifiedWidgets.size() == 1)
  {
    // index for other tab is 0 or 1, whichever is not ours
    int newIndex = (index == 1 ? 0 : 1);
    // Set icon from our only other tabified widget
    DockWidget* firstTab = dynamic_cast<DockWidget*>(tabifiedWidgets[0]);
    if (firstTab != nullptr && firstTab->windowTitle() == tabBar->tabText(newIndex))
    {
      firstTab->installTabEventFilter_(tabBar);
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
  const QString& filePath = windowFilePath();
  if (filePath.isEmpty())
  {
    titleBarTitle_->setText(windowTitle());
    return;
  }

  // Form a string that includes the file path
  const QFileInfo fi(filePath);
  titleBarTitle_->setText(tr("%1   [%2]  %3")
    .arg(windowTitle())
    .arg(fi.fileName())
    .arg(QDir::toNativeSeparators(fi.absolutePath()))
  );
}

void DockWidget::updateTitleBarIcon_()
{
  // make the window icon twice as large as the text point size
  int newPointSize = titleBarPointSize_ * 2;
  titleBarIcon_->setPixmap(windowIcon().pixmap(QSize(newPointSize, newPointSize)));
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
  Q_EMIT(closedGui());
}

void DockWidget::setDefaultSize(const QSize& defaultSize)
{
  defaultSize_ = defaultSize;
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
  if (widget == nullptr)
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
  return !allDockingDisabled() && isDockable_;
}

void DockWidget::setDockable(bool dockable)
{
  // Note: Intentionally not doing early-out here because value may match but
  // we may need to do work, since changing disable-all-docking can eventually
  // call this method.

  // Override the dockability flag with the global if needed
  const bool globalDockDisable = allDockingDisabled();
  if (globalDockDisable)
    dockable = false;

  // Update settings and QMenu's QAction
  bool emitIt = (dockable != isDockable_);
  // Do not override isDockable_ if globalDockDisable is active
  if (!globalDockDisable)
    isDockable_ = dockable;
  else
    emitIt = false;

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
    Q_EMIT isDockableChanged(isDockable_);
}

bool DockWidget::allDockingDisabled() const
{
  return (disableAllDocking_ != nullptr && disableAllDocking_->value());
}

void DockWidget::restoreDefaultLayout()
{
  // remove geometry from saved settings
  if (settings_)
  {
    settings_->setValue(path_() + DOCK_WIDGET_GEOMETRY, QVariant());
    settings_->setValue(path_() + DOCK_WIDGET_UNMAX_GEOMETRY, QVariant());
  }
  else
  {
    QSettings settings;
    settings.setValue(path_() + DOCK_WIDGET_GEOMETRY, QVariant());
    settings.setValue(path_() + DOCK_WIDGET_UNMAX_GEOMETRY, QVariant());
  }
  // remove main window temporarily to restore a default state in loadSettings_()
  auto mainWindow = mainWindow_;
  mainWindow_ = nullptr;
  loadSettings_();
  mainWindow_ = mainWindow;
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
  for (auto it = fromBars.begin(); it != fromBars.end(); ++it)
  {
    QTabBar* tabBar = *it;
    // Now search each tab bar for the tab whose title matches ours
    int numTabs = tabBar->count();
    for (index = 0; index < numTabs; index++)
    {
      if (tabBar->tabText(index) == title)
        return tabBar;
    }
  }
  return nullptr;
}

QAction* DockWidget::isDockableAction() const
{
  return dockableAction_;
}

bool DockWidget::isMaximized_() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  QDesktopWidget dw;
  return geometry() == dw.availableGeometry(this);
#else
  auto* currentScreen = screen();
  return currentScreen && geometry() == currentScreen->availableGeometry();
#endif
}

bool DockWidget::searchEnabled() const
{
  return searchLineEdit_ != nullptr;
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
    searchLineEdit_ = nullptr;
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
  if (titleBar_ == nullptr || titleBar_->layout() == nullptr)
    return 1;
  QBoxLayout* layout = dynamic_cast<QBoxLayout*>(titleBar_->layout());
  if (layout == nullptr)
    return 1;
  const int numPrev = titleBar_->layout()->count();

  // Calculate the actual index -- offset by icon, title, and maybe search edit if it exists
  const int actualIndex = beforeIndex + (searchLineEdit_ == nullptr ? 0 : 1) + SEARCH_LAYOUT_INDEX;
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

void DockWidget::setTitleBarTextSize(int pointSize)
{
  if (titleBarPointSize_ == pointSize)
    return;
  titleBarPointSize_ = pointSize;
  QFont titleFont = titleBarTitle_->font();
  titleFont.setPointSize(titleBarPointSize_);
  titleBarTitle_->setFont(titleFont);
  updateTitleBarIcon_();
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
  while (widget != nullptr)
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

  // Schedule a fix to the tabs, if it is tabified
  if (!isFloating())
    QTimer::singleShot(0, this, SLOT(fixTabIcon_()));

  // Do nothing if dock title styling is turned off
  if (extraFeatures_.testFlag(DockNoTitleStylingHint) || titleBarWidget() == noTitleBar_)
    return;
  setFocus();
  activateWindow();  // Covers highlighting when floating

  // Make sure the dock widget is visible. Recenter it if needed
  QWidget* parentWidget = dynamic_cast<QWidget*>(parent());
  if (!parentWidget)
    parentWidget = mainWindow_;
  ensureVisible(*this, parentWidget);
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

void DockWidget::setWindowFilePath(const QString& path)
{
  QDockWidget::setWindowFilePath(path);
  updateTitleBarText_();
}

void DockWidget::setGlobalSettings(Settings* globalSettings)
{
  if (globalSettings_ == globalSettings)
    return;
  globalSettings_ = globalSettings;
  applyGlobalSettings_();
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
  QVariant normalGeometry;
  if (settings_)
  {
    widgetGeometry = settings_->value(path_() + DOCK_WIDGET_GEOMETRY, simQt::Settings::MetaData(simQt::Settings::SIZE, QVariant(), "", simQt::Settings::PRIVATE));
    normalGeometry = settings_->value(path_() + DOCK_WIDGET_UNMAX_GEOMETRY, simQt::Settings::MetaData(simQt::Settings::SIZE, QVariant(), "", simQt::Settings::PRIVATE));
  }
  else
  {
    QSettings settings;
    widgetGeometry = settings.value(path_() + DOCK_WIDGET_GEOMETRY);
    normalGeometry = settings.value(path_() + DOCK_WIDGET_UNMAX_GEOMETRY);
  }

  // Initialize the bound setting for disable-all-docking
  applyGlobalSettings_();

  // initialize normal geometry to the setting if it's valid
  if (normalGeometry.isValid())
    normalGeometry_ = normalGeometry.toRect();

  // if the normal geometry isn't valid, just use current geometry
  if (!normalGeometry_.isValid())
    normalGeometry_ = geometry();

  restoreFloating_(widgetGeometry.toByteArray());
}

void DockWidget::restoreFloating_(const QByteArray& geometryBytes)
{
  // Restore the widget from the main window
  if (mainWindow_ == nullptr)
  {
    // Must be floatable, because we can't dock without it
    assert(features().testFlag(DockWidgetFloatable));
    if (features().testFlag(DockWidgetFloatable))
      setFloatingGeometry_(geometryBytes);
    return;
  }

  // If ignoring settings, bypass main window. Otherwise give main window first opportunity to restore the position
  if (extraFeatures_.testFlag(DockWidgetIgnoreSettings) || !mainWindow_->restoreDockWidget(this))
  {
    const bool globalNoDocking = (disableAllDocking_ != nullptr && disableAllDocking_->value());
    // Restoration failed; new window.  Respect the features() flag to pop up or dock.
    if (features().testFlag(DockWidgetFloatable) || globalNoDocking)
      setFloatingGeometry_(geometryBytes);
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
    settings_->setValue(path_() + DOCK_WIDGET_UNMAX_GEOMETRY, normalGeometry_, simQt::Settings::MetaData(simQt::Settings::SIZE, QVariant(), "", simQt::Settings::PRIVATE));
  }
  else
  {
    // Save geometry since we can't save the widget (no settings_ pointer)
    QSettings settings;
    settings.setValue(path_() + DOCK_WIDGET_GEOMETRY, saveGeometry());
    settings.setValue(path_() + DOCK_WIDGET_UNMAX_GEOMETRY, normalGeometry_);
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

void DockWidget::setFloatingGeometry_(const QByteArray& geometryBytes)
{
  // geometry is empty, add to main window momentarily so main window state can track geometry
  if (geometryBytes.isEmpty() && mainWindow_)
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
  setFloating(true);
  if (!restoreGeometry(geometryBytes))
  {
    // if restoreGeometry failed, use the default size if it is valid
    if (!defaultSize_.isEmpty())
      resize(defaultSize_);
    else // otherwise, resize to the sizeHint, in case the initial resize hasn't happened yet
      resize(sizeHint());

    // Qt on Linux RHEL8+ (esp Wayland) with multi-screen has problems with positioning widgets such
    // that the dock widget defaults to (0,0) global instead of near the parent window. This attempts to
    // fix the position so that it stays on the same screen as the main window in these cases. Attempt to
    // fix SIM-16068 and SIMDIS-3901. This happens on Qt 5.9 and 5.15 both.
    QWidget* parentWidget = dynamic_cast<QWidget*>(parent());
    if (!parentWidget)
      parentWidget = mainWindow_;
    QtUtils::centerWidgetOnParent(*this, parentWidget);
  }
}

void DockWidget::setGlobalNotDockableFlag_(bool disallowDocking)
{
  if (dockableAction_)
    dockableAction_->setEnabled(!disallowDocking);
  // Call setDockable() with the current dockable state. setDockable()
  // will check disableAllDocking_'s value and dock or undock appropriately
  setDockable(isDockable_);
}

void DockWidget::setBorderThickness_(int thickness)
{
  setStyleSheet(QString("QDockWidget { border: %1px solid #d0d0d0; }").arg(thickness));
}

void DockWidget::applyGlobalSettings_()
{
  if (!globalSettings_)
    return;
  delete disableAllDocking_;
  disableAllDocking_ = new simQt::BoundBooleanSetting(this, *globalSettings_, DockWidget::DISABLE_DOCKING_SETTING,
    DockWidget::DISABLE_DOCKING_METADATA);
  connect(disableAllDocking_, SIGNAL(valueChanged(bool)), this, SLOT(setGlobalNotDockableFlag_(bool)));
  setGlobalNotDockableFlag_(disableAllDocking_->value());

  delete borderThickness_;
  borderThickness_ = new simQt::BoundIntegerSetting(this, *globalSettings_, DOCK_BORDER_THICKNESS, DOCK_BORDER_METADATA);
  connect(borderThickness_, SIGNAL(valueChanged(int)), this, SLOT(setBorderThickness_(int)));
  setBorderThickness_(borderThickness_->value());
}

void DockWidget::installTabEventFilter_(QTabBar* tabBar)
{
  // Only register with 1 tab bar, may still be registered with old tab bars if the dock widget
  // is being moved between tabs. Uninstall old filters first if necessary
  uninstallTabEventFilter_();

  if (tabDragFilter_)
    tabDragFilter_->setTabBar(tabBar);
}

void DockWidget::uninstallTabEventFilter_()
{
  if (tabDragFilter_)
    tabDragFilter_->uninstall(mainWindow_);
}

}
