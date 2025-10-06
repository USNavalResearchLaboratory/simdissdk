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
#ifndef QTTOUCH_MAINWINDOW_H
#define QTTOUCH_MAINWINDOW_H

#include <vector>
#include <QMainWindow>
#include <QTimer>
#include "simVis/ViewManager.h"
#include "osg/ref_ptr"
#include "osg/observer_ptr"

namespace osgGA { class GUIEventAdapter; }
class QLineEdit;
class QStandardItemModel;

/**
 * A simple MainWindow derivative that shows one way to embed a
 * simVis::ViewManager configuration in a Qt UI.
 */
class MainWindow : public QMainWindow
{
  Q_OBJECT;
public:
  explicit MainWindow(QWidget* parent=nullptr);

  /** Returns the view manager configured in the main window */
  simVis::ViewManager* getViewManager() const;
  /** Displays content of an OSG event */
  void processOsgEvent(const osgGA::GUIEventAdapter& ea);

private:
  /** Creates the dock widget for the touch device display */
  void addTouchDevicesDock_();
  /** Creates the dock widget for most recent touch/mouse event */
  void addMostRecentDock_();

  QTimer timer_;
  osg::ref_ptr<simVis::ViewManager> viewMan_;

  QLineEdit* recentX_;
  QLineEdit* recentY_;
  QLineEdit* recentEvent_;
  QLineEdit* recentTouchPts_;
  QStandardItemModel* touchValues_;
};

#endif /* QTTOUCH_MAINWINDOW_H */
