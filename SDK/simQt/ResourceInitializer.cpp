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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QtCore/qglobal.h>
#include <QKeySequence>
#include <QList>
#include "simQt/ConsoleDataModel.h"
#ifdef HAVE_SIMDATA
#include "simQt/EntityStateFilter.h"
#include "simQt/EntityTreeComposite.h"
#endif
#include "simQt/SettingsModel.h"
#include "simQt/ResourceInitializer.h"

/// Local initialization for the simQt library.  Note that this cannot be in a namespace
void qt_Initialize_simQt()
{
  Q_INIT_RESOURCE(simQtResources);
}

namespace simQt
{

void ResourceInitializer::initialize()
{
  static bool simQtRccInit = false;
  // Try to avoid initializing more than once
  if (!simQtRccInit)
  {
    qt_Initialize_simQt();

    // Register meta types for use in QSettings
    qRegisterMetaTypeStreamOperators<simQt::Settings::MetaData>("simQt::Settings::MetaData");
    qRegisterMetaTypeStreamOperators<QList<QKeySequence> >("QList<QKeySequence>");
#ifdef HAVE_SIMDATA
    qRegisterMetaTypeStreamOperators<simQt::EntityTreeComposite::FilterConfiguration>("simQt::EntityTreeComposite::FilterConfiguration");
#endif

    // Register meta type for thread safety in channels
    qRegisterMetaType<simNotify::NotifySeverity>("simNotify::NotifySeverity");

    // Register meta type for use as an argument in signals/slots
#ifdef HAVE_SIMDATA
    qRegisterMetaType<EntityStateFilter::State>("EntityStateFilter::State");
#endif
  }
  simQtRccInit = true;
  // If other resource files are added, it might be advantageous to
  // add them in here as well.  Alternatively, different functions
  // could be used to initialize different resource sets.
}

}

