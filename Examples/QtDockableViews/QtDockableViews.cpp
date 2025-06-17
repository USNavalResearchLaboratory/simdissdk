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
/* -*-c++-*- */
/**
 * Qt Dockable Views
 * Demonstrates using simVis::View objects in QDockWidgets with a QMainWindow
 */

#include "osgEarth/Registry"

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/System/Utils.h"
#include "simQt/ViewerWidgetAdapter.h"
#include "simUtil/ExampleResources.h"

#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/SceneManager.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDockWidget>
#include <QLayout>
#include <QMainWindow>
#include <QResizeEvent>
#include <QTimer>
#include <QToolBar>
#include <QWindow>

#include "MyMainWindow.h"

int usage(char** argv)
{
  SIM_NOTICE << argv[0] << "\n"
    << "    --framerate [n]     : set the framerate"
    << std::endl;

  return 0;
}

////////////////////////////////////////////////////////////////////
MyMainWindow::MyMainWindow(int framerate)
  : timerInterval_(1000 / std::max(1, framerate))
{
  // create toolbar
  QToolBar* toolbar = new QToolBar(this);
  QAction* dialogAction = new QAction(tr("New Dialog"), this);
  QAction* dockableAction = new QAction(tr("New Dockable"), this);
  QAction* mainViewAction = new QAction(tr("New Main View Pane"), this);
  toolbar->addAction(dialogAction);
  toolbar->addAction(dockableAction);
  toolbar->addAction(mainViewAction);
  addToolBar(Qt::TopToolBarArea, toolbar);

  // set a blank central widget
  QWidget* center = new QWidget(this);
  center->setLayout(new QHBoxLayout());
  center->layout()->setMargin(0);
  setCentralWidget(center);

  // we need a map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // A scene manager that all our views will share.
  sceneMan_ = new simVis::SceneManager();
  sceneMan_->setMap(map.get());

  // add sky node
  simExamples::addDefaultSkyNode(sceneMan_.get());

  // create our first widget, seems to be required on startup
  createViewDockable_();

  // connect actions to our slots
  connect(dialogAction, SIGNAL(triggered()), this, SLOT(createViewDialog_()));
  connect(dockableAction, SIGNAL(triggered()), this, SLOT(createViewDockable_()));
  connect(mainViewAction, SIGNAL(triggered()), this, SLOT(createMainView_()));

}

MyMainWindow::~MyMainWindow()
{
}

simQt::ViewerWidgetAdapter* MyMainWindow::newWidget_(const QString& viewName)
{
  auto* viewManager = new simVis::ViewManager;
  viewManagers_.push_back(viewManager);
  osg::ref_ptr<simVis::View> view = createView_(*viewManager, viewName);

  simQt::ViewerWidgetAdapter* viewWidget = new simQt::ViewerWidgetAdapter(this);
  viewWidget->setViewer(viewManager->getViewer());
  viewWidget->setTimerInterval(timerInterval_);
  viewWidget->setMinimumSize(2, 2);
  viewWidget->resize(100, 100);
  return viewWidget;
}

void MyMainWindow::createViewDialog_()
{
  const QString viewName = tr("Dialog View %1").arg(viewCounter_++);

  // now create a dock widget for each inset
  QDialog* dialog = new QDialog(this);
  dialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
  dialog->setWindowTitle(viewName);
  dialog->setLayout(new QHBoxLayout());
  dialog->layout()->setMargin(0);
  dialog->layout()->addWidget(newWidget_(viewName));
  dialog->resize(100, 100);
  dialog->show();
}

void MyMainWindow::createViewDockable_()
{
  const QString viewName = tr("Dockable View %1").arg(viewCounter_++);

  // now create a dock widget for each inset
  QDockWidget* dockable = new QDockWidget(this);
  dockable->setWidget(newWidget_(viewName));
  dockable->setWindowTitle(viewName);
  dockable->resize(100, 100);
  addDockWidget(Qt::RightDockWidgetArea, dockable);
}

void MyMainWindow::createMainView_()
{
  // Make a main view, hook it up, and add it to the view manager.
  const QString viewName = tr("Main View %1").arg(viewCounter_++);

  // Make a Qt Widget to hold our view, and add that widget to the main window.
  centralWidget()->layout()->addWidget(newWidget_(viewName));
}

simVis::View* MyMainWindow::createView_(simVis::ViewManager& viewManager, const QString& name) const
{
  simVis::View* view = new simVis::View();
  view->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  view->setName(name.toStdString());

  // attach the scene manager and add it to the view manager.
  view->setSceneManager(sceneMan_.get());
  viewManager.addView(view);
  view->installDebugHandlers();

  // by default, the database pager unreferenced image objects once it downloads them
  // the driver. In composite viewer mode we don't want that since we may be adding
  // and removing views.  This may use more memory, but it's a requirement for multiple GCs.
  view->getScene()->getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, false);

  return view;
}
////////////////////////////////////////////////////////////////////

void warningMessageFilter(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  // Block spammed warning message from setGeometry() calls caused when manually resizing a QDialog.
  // This is a known Qt bug in Qt 5.15 that is unresolved: https://bugreports.qt.io/browse/QTBUG-73258
#ifndef NDEBUG
  if (type != QtWarningMsg || !msg.startsWith("QWindowsWindow::setGeometry"))
#endif
  {
    QByteArray localMsg = msg.toLocal8Bit();
    fprintf(stdout, "%s", localMsg.constData());
  }

}

int main(int argc, char** argv)
{
  simCore::initializeSimdisEnvironmentVariables();
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  if (arguments.read("--help"))
    return usage(argv);

  // Need to turn off the un-ref image data after apply, else the multiple graphics
  // contexts will attempt to grab images that no longer exist.  This should be called
  // if you expect multiple graphics contexts rendering the same scene.
  osgEarth::Registry::instance()->unRefImageDataAfterApply() = false;

  // read the framerate
  int framerate = 20;
  arguments.read("--framerate", framerate);
  if (framerate <= 0)
    framerate = 20;

  // OK, time to set up the Qt Application and windows.
  qInstallMessageHandler(warningMessageFilter);
  QApplication qapp(argc, argv);

  // Our custom main window contains a ViewManager.
  MyMainWindow win(framerate);
  win.setGeometry(200, 400, 400, 400);

  // fire up the GUI.
  win.show();
  qapp.exec();
  return 0;
}
