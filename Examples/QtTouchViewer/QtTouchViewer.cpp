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
/**
 * ViewManager Qt Data Model -
 * Demonstrates basic use of the simQt::ViewManagerDataModel class with a Qt UI.
 */

#include <cstdlib>
#include <QApplication>
#include <QDockWidget>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QStandardItemModel>
#include <QTreeView>
#include <QTouchDevice>
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simQt/ViewManagerDataModel.h"
#include "simQt/ViewWidget.h"
#include "simUtil/ExampleResources.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/SceneManager.h"
#include "MainWindow.h"

/** Forwards event adapter content to the main window */
class ForwardTouchEvents : public osgGA::GUIEventHandler
{
public:
  explicit ForwardTouchEvents(MainWindow* mainWindow)
    : mainWindow_(mainWindow)
  {
  }

  // From GUIEventHandler:
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override
  {
    if (mainWindow_)
      mainWindow_->processOsgEvent(ea);
    return osgGA::GUIEventHandler::handle(ea, aa);
  }

private:
  QPointer<MainWindow> mainWindow_;
};

///////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  // create a viewer manager. The "args" are optional.
  viewMan_ = new simVis::ViewManager();
  // Note that the logarithmic depth buffer is not installed

  // disable the default ESC-to-quit event:
  viewMan_->getViewer()->setKeyEventSetsDone(0);
  viewMan_->getViewer()->setQuitEventSetsDone(false);

  addTouchDevicesDock_();
  addMostRecentDock_();

  // timer fires a paint event.
  connect(&timer_, SIGNAL(timeout()), this, SLOT(update()));
  // timer single shot to avoid infinite loop problems in Qt on MSVC11
  timer_.setSingleShot(true);
  timer_.start(10);
}

void MainWindow::paintEvent(QPaintEvent* e)
{
  // refresh all the views.
  viewMan_->frame();
  timer_.start();
}

simVis::ViewManager* MainWindow::getViewManager() const
{
  return viewMan_.get();
}

void MainWindow::addTouchDevicesDock_()
{
  const auto& allDevices = QTouchDevice::devices();
  QDockWidget* listDock = new QDockWidget(tr("Touch Devices"), this);
  addDockWidget(Qt::LeftDockWidgetArea, listDock);

  if (allDevices.empty())
  {
    listDock->setWidget(new QLabel(tr("No touch devices detected"), this));
    return;
  }

  // Fill out a standard item model with the devices
  QStandardItemModel* model = new QStandardItemModel(allDevices.size(), 3, this);
  model->setHeaderData(0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  model->setHeaderData(1, Qt::Horizontal, tr("Type"), Qt::DisplayRole);
  model->setHeaderData(2, Qt::Horizontal, tr("#Points"), Qt::DisplayRole);
  for (int rowNum = 0; rowNum < allDevices.size(); ++rowNum)
  {
    const auto& device = allDevices[rowNum];
    if (device->name().isEmpty())
      model->setItem(rowNum, 0, new QStandardItem(tr("<none>")));
    else
      model->setItem(rowNum, 0, new QStandardItem(device->name()));
    if (device->type() == QTouchDevice::TouchPad)
      model->setItem(rowNum, 1, new QStandardItem(tr("Touchpad")));
    else
      model->setItem(rowNum, 1, new QStandardItem(tr("Touchscreen")));
    model->setItem(rowNum, 2, new QStandardItem(QString::number(device->maximumTouchPoints())));
  }

  QTreeView* deviceList = new QTreeView(this);
  deviceList->setEditTriggers(QAbstractItemView::NoEditTriggers);
  deviceList->setRootIsDecorated(false);
  deviceList->setModel(model);
  deviceList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  listDock->setWidget(deviceList);
}

void MainWindow::addMostRecentDock_()
{
  recentX_ = new QLineEdit(this);
  recentX_->setReadOnly(true);

  recentY_ = new QLineEdit(this);
  recentY_->setReadOnly(true);

  recentEvent_ = new QLineEdit(this);
  recentEvent_->setReadOnly(true);

  recentTouchPts_ = new QLineEdit(this);
  recentTouchPts_->setReadOnly(true);

  touchValues_ = new QStandardItemModel(this);
  touchValues_->setHorizontalHeaderLabels({ tr("X"), tr("Y"), tr("State"), tr("ID") });
  QTreeView* pointList = new QTreeView(this);
  pointList->setEditTriggers(QAbstractItemView::NoEditTriggers);
  pointList->setRootIsDecorated(false);
  pointList->setModel(touchValues_);
  pointList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  QWidget* recentStateWidget = new QWidget(this);

  QGridLayout* layout = new QGridLayout(recentStateWidget);
  layout->addWidget(new QLabel(tr("X:"), this), 0, 0);
  layout->addWidget(recentX_, 0, 1);
  layout->addWidget(new QLabel(tr("Y:"), this), 1, 0);
  layout->addWidget(recentY_, 1, 1);
  layout->addWidget(new QLabel(tr("Evt:"), this), 2, 0);
  layout->addWidget(recentEvent_, 2, 1);
  layout->addWidget(new QLabel(tr("# Pts:"), this), 3, 0);
  layout->addWidget(recentTouchPts_, 3, 1);
  layout->addWidget(pointList, 4, 0, 1, 2);

  QDockWidget* dock = new QDockWidget(tr("Recent State"), this);
  dock->setWidget(recentStateWidget);
  addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::processOsgEvent(const osgGA::GUIEventAdapter& ea)
{
  // Only care about push, drag/move, and release events
  switch (ea.getEventType())
  {
  case osgGA::GUIEventAdapter::PUSH:
    recentEvent_->setText(tr("PUSH"));
    break;
  case osgGA::GUIEventAdapter::RELEASE:
    recentEvent_->setText(tr("RELEASE"));
    break;
  case osgGA::GUIEventAdapter::MOVE:
    recentEvent_->setText(tr("MOVE"));
    break;
  case osgGA::GUIEventAdapter::DRAG:
    recentEvent_->setText(tr("DRAG"));
    break;
  default: // ignore all others with a return
    return;
  }

  recentX_->setText(QString::number(ea.getX()));
  recentY_->setText(QString::number(ea.getY()));

  const auto* touchData = ea.getTouchData();
  const int numTouchPoints = (touchData == nullptr) ? 0 : touchData->getNumTouchPoints();
  recentTouchPts_->setText(QString::number(numTouchPoints));

  if (touchData)
  {
    int row = 0;
    for (const osgGA::GUIEventAdapter::TouchData::TouchPoint& point : *touchData)
    {
      touchValues_->setItem(row, 0, new QStandardItem(QString::number(point.x)));
      touchValues_->setItem(row, 1, new QStandardItem(QString::number(point.y)));

      QString phase = tr("Unknown");
      switch (point.phase)
      {
      case osgGA::GUIEventAdapter::TOUCH_UNKNOWN:
        break;
      case osgGA::GUIEventAdapter::TOUCH_BEGAN:
        phase = tr("Began");
        break;
      case osgGA::GUIEventAdapter::TOUCH_MOVED:
        phase = tr("Moved");
        break;
      case osgGA::GUIEventAdapter::TOUCH_STATIONERY:
        phase = tr("Stationary");
        break;
      case osgGA::GUIEventAdapter::TOUCH_ENDED:
        phase = tr("Ended");
        break;
      }

      touchValues_->setItem(row, 2, new QStandardItem(phase));
      touchValues_->setItem(row, 3, new QStandardItem(QString::number(point.id)));
      ++row;
    }
  }

  // Remove unused rows
  if (numTouchPoints < touchValues_->rowCount())
    touchValues_->removeRows(numTouchPoints, touchValues_->rowCount() - numTouchPoints);
}

///////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  // Configure a map, scene manager, and default sky node
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());
  simExamples::addDefaultSkyNode(sceneMan.get());

  // Set up the Qt application, main window, and central widget.
  QApplication qapp(argc, argv);
  MainWindow win;
  win.setGeometry(50, 50, 1024, 768);
  QWidget* center = new QWidget();
  center->setLayout(new QHBoxLayout());
  win.setCentralWidget(center);

  // Make a view, which is needed to instantiate a ViewWidget, which is the OSG display
  osg::ref_ptr<simVis::View> mainview = new simVis::View();
  mainview->setName("Main View");
  mainview->setSceneManager(sceneMan.get());
  // Note that the view manager here is owned by the window
  win.getViewManager()->addView(mainview.get());

  // Make a Qt Widget to hold our view, and add that widget to the main window.
  QWidget* viewWidget = new simQt::ViewWidget(mainview.get());
  center->layout()->addWidget(viewWidget);

  // Add one inset to the top-right
  osg::ref_ptr<simVis::View> inset = new simVis::View();
  // set up the new inset's extents as a percentage of the parent's size.
  inset->setExtents(simVis::View::Extents(0.65, 0.65, 0.35, 0.35, true));
  inset->setSceneManager(sceneMan.get());
  inset->setName("Inset");
  // Copy the earth manipulator settings from the parent
  inset->applyManipulatorSettings(*mainview);
  mainview->addInset(inset.get());

  // Forward all touch-related GUI events to the main window
  osg::ref_ptr<ForwardTouchEvents> fwdEvents(new ForwardTouchEvents(&win));
  mainview->addEventHandler(fwdEvents);
  inset->addEventHandler(fwdEvents);

  // fire up the GUI.
  win.show();
  qapp.exec();
}
