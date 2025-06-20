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
/**
 * ViewManager Qt Data Model -
 * Demonstrates basic use of the simQt::ViewManagerDataModel class with a Qt UI.
 */

#include <cassert>
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/System/Utils.h"
#include "simUtil/ExampleResources.h"

#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/SceneManager.h"

#include "simQt/ViewManagerDataModel.h"
#include "simQt/ViewerWidgetAdapter.h"

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QLayout>
#include <QDockWidget>
#include <QTreeView>
#include <QListView>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#include <cstdlib>

#include "MainWindow.h"

int usage(char** argv)
{
  SIM_NOTICE << argv[0] << "\n"
    << "    --views [n]         : open 'n' views"
    << std::endl;

  return 0;
}


MainWindow::MainWindow()
  : QMainWindow(),
    numInsetsCreated_(0)
{
  // create a viewer manager. The "args" are optional.
  viewMan_ = new simVis::ViewManager();

  // View Manager will support multiple top level CompositeViewer instances for osgQOpenGL
  viewMan_->setUseMultipleViewers(true);

  // Note that the logarithmic depth buffer is not installed

  // Create a set of buttons on the side to add/remove views
  QWidget* buttonWidget = new QWidget(this);
  QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  QPushButton* addButton = new QPushButton("Add", this);
  QPushButton* removeButton = new QPushButton("Remove", this);
  buttonLayout->addWidget(addButton);
  buttonLayout->addWidget(removeButton);
  QDockWidget* buttonDock = new QDockWidget("Actions");
  buttonDock->setWidget(buttonWidget);
  addDockWidget(Qt::LeftDockWidgetArea, buttonDock);

  // Create a few interesting dock widget GUIs
  QDockWidget* treeDock = new QDockWidget("Tree View 1", this);
  topTreeView_ = new QTreeView(this);
  topTreeView_->setHeaderHidden(true);
  treeDock->setWidget(topTreeView_);
  addDockWidget(Qt::LeftDockWidgetArea, treeDock);

  // List view
  QDockWidget* listDock = new QDockWidget("List View", this);
  QListView* listView = new QListView(this);
  listDock->setWidget(listView);
  addDockWidget(Qt::LeftDockWidgetArea, listDock);

  // List view flat
  QDockWidget* flatListDock = new QDockWidget("Flat List", this);
  QListView* flatListView = new QListView(this);
  flatListDock->setWidget(flatListView);
  addDockWidget(Qt::LeftDockWidgetArea, flatListDock);
  tabifyDockWidget(flatListDock, listDock);

  // Second tree view
  QDockWidget* treeDock2 = new QDockWidget("Tree View 2", this);
  QTreeView* treeView2 = new QTreeView(this);
  treeView2->setSortingEnabled(true);
  treeView2->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked);
  treeDock2->setWidget(treeView2);
  addDockWidget(Qt::LeftDockWidgetArea, treeDock2);

  // No-checks tree view
  QDockWidget* noChecksTreeDock = new QDockWidget("No Checks", this);
  QWidget* noChecksWidget = new QWidget(this);
  QVBoxLayout* noChecksVBox = new QVBoxLayout();
    noChecksVBox->setContentsMargins(0, 0, 0, 0);
  QTreeView* noChecksTreeView = new QTreeView(this);
  QHBoxLayout* noChecksButtonsHBox = new QHBoxLayout();
  QPushButton* toggleChecks = new QPushButton("Show Checks", this);
    toggleChecks->setCheckable(true);
  QPushButton* toggleTree = new QPushButton("Tree Mode", this);
    toggleTree->setCheckable(true);
    toggleTree->setChecked(true);
  // Set all the layouts and positioning
  noChecksTreeDock->setWidget(noChecksWidget);
  noChecksWidget->setLayout(noChecksVBox);
  noChecksVBox->addLayout(noChecksButtonsHBox);
  noChecksVBox->addWidget(noChecksTreeView);
  noChecksButtonsHBox->addWidget(toggleChecks);
  noChecksButtonsHBox->addWidget(toggleTree);
  addDockWidget(Qt::LeftDockWidgetArea, noChecksTreeDock);
  tabifyDockWidget(treeDock2, noChecksTreeDock);

  // Set up the data model, bind to view manager, and set the model for all 3 views
  simQt::ViewManagerDataModel* dataModel = new simQt::ViewManagerDataModel(this);
  dataModel->bindTo(viewMan_.get());
  listView->setModel(dataModel);
  topTreeView_->setModel(dataModel);
  treeView2->setModel(dataModel);

  // Create a flat data model
  simQt::ViewManagerDataModel* flatDataModel = new simQt::ViewManagerDataModel(this);
  flatDataModel->setHierarchical(false);
  flatDataModel->bindTo(viewMan_.get());
  flatListView->setModel(flatDataModel);

  // Create a data model with no checks
  simQt::ViewManagerDataModel* noChecksDataModel = new simQt::ViewManagerDataModel(this);
  noChecksDataModel->bindTo(viewMan_.get());
  noChecksDataModel->setUserCheckable(false);
  noChecksTreeView->setModel(noChecksDataModel);

  connect(addButton, SIGNAL(clicked()), this, SLOT(addView()));
  connect(removeButton, SIGNAL(clicked()), this, SLOT(removeView()));
  connect(dataModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), topTreeView_, SLOT(expandAll()));
  connect(dataModel, SIGNAL(modelReset()), topTreeView_, SLOT(expandAll()));
  connect(dataModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), treeView2, SLOT(expandAll()));
  connect(dataModel, SIGNAL(modelReset()), treeView2, SLOT(expandAll()));
  connect(noChecksDataModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), noChecksTreeView, SLOT(expandAll()));
  connect(noChecksDataModel, SIGNAL(modelReset()), noChecksTreeView, SLOT(expandAll()));
  connect(toggleTree, SIGNAL(toggled(bool)), noChecksDataModel, SLOT(setHierarchical(bool)));
  connect(toggleChecks, SIGNAL(toggled(bool)), noChecksDataModel, SLOT(setUserCheckable(bool)));
}

simVis::ViewManager* MainWindow::getViewManager() const
{
  return viewMan_.get();
}

void MainWindow::addMainView(simVis::View* mainView)
{
  mainView->setName("Main View");
  mainViews_.push_back(mainView);
}

void MainWindow::addView()
{
  if (mainViews_.empty())
    return;

  // Add a new inset for each main view
  for (std::vector<osg::observer_ptr<simVis::View> >::iterator iter = mainViews_.begin(); iter != mainViews_.end(); ++iter)
  {
    if (!iter->valid())
      continue;

    osg::ref_ptr<simVis::View> inset = new simVis::View();
    // Get X and Y values between 0 and 0.9
    float x = (0.9f * rand()) / RAND_MAX;
    float y = (0.9f * rand()) / RAND_MAX;
    // Verify math to ensure we're within bounds
    assert(x <= 0.9f && x >= 0.f);
    assert(y <= 0.9f && y >= 0.f);
    // Get reasonable width values
    float w = 0.1f + (0.9f - x) * rand() / RAND_MAX;
    float h = 0.1f + (0.9f - y) * rand() / RAND_MAX;
    // Verify math to make sure we don't overrun the edge of the screen
    assert(x + w <= 1.f);
    assert(y + h <= 1.f);

    // set up the new inset's extents as a percentage of the parent's size.
    inset->setExtents(simVis::View::Extents(x, y, w, h, true));
    inset->setSceneManager((*iter)->getSceneManager());
    inset->setName(osgEarth::Util::Stringify() << "New " << (++numInsetsCreated_));
    // Copy the Earth Manipulator settings from the parent view
    inset->applyManipulatorSettings(**iter);
    (*iter)->addInset(inset.get());
  }
}

void MainWindow::removeView()
{
  QVariant data = topTreeView_->model()->data(topTreeView_->currentIndex(), simQt::ViewManagerDataModel::VIEW_ROLE);
  simVis::View* selectedView = static_cast<simVis::View*>(data.value<void*>());
  if (selectedView == nullptr || mainViews_.empty())
    return;

  simVis::View* hostView = selectedView->getHostView();
  if (hostView)
    hostView->removeInset(selectedView);
}


int main(int argc, char** argv)
{
  simCore::initializeSimdisEnvironmentVariables();
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  if (arguments.read("--help"))
    return usage(argv);

  // read the number of views to open:
  int numViews = 2;
  arguments.read("--views", numViews);

  // First we need a map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // A scene manager that all our views will share.
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());

  // add sky node
  simExamples::addDefaultSkyNode(sceneMan.get());

  // OK, time to set up the Qt Application and windows.
  QApplication qapp(argc, argv);

  // Our custom main window contains a ViewManager.
  MainWindow win;
  win.setGeometry(50, 50, 150 + 400*numViews, 400);
  QWidget* center = new QWidget();
  center->setLayout(new QHBoxLayout());
  win.setCentralWidget(center);

  // Create views and connect them to our scene.
  for (int i = 0; i < numViews; ++i)
  {
    // Make a view, hook it up, and add it to the view manager.
    osg::ref_ptr<simVis::View> mainview = new simVis::View();
    win.addMainView(mainview.get());
    // Note the artificial scopes below ensure separation of concerns

    {
      // Make a Qt Widget to hold our view, and add that widget to the
      // main window.
      auto* viewerWidget = new simQt::ViewerWidgetAdapter(simQt::GlImplementation::Window, &win);
      viewerWidget->setTimerInterval(10);
      center->layout()->addWidget(viewerWidget);

      // attach the scene manager and add it to the view manager.
      mainview->setSceneManager(sceneMan.get());
      win.getViewManager()->addView(mainview.get());
      viewerWidget->setViewer(win.getViewManager()->getViewer(mainview.get()));
    }

    {
      // Each top-level view needs an Inset controller so the user can draw
      // and interact with inset views.
      osg::ref_ptr<simVis::View> inset = new simVis::View();
      // set up the new inset's extents as a percentage of the parent's size.
      inset->setExtents(simVis::View::Extents(0.2, 0.2, 0.5, 0.5, true));
      inset->setSceneManager(sceneMan.get());
      inset->setName(osgEarth::Util::Stringify() << "Inset " << (i+1) << " (1/2)");
      // Copy the earth manipulator settings from the parent
      inset->applyManipulatorSettings(*mainview);
      mainview->addInset(inset.get());
    }

    {
      // set up the new inset's extents as a percentage of the parent's size.
      osg::ref_ptr<simVis::View> inset2 = new simVis::View();
      inset2->setExtents(simVis::View::Extents(0.7, 0.6, 0.2, 0.2, true));
      inset2->setSceneManager(sceneMan.get());
      inset2->setName(osgEarth::Util::Stringify() << "Inset " << (i+1) << " (2/2)");
      // Copy the earth manipulator settings from the parent
      inset2->applyManipulatorSettings(*mainview);
      mainview->addInset(inset2.get());
    }
  }

  // fire up the GUI.
  win.show();
  qapp.exec();
}
