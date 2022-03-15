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
#ifndef SIMQT_STARTUP_LAYOUT_TASK_H
#define SIMQT_STARTUP_LAYOUT_TASK_H

#include <memory>
#include <QObject>
#include "simCore/Common/Export.h"

class QWidget;

namespace simQt {

/**
 * Start-Up task that encapsulates the job of displaying a widget.  This can be used to conveniently
 * bind the display of a QDockWidget or QDialog to start-up, while keeping resource allocation optional.
 * This is crucial for large dialogs like SuperForm that consume lots of resources.
 *
 * The class automatically ties into widget destruction signals, so it will never have an invalid state.
 *
 * You must connect() the executed() signal to your desired slot to execute the showing of the widget.
 * Note that a convenience constructor is provided to automate this process.
 */
class SDKQT_EXPORT StartupLayoutTask : public QObject
{
  Q_OBJECT;
public:
  /** Default constructor. */
  StartupLayoutTask();
  /**
   * Convenience constructor to connect() the executed() signal to the provided receiver and method.
   * @param receiver Receiver object for the executed() signal
   * @param method Receiver method for the executed() signal (e.g. SLOT(showDialog()))
   */
  StartupLayoutTask(QObject* receiver, const char* method);

  virtual ~StartupLayoutTask();

  /** Returns true if the widget is non-nullptr and visible at the time of query */
  virtual bool shouldExecuteOnNextStartup() const;
  /** Emits the executed() signal */
  virtual void execute();

public slots:
  /** Sets a widget to monitor for visibility.  You may set to nullptr when widget goes away. */
  void setWidget(QWidget* widget);
  /** Clears the widget; same behavior as setWidget(nullptr) */
  void clearWidget();

signals:
  /** Emitted when the StartupLayoutManager requests we execute. */
  void executed();

private:
  QWidget* widget_;
};
typedef std::shared_ptr<StartupLayoutTask> StartupLayoutTaskPtr;

}

#endif /* SIMQT_STARTUP_LAYOUT_TASK_H */
