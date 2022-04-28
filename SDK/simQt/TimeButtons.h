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
#ifndef SIMQT_TIMEBUTTONS_H
#define SIMQT_TIMEBUTTONS_H

#include <QAction>
#include <QIcon>
#include <QWidget>
#include "simCore/Common/Export.h"
#include "simCore/Time/Clock.h"

class Ui_TimeButtons;

namespace simQt {

/** Wrapper around the functions to operate with buttons */
class SDKQT_EXPORT ButtonActions : public QObject
{
  Q_OBJECT;
public:
  /** Constructor */
  ButtonActions(QWidget *parent = nullptr);
  virtual ~ButtonActions();

  /** Returns step decrease action */
  QAction* stepDecreaseAction() const;
  /** Returns step back action */
  QAction* stepBackAction() const;
  /** Returns play reverse action */
  QAction* playReverseAction() const;
  /** Returns stop action */
  QAction* stopAction() const;
  /** Returns play action */
  QAction* playAction() const;
  /** Returns step forward action */
  QAction* stepForwardAction() const;
  /** Returns step increase action */
  QAction* stepIncreaseAction() const;
  /** Returns real-time action */
  QAction* realTimeAction() const;
  /** Returns toggle loop action */
  QAction* toggleLoopAction() const;
  /** Returns Start/Stop action */
  QAction* startStopAction() const;
  /** Returns all actions */
  QList<QAction*> actions() const;
  /** Sets the clock manager */
  void setClockManager(simCore::Clock *clock);

signals:
  /** Echo outs the time control enable state changed*/
  void timeControlEnableStateChanged(bool isEnabled);

private slots:
  void clockStop_();
  void clockPlay_();
  void clockStartStop_();
  void clockPlayBackwards_();
  void clockStepBack_();
  void clockStepForward_();
  void clockStepDecrease_();
  void clockStepIncrease_();
  void clockRealTime_(bool pressed);
  void clockToggleLoop_(bool pressed);

private:
  void updateEnabledState_();
  void updateCheckedState_();

  simCore::Clock *clock_; ///< shared ref to the clock manager
  class TimeModeObserver;
  simCore::Clock::ModeChangeObserverPtr observer_;
  QAction* stepDecrease_;
  QAction* stepBack_;
  QAction* playReverse_;
  QAction* stop_;
  QAction* play_;
  QAction* startStop_;
  QAction* stepForward_;
  QAction* stepIncrease_;
  QAction* realTime_;
  QAction* toggleLoop_;
  QIcon stopIcon_;
  QIcon playIcon_;
};

/**
 * Wrapper around the Ui_TimeButtons widget, tying together the ButtonActions to actual buttons
 *
 * Provides the connection to the button press functionality.  Note that toggle loop is not part
 * of the default Ui_TimeButtons layout, and is therefore not linked to any widget (because there
 * is no existing widget)
 */
class SDKQT_EXPORT TimeButtons : public QWidget
{
  Q_OBJECT;

public:
  /** Constructor */
  TimeButtons(QWidget *parent = nullptr);
  virtual ~TimeButtons();

  /** Binds the button to the action */
  void bindToActions(ButtonActions* actions);

  /** Resizes all the buttons to the given size; default size is 32 pixels */
  void resizeButtons(int size);

private:
  Ui_TimeButtons *ui_; ///< wrapper on the QT Designer widget
};

}

#endif /* SIMQT_TIMEBUTTONS_H */
