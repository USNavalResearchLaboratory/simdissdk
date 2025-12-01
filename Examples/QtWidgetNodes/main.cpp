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
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStyleHints>
#include <QTimer>
#include <QWidget>
#include "simCore/System/Utils.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simUtil/ExampleResources.h"
#include "simQt/QWidgetNode.h"
#include "simQt/ViewerWidgetAdapter.h"

namespace
{

/** Generates a test image */
QImage generateTestImage(int width, int height)
{
  QImage image(width, height, QImage::Format_ARGB32);
  image.fill(Qt::black);

  // Draw a red rectangle in the top-left corner
  for (int x = 0; x < width / 4; ++x)
  {
    for (int y = 0; y < height / 4; ++y)
    {
      image.setPixelColor(x, y, Qt::red);
    }
  }

  // Draw a green rectangle in the bottom-right corner
  for (int x = width * 3 / 4; x < width; ++x)
  {
    for (int y = height * 3 / 4; y < height; ++y)
    {
      image.setPixelColor(x, y, Qt::green);
    }
  }

  // Draw a blue diagonal line
  for (int i = 0; i < qMin(width, height); ++i)
  {
    image.setPixelColor(i, i, Qt::blue);
  }

  // Write "TEST" in white in the center
  QFont font("Arial", qMin(width, height) / 10); // Adjust font size
  QPainter painter(&image);
  painter.setFont(font);
  painter.setPen(Qt::white);
  painter.drawText(image.rect(), Qt::AlignCenter, "TEST");

  return image;
}

/** Creates an osg::MatrixTransform for placing HUD elements */
osg::ref_ptr<osg::MatrixTransform> createHudTransform(float x, float y, float scale)
{
  osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
  osg::Matrix m;
  m.makeScale(scale, scale, 1.0f);
  m.postMultTranslate(osg::Vec3(x, y, 0.0f));
  xform->setMatrix(m);
  return xform;
}

}

/**
 * Main window that shows a viewer widget and several overlays on the HUD
 * that are generated from QImage, QLabel, and other QWidget instances.
 */
class MainWindow : public QMainWindow
{
public:
  explicit MainWindow(simVis::View* mainView, QWidget* parent = nullptr)
    : QMainWindow(parent),
    mainView_(mainView)
  {
    // UI Setup
    setWindowTitle("Qt Widget Nodes Example");
    simQt::ViewerWidgetAdapter* viewerWidget = new simQt::ViewerWidgetAdapter(simQt::GlImplementation::Widget, this);
    viewerWidget->setViewer(mainView->getViewerBase());
    setCentralWidget(viewerWidget);

    // Create HUD camera
    hudCamera_ = mainView->getOrCreateHUD();

    // Menu and Actions
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    fileMenu->addAction(exitAction);

    QMenu* widgetsMenu = menuBar()->addMenu("&Widgets");
    QAction* showImageAction = new QAction("Show &Image", this);
    connect(showImageAction, &QAction::triggered, this, &MainWindow::showImage_);
    widgetsMenu->addAction(showImageAction);

    QAction* showLabelAction = new QAction("Show &Label", this);
    connect(showLabelAction, &QAction::triggered, this, &MainWindow::showLabel_);
    widgetsMenu->addAction(showLabelAction);

    QAction* showCompositeAction = new QAction("Show &Composite Widget", this);
    connect(showCompositeAction, &QAction::triggered, this, &MainWindow::showCompositeWidget_);
    widgetsMenu->addAction(showCompositeAction);
  }

protected:
  /** Called when the application is closing to cleanup */
  virtual void closeEvent(QCloseEvent* event) override
  {
    // Remove all of the widgets from the HUD
    for (const auto& hudElement : hudElements_)
      hudCamera_->removeChild(hudElement.first);
  }

private:
  void showImage_()
  {
    static QImage testImage = generateTestImage(256, 256);

    // Qt Version
    QLabel* qtImageLabel = new QLabel;
    qtImageLabel->setPixmap(QPixmap::fromImage(testImage));
    qtImageLabel->setWindowTitle("Qt Image");
    qtImageLabel->show();

    // OSG Version
    osg::ref_ptr<simQt::QImageNode> imageNode = new simQt::QImageNode;
    imageNode->setImage(testImage);

    // Add to hud
    osg::ref_ptr<osg::MatrixTransform> hudXform = createHudTransform(650.f, 340.f, 1.f);
    hudXform->addChild(imageNode.get());
    hudCamera_->addChild(hudXform.get());

    hudElements_.push_back(std::make_pair(hudXform, imageNode));
  }

  void showLabel_()
  {
    // Qt Version
    QLabel* qtLabel = new QLabel("This is a long label that will word wrap in Qt.\nYou can add\nmultiple lines to Qt labels and it will render correctly.");
    qtLabel->setWordWrap(true);
    qtLabel->setWindowTitle("Qt Label");
    qtLabel->setStyleSheet("font-size: 30px; color: forestgreen;");
    qtLabel->resize(300, 300);
    qtLabel->show();

    // OSG Version
    osg::ref_ptr<simQt::QLabelDropShadowNode> labelNode = new simQt::QLabelDropShadowNode;
    labelNode->render(qtLabel);

    // re-render on a timer, to catch GUI window updates. In real code you'd want to connect
    // this to some signal that monitors the dimensions or content but in this simple example
    // we're just using a simple timer.
    QTimer* renderTimer = new QTimer(qtLabel);
    renderTimer->setInterval(500);
    renderTimer->setSingleShot(false);
    connect(renderTimer, &QTimer::timeout, qtLabel, [qtLabel, labelNode]() {
      labelNode->render(qtLabel);
      });
    renderTimer->start();

    // Add to hud
    osg::ref_ptr<osg::MatrixTransform> hudXform = createHudTransform(100.f, 400.f, 1.f);
    hudXform->addChild(labelNode.get());
    hudCamera_->addChild(hudXform.get());

    hudElements_.push_back(std::make_pair(hudXform, labelNode));
  }

  void showCompositeWidget_()
  {
    // Qt Version
    QWidget* compositeWidget = new QWidget;
    // Need to auto-fill background or the background is transparent, which looks bad on the scene
    compositeWidget->setAutoFillBackground(true);
    QFormLayout* layout = new QFormLayout(compositeWidget);
    QLineEdit* lineEdit = new QLineEdit("Enter Text");
    QSpinBox* spinBox = new QSpinBox;
    QCheckBox* checkBox = new QCheckBox("Enable");
    QPushButton* button = new QPushButton("Click Me");

    layout->addRow("Text:", lineEdit);
    layout->addRow("Number:", spinBox);
    layout->addRow("Enabled:", checkBox);
    layout->addRow(button);

    compositeWidget->setWindowTitle("Qt Composite Widget");
    compositeWidget->resize(200, 150);
    compositeWidget->show();

    // OSG Version
    osg::ref_ptr<simQt::QWidgetNode> widgetNode = new simQt::QWidgetNode;
    widgetNode->render(compositeWidget);

    // re-render on a timer, to catch GUI window updates. In real code you'd want to connect
    // this to some signal that monitors the dimensions or content but in this simple example
    // we're just using a simple timer.
    QTimer* renderTimer = new QTimer(compositeWidget);
    renderTimer->setInterval(500);
    renderTimer->setSingleShot(false);
    connect(renderTimer, &QTimer::timeout, compositeWidget, [compositeWidget, widgetNode]() {
      widgetNode->render(compositeWidget);
      });
    renderTimer->start();

    // Add to hud
    osg::ref_ptr<osg::MatrixTransform> hudXform = createHudTransform(100.f, 120.f, 1.f);
    hudXform->addChild(widgetNode.get());
    hudCamera_->addChild(hudXform.get());

    hudElements_.push_back(std::make_pair(hudXform, widgetNode));
  }

private:
  osg::ref_ptr<simVis::View> mainView_;
  osg::ref_ptr<osg::Camera> hudCamera_;
  std::vector<std::pair<osg::ref_ptr<osg::MatrixTransform>, osg::ref_ptr<simQt::QImageBasedNode>>> hudElements_;
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
  osg::ref_ptr<simVis::View> mainView = new simVis::View();
  mainView->setSceneManager(sceneMan.get());
  mainView->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  mainView->installBasicDebugHandlers();

  // The ViewManager coordinates the rendering of all our views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager();
  viewMan->addView(mainView.get());

  QApplication app(argc, argv);

  // Force light mode for now until we fully support dark mode
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
  app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif

  MainWindow mainWindow(mainView.get());
  mainWindow.resize(1024, 768);
  mainWindow.show();

  return app.exec();
}
