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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QGLWidget>
#include <QWindow>
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "Reader.h"
#include "Gui.h"
#include "MyMainWindow.h"

namespace SdkQThreadExample{

MyMainWindow::MyMainWindow(simVis::ViewManager* viewMan, simData::DataStore& dataStore)
  : viewManager_(viewMan),
    dataStore_(dataStore),
    reader_(nullptr),
    generatorDialog_(nullptr)
{
  // disable the default ESC-to-quit event:
  viewManager_->getViewer()->setKeyEventSetsDone(0);
  viewManager_->getViewer()->setQuitEventSetsDone(false);

  // timer fires a paint event.
  connect(&redrawTimer_, SIGNAL(timeout()), this, SLOT(update()));
  // timer single shot to avoid infinite loop problems in Qt on MSVC11
  redrawTimer_.setSingleShot(true);
  redrawTimer_.start(20);

  _statsHandler = new simUtil::StatsHandler;
  simVis::fixStatsHandlerGl2BlockyText(_statsHandler.get());
  osg::observer_ptr<simVis::View> mainView = viewManager_->getView(0);
  if (mainView.valid())
  {
    mainView->addEventHandler(_statsHandler.get());

    // Set an initial viewpoint near the data
    simVis::Viewpoint vp;
    vp.focalPoint()->set(osgEarth::SpatialReference::get("wgs84"), osg::Vec3d(-159.3, 22.4, 0.0), osgEarth::ALTMODE_ABSOLUTE);
    vp.pitch()->set(-45.0, osgEarth::Units::DEGREES);
    vp.heading()->set(25.0, osgEarth::Units::DEGREES);
    vp.range()->set(1e5, osgEarth::Units::METERS);
    mainView->setViewpoint(vp);
  }
}

MyMainWindow::~MyMainWindow()
{
  delete generatorDialog_;
  delete reader_;
}

void MyMainWindow::setGlWidget(QGLWidget* glWidget)
{
  setCentralWidget(glWidget);
  glWindow_ = glWidget->windowHandle();
}

void MyMainWindow::paintEvent(QPaintEvent* e)
{
  // refresh all the views.
  if (glWindow_ && glWindow_->isExposed())
    viewManager_->frame();
  redrawTimer_.start();

  // Update the GUI at the slow rate of the paintEvent instead of at the data rate
  if ((reader_ != nullptr) && (generatorDialog_ != nullptr))
    generatorDialog_->updateNumberProcessed(reader_->numberProcessed());
}

void MyMainWindow::showGenerateDialog()
{
  if (reader_ == nullptr)
  {
    reader_ = new Reader(dataStore_);
  }

  if (generatorDialog_ == nullptr)
  {
    generatorDialog_ = new Gui(this);
    // When the user click the Start button signal the reader to start
    QObject::connect(generatorDialog_, SIGNAL(startClicked()), reader_, SLOT(start()));
    // When the user clicks the Stop button signal the reader  to stop
    QObject::connect(generatorDialog_, SIGNAL(stopClicked()), reader_, SLOT(stop()));
  }

  generatorDialog_->show();
}

}
