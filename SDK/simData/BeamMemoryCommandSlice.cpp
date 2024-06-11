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
#include <limits>
#include "simData/DataStore.h"
#include "simData/MemoryDataSlice.h"

namespace simData
{

void BeamMemoryCommandSlice::update(DataStore *ds, ObjectId id, double time, DataStore::CommitResult& results)
{
  clearChanged();
  if (updates_.empty())
  {
    reset_();
    return;
  }

  // process all command updates in one prefs transaction
  DataStore::Transaction t;
  BeamPrefs* prefs = ds->mutable_beamPrefs(id, &t, &results);
  if (!prefs)
    return;

  // if requested time is before the beginning
  const BeamCommand *first = updates_.front();
  if (time < first->time())
  {
    if (current())
    {
      // commands have been executed - beam may no longer be in default state, so we need to reset beam to default
      prefs->clear_targetid();
      prefs->mutable_commonprefs()->set_datadraw(false);
      t.complete(&prefs);
    }
    reset_();
    return;
  }

  const BeamCommand* lastBeamCommand = current();
  if ((!lastBeamCommand || time >= lastBeamCommand->time()) && (earliestInsert_ > lastUpdateTime_))
  {
    // time moved forward: execute all commands from lastUpdateTime_ to new current time
    hasChanged_ = advance_(prefs, lastUpdateTime_, time);

    // Check for repeated scalars in the command, forcing complete replacement instead of add-value
    conditionalClearRepeatedFields_(prefs, &commandPrefsCache_);

    // apply the current command state at every update; commands override prefs settings
    prefs->MergeFrom(commandPrefsCache_);
    t.complete(&prefs);
  }
  else
  {
    // time moved backwards: reset and execute all commands from start to new current time
    // reset lastUpdateTime_
    reset_();

    // reset important prefs to default; we will commit these changes regardless of commands
    prefs->clear_targetid();
    prefs->mutable_commonprefs()->set_datadraw(false);

    // advance time forward, execute all commands from 0.0 (use -1.0 since we need a time before 0.0) to new current time
    advance_(prefs, -1.0, time);
    conditionalClearRepeatedFields_(prefs, &commandPrefsCache_);

    hasChanged_ = true;

    // apply the current command state at every update; commands override prefs settings
    prefs->MergeFrom(commandPrefsCache_);
    t.complete(&prefs);
  }

  // reset to no inserted commands
  earliestInsert_ = std::numeric_limits<double>::max();
}


} // End of namespace simData

