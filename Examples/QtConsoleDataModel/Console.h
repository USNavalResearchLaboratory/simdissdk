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
#ifndef CONSOLEDATAMODELTEST_CONSOLE_H
#define CONSOLEDATAMODELTEST_CONSOLE_H

#include <QWidget>
#include <QTimer>
#include "simNotify/NotifySeverity.h"

namespace simQt { class ConsoleDataModel; }

class Ui_Console;

/** Provides a GUI for showing the console data model and some editing controls */
class Console : public QWidget
{
  Q_OBJECT;

public:
  Console(simQt::ConsoleDataModel& dataModel, QWidget* parent=nullptr);
  virtual ~Console();

private Q_SLOTS:
  /** Generate a number of entries based on GUI state */
  void generateEntries_();
  /** Process a change in the flood rate */
  void setFloodRate_(int hz);
  /** Toggle the timer on or off */
  void toggleFloodTimer_(bool turnOn);

private:
  /** Returns the GUI's selected Notify Severity */
  simNotify::NotifySeverity severity_() const;
  /** Populates the contents of the minimum severity combo box */
  void populateMinSeverity_();

  /** Responds to threaded-flood button, creating three threads and flooding notify. */
  void threeThreadFlood_();
  /** Single function called from thread in order to flood notifications from threads. */
  void floodOneChannel_(simNotify::NotifySeverity severity, const QString& fmt);

  Ui_Console* ui_;
  QTimer floodTimer_;
};

#endif /* CONSOLEDATAMODELTEST_CONSOLE_H */
