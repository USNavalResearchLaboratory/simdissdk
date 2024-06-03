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
#include <QStringList>
#include "simQt/Settings.h"
#include "simQt/StartupLayoutManager.h"

namespace simQt
{

/** Settings entry for startup tasks list */
static const QString STARTUP_TASKS = "Startup Tasks";
/** Meta data for the Settings entry */
static const simQt::Settings::MetaData STARTUP_TASKS_METADATA = simQt::Settings::MetaData::makeString(
  QVariant(),
  "Names of tasks to execute on start-up.",
  simQt::Settings::PRIVATE
);


StartupLayoutManager::StartupLayoutManager()
{
}

StartupLayoutManager::~StartupLayoutManager()
{
}

int StartupLayoutManager::registerTask(const QString& name, StartupLayoutTaskPtr task)
{
  // Don't overwrite existing tasks
  if (tasks_.contains(name))
    return 1;
  tasks_[name] = task;
  return 0;
}

int StartupLayoutManager::unregisterTask(const QString& name)
{
  auto it = tasks_.find(name);
  if (it == tasks_.end())
    return 1;

  tasks_.erase(it);
  return 0;
}

void StartupLayoutManager::executeTasks(simQt::Settings& fromSettings)
{
  // Retrieve the list of tasks that we should execute, as stated by registry
  QStringList tasksToExecute = fromSettings.value(STARTUP_TASKS, STARTUP_TASKS_METADATA).toStringList();
  for (auto it = tasksToExecute.begin(); it != tasksToExecute.end(); ++it)
  {
    // Get the corresponding task by name from our registered tasks
    StartupLayoutTaskPtr task = tasks_.value(*it);
    // ... and execute it.
    if (task != nullptr)
      task->execute();
  }
}

void StartupLayoutManager::saveToSettings(simQt::Settings& toSettings)
{
  // Gather a list of tasks to save
  QStringList tasksToSave;
  auto keys = tasks_.keys();
  for (auto it = keys.begin(); it != keys.end(); ++it)
  {
    // Task itself knows whether it ought to be saved
    StartupLayoutTaskPtr task = tasks_.value(*it);
    if (task != nullptr && task->shouldExecuteOnNextStartup())
      tasksToSave.push_back(*it);
  }

  // Save to the Settings
  toSettings.setValue(STARTUP_TASKS, tasksToSave, STARTUP_TASKS_METADATA);
}

}
