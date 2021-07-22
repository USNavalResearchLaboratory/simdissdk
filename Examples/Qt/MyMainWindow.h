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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef EXAMPLES_QT_MYMAINWINDOW_H
#define EXAMPLES_QT_MYMAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QSignalMapper>
#include <QGLWidget>
#include <QWindow>

#include "simVis/Utils.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simUtil/StatsHandler.h"

// custom MainWindow that runs a timer against the ViewManager.
class MyMainWindow : public QMainWindow
{
  Q_OBJECT;

public:
  explicit MyMainWindow(simVis::ViewManager* viewMan)
    : glWindow_(nullptr)
  {
    viewMan_ = viewMan;

    // disable the default ESC-to-quit event:
    viewMan_->getViewer()->setKeyEventSetsDone(0);
    viewMan_->getViewer()->setQuitEventSetsDone(false);

    // timer fires a paint event.
    connect(&timer_, SIGNAL(timeout()), this, SLOT(update()));
    // timer single shot to avoid infinite loop problems in Qt on MSVC11
    timer_.setSingleShot(true);
    timer_.start(20);

    statsHandler_ = new simUtil::StatsHandler;
    simVis::fixStatsHandlerGl2BlockyText(statsHandler_.get());
    osg::observer_ptr<simVis::View> mainView = viewMan_->getView(0);
    if (mainView.valid())
      mainView->addEventHandler(statsHandler_.get());
  }

  void setGlWidget(QGLWidget* glWidget)
  {
    setCentralWidget(glWidget);
    glWindow_ = glWidget->windowHandle();
  }

  void paintEvent(QPaintEvent* e)
  {
    // refresh all the views.
    if (glWindow_ && glWindow_->isExposed())
      viewMan_->frame();
    timer_.start();
  }

public slots:

  void setTimerInterval(int value)
  {
    timer_.setInterval(value);
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
  QTimer                              timer_;
  osg::ref_ptr<simVis::ViewManager>   viewMan_;
  osg::ref_ptr<simUtil::StatsHandler> statsHandler_;
  QWindow*                            glWindow_;
};

#endif /* EXAMPLES_QT_MYMAINWINDOW_H */
