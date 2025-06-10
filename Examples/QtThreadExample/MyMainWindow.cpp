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
#include <QTimer>
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "Reader.h"
#include "Gui.h"
#include "MyMainWindow.h"

namespace SdkQThreadExample{

MyMainWindow::MyMainWindow(simVis::ViewManager* viewMan, simData::DataStore& dataStore)
  : viewManager_(viewMan),
    dataStore_(dataStore)
{
  statsHandler_ = new simUtil::StatsHandler;
  simVis::fixStatsHandlerGl2BlockyText(statsHandler_.get());
  osg::observer_ptr<simVis::View> mainView = viewManager_->getView(0);
  if (mainView.valid())
  {
    mainView->addEventHandler(statsHandler_.get());

    // Set an initial viewpoint near the data
    simVis::Viewpoint vp;
    vp.focalPoint()->set(osgEarth::SpatialReference::get("wgs84"), osg::Vec3d(-159.3, 22.4, 0.0), osgEarth::ALTMODE_ABSOLUTE);
    vp.pitch()->set(-45.0, osgEarth::Units::DEGREES);
    vp.heading()->set(25.0, osgEarth::Units::DEGREES);
    vp.range()->set(1e5, osgEarth::Units::METERS);
    mainView->setViewpoint(vp);
  }

  QTimer* updatedNumProcessedTimer = new QTimer(this);
  updatedNumProcessedTimer->setInterval(500); // twice per second
  updatedNumProcessedTimer->setSingleShot(false);
  // Update the GUI at the slow rate of the paintEvent instead of at the data rate
  connect(updatedNumProcessedTimer, &QTimer::timeout, this, [this]() {
    if (reader_ && generatorDialog_)
      generatorDialog_->updateNumberProcessed(reader_->numberProcessed());
  });
  updatedNumProcessedTimer->start();
}

MyMainWindow::~MyMainWindow()
{
  delete generatorDialog_;
  delete reader_;
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
