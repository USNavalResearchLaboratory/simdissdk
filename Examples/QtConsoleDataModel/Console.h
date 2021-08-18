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

private slots:
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

  Ui_Console* ui_;
  QTimer floodTimer_;
};

#endif /* CONSOLEDATAMODELTEST_CONSOLE_H */
