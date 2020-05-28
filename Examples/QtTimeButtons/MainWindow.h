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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef EXAMPLES_TIME_BUTTONS_MAIN_WINDOW_H
#define EXAMPLES_TIME_BUTTONS_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "simData/DataStore.h"
#include "simVis/ViewManager.h"

namespace simCore { class ClockImpl; }
namespace simData { class MemoryDataStore; }
namespace simUtil { class PlatformSimulator; }

class MainWindow : public QMainWindow
{
  Q_OBJECT;
public:
  MainWindow();

private slots:
  void notifyFrameUpdate_();

private:
  simData::ObjectId addPlatform_(simData::DataStore &dataStore);
  void setupSimulatedPlatform_();

  /// Populate 'dataStore' with simulated data (provided by 'sim'), through 'endTimeS', at the given 'dataRateHz'
  void populateDataStore_(simData::DataStore &dataStore, simUtil::PlatformSimulator &sim, double endTimeS, double dataRateHz);

  QTimer updateTimer_;
  osg::ref_ptr<simVis::ViewManager> viewMan_;
  simData::MemoryDataStore *dataStore_;
  simCore::ClockImpl *clock_;
};

#endif

