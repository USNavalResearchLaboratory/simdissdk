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
#include <QAction>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include "simCore/System/Utils.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simUtil/ExampleResources.h"
#include "simQt/ViewerWidgetAdapter.h"

static const QString DEFAULT_DROP_BOX_STYLE = "border: 2px dashed gray; padding: 20px";

/**
 * Main window showing a Widget-based and Window-based viewer widget adapter, side by side.
 * It has hot key actions 1, 2, 3, and 4, that do simple print statements on a label. Two
 * of these actions use WidgetWithChildrenShortcut. The window also accept drag-and-drop.
 *
 * The goal of the example is to show that both types of actions work with both types of
 * viewer widget adapters, and that drag and drop works as well with both types of adapters.
 */
class MainWindow : public QMainWindow
{
public:
  MainWindow(simVis::View* view1, simVis::View* view2, QWidget* parent = nullptr)
    : QMainWindow(parent)
  {
    // UI Setup
    setWindowTitle("Qt Drag and Drop Example");
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Placeholder widgets side-by-side
    simQt::ViewerWidgetAdapter* viewerWidget = new simQt::ViewerWidgetAdapter(simQt::GlImplementation::Widget, this);
    viewerWidget->setViewer(view1->getViewerBase());
    simQt::ViewerWidgetAdapter* viewerWindow = new simQt::ViewerWidgetAdapter(simQt::GlImplementation::Window, this);
    viewerWindow->setViewer(view2->getViewerBase());

    // Labels for the placeholder widgets
    QLabel* widgetViewerLabel = new QLabel("Widget-based Viewer Adapter", this);
    QLabel* windowViewerLabel = new QLabel("Window-based Viewer Adapter", this);

    QHBoxLayout* horizViewersLayout = new QHBoxLayout();
    QVBoxLayout* vertWidgetLayout = new QVBoxLayout();
    QVBoxLayout* vertWindowLayout = new QVBoxLayout();

    vertWidgetLayout->addWidget(widgetViewerLabel);
    vertWidgetLayout->addWidget(viewerWidget);

    vertWindowLayout->addWidget(windowViewerLabel);
    vertWindowLayout->addWidget(viewerWindow);

    horizViewersLayout->addLayout(vertWidgetLayout);
    horizViewersLayout->addLayout(vertWindowLayout);

    mainLayout->addLayout(horizViewersLayout);

    label_ = new QLabel("Drop files here", this);
    defaultTextColor_ = label_->palette().color(QPalette::WindowText);
    label_->setAlignment(Qt::AlignCenter);
    label_->setStyleSheet(DEFAULT_DROP_BOX_STYLE);
    mainLayout->addWidget(label_);

    setCentralWidget(centralWidget);
    setAcceptDrops(true);  // Enable drag and drop for the main window

    // Menu and Actions
    QMenu* fileMenu = menuBar()->addMenu("&File");

    // Create 4 dummy actions that set the label text.
    for (int k = 0; k < 4; ++k)
    {
      QAction* action = new QAction(tr("Action %1").arg(k + 1), this);
      action->setShortcut(QKeySequence(QString::number(k + 1)));
      // Set shortcut context for actions 3 and 4
      if (k >= 2)
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);

      addAction(action);
      fileMenu->addAction(action);
      // Print a simple message
      connect(action, &QAction::triggered, this, [this, k]() {
        setLabelText_(tr("Triggered action #%1").arg(k + 1));
        });
    }

    fileMenu->addSeparator();

    QAction* exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    addAction(exitAction);
    fileMenu->addAction(exitAction);
  }

protected:
  virtual void dragEnterEvent(QDragEnterEvent* event) override
  {
    if (event->mimeData()->hasUrls())
    {
      event->acceptProposedAction();
      label_->setStyleSheet("border: 2px solid green; padding: 20px;"); // Visual feedback
      setLabelText_("Dragging over...");
    }
    else
    {
      event->ignore(); // Ignore the drag if it doesn't contain URLs
    }
  }

  virtual void dragLeaveEvent(QDragLeaveEvent* event) override
  {
    label_->setStyleSheet(DEFAULT_DROP_BOX_STYLE); // Reset style
    setLabelText_("Drop files here");
  }

  virtual void dropEvent(QDropEvent* event) override
  {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
      QStringList pathList;
      QList<QUrl> urlList = mimeData->urls();

      for (const QUrl& url : urlList)
        pathList.append(url.toLocalFile()); // Get the local file path

      if (!pathList.isEmpty())
      {
        QString fileListText = "Dropped files:\n";
        for (const QString& filePath : pathList)
          fileListText += filePath + "\n";
        setLabelText_(fileListText);
      }
      else
        setLabelText_("No files dropped.");

      event->acceptProposedAction(); // Accept the drop
    }
    else
    {
      event->ignore(); // Ignore the drop if it doesn't contain URLs
      setLabelText_("Invalid drop!");
    }

    label_->setStyleSheet(DEFAULT_DROP_BOX_STYLE); // Reset style
  }

private:
  /** Helper to set the label text and also do a quick flash. */
  void setLabelText_(const QString& text) const
  {
    label_->setText(text);
    flashLabel_(label_, Qt::red, 100);
  }

  /** Helper to flash a label a given color for a given time period */
  void flashLabel_(QLabel* label, const QColor& flashColor, int durationMs) const
  {
    if (!label)
      return;

    QTimer* flashTimer = new QTimer(label); // Parent to the label, so it's deleted when the label is deleted
    flashTimer->setSingleShot(true); // Only run once

    QObject::connect(flashTimer, &QTimer::timeout, [=, this]() {
      // Restore the original color after the duration
      QPalette p = label->palette();
      p.setColor(QPalette::WindowText, defaultTextColor_); // Restore original color
      label->setPalette(p);
      flashTimer->deleteLater(); // Clean up the timer.
      });

    // Set the flash color
    QPalette p = label->palette();
    p.setColor(QPalette::WindowText, flashColor); // Set the flash color
    label->setPalette(p);

    flashTimer->start(durationMs); // Start the timer
  }

  QLabel* label_ = nullptr;
  QColor defaultTextColor_ = Qt::black;
};

int main(int argc, char* argv[])
{
  simCore::initializeSimdisEnvironmentVariables();

  simExamples::configureSearchPaths();

  // a Map and a Scene Manager:
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(simExamples::createDefaultExampleMap());
  simExamples::addDefaultSkyNode(sceneMan.get());

  // Views to embed in our widgets
  osg::ref_ptr<simVis::View> view1 = new simVis::View();
  view1->setSceneManager(sceneMan.get());
  view1->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  view1->installBasicDebugHandlers();
  osg::ref_ptr<simVis::View> view2 = new simVis::View();
  view2->setSceneManager(sceneMan.get());
  view2->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  view2->installBasicDebugHandlers();

  // The ViewManager coordinates the rendering of all our views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager();
  viewMan->setUseMultipleViewers(true);
  viewMan->addView(view1.get());
  viewMan->addView(view2.get());

  QApplication app(argc, argv);

  MainWindow mainWindow(view1.get(), view2.get());
  mainWindow.resize(1200, 600);
  mainWindow.show();

  return app.exec();
}
