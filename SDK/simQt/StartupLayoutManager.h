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
#ifndef SIMQT_STARTUP_LAYOUT_MANAGER_H
#define SIMQT_STARTUP_LAYOUT_MANAGER_H

#include <QMap>
#include <QString>
#include "simCore/Common/Export.h"
#include "simQt/StartupLayoutTask.h"

namespace simQt {

class Settings;

/**
 * Defines a manager that contains named start-up tasks.  On application shutdown, the
 * tasks are queried to determine whether they should be included in persistent storage
 * (simQt::Settings).  If included, they will be executed on the next application start-up.
 */
class SDKQT_EXPORT StartupLayoutManager
{
public:
  StartupLayoutManager();
  virtual ~StartupLayoutManager();

  /** Registers a task to execute potentially on start-up */
  int registerTask(const QString& name, StartupLayoutTaskPtr task);

  /** Unregisters a task */
  int unregisterTask(const QString& name);

  /** Executes all tasks that are marked in the Settings for starting on initialization */
  void executeTasks(simQt::Settings& fromSettings);
  /** Saves a set of tasks to persistent storage that indicate they should execute on next start-up */
  void saveToSettings(simQt::Settings& toSettings);

private:
  /** Map of task name to task pointer */
  QMap<QString, StartupLayoutTaskPtr> tasks_;
};

}

#endif /* SIMQT_STARTUP_LAYOUT_MANAGER_H */
