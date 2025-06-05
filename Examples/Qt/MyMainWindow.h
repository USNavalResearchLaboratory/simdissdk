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
#ifndef EXAMPLES_QT_MYMAINWINDOW_H
#define EXAMPLES_QT_MYMAINWINDOW_H

#include <QMainWindow>
#include "simVis/Utils.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simQt/ViewerWidgetAdapter.h"
#include "simUtil/StatsHandler.h"

// custom MainWindow that runs a timer against the ViewManager.
class MyMainWindow : public QMainWindow
{
  Q_OBJECT;

public:
  explicit MyMainWindow(simVis::ViewManager* viewMan)
  {
    viewMan_ = viewMan;

    viewerWidget_ = new simQt::ViewerWidgetAdapter(this);
    viewerWidget_->setViewer(viewMan_->getViewer());
    setCentralWidget(viewerWidget_);

    statsHandler_ = new simUtil::StatsHandler;
    simVis::fixStatsHandlerGl2BlockyText(statsHandler_.get());
    osg::observer_ptr<simVis::View> mainView = viewMan_->getView(0);
    if (mainView.valid())
      mainView->addEventHandler(statsHandler_.get());
  }

public Q_SLOTS:

  void setTimerInterval(int value)
  {
    viewerWidget_->setTimerInterval(value);
  }

  void toggleFrameRate(bool turnOn)
  {
    // Pick the StatsType based on turnOn flag
    simUtil::StatsHandler::StatsType type =
      (turnOn ? simUtil::StatsHandler::FRAME_RATE :
                simUtil::StatsHandler::NO_STATS);
    // Update the stats type in the handler
    statsHandler_->setStatsType(type, viewMan_->getView(0));
  }

protected:
  osg::ref_ptr<simVis::ViewManager>   viewMan_;
  osg::ref_ptr<simUtil::StatsHandler> statsHandler_;
  simQt::ViewerWidgetAdapter* viewerWidget_ = nullptr;
};

#endif /* EXAMPLES_QT_MYMAINWINDOW_H */
