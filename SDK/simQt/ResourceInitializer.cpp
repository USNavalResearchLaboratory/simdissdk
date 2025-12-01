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
#include <QtCore/qglobal.h>
#include <QKeySequence>
#include <QList>
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/MagneticVariance.h"
#include "simCore/Calc/VerticalDatum.h"
#include "simCore/Calc/Units.h"
#include "simCore/String/Angle.h"
#include "simCore/Time/Constants.h"

#include "simQt/ConsoleDataModel.h"
#ifdef HAVE_SIMDATA
#include "simQt/EntityStateFilter.h"
#include "simQt/EntityTreeComposite.h"
#endif
#include "simQt/SettingsModel.h"
#include "simQt/ResourceInitializer.h"

// Qt6 meta types need to be declared only once and should only be in a .cpp file
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
Q_DECLARE_METATYPE(simCore::Units);
Q_DECLARE_METATYPE(simCore::TimeFormat);
Q_DECLARE_METATYPE(simCore::CoordinateSystem);
Q_DECLARE_METATYPE(simCore::VerticalDatum);
Q_DECLARE_METATYPE(simCore::MagneticVariance);
Q_DECLARE_METATYPE(simCore::GeodeticFormat);
#endif


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
    ResourceInitializer::registerMetaTypes();
  }
  simQtRccInit = true;
  // If other resource files are added, it might be advantageous to
  // add them in here as well.  Alternatively, different functions
  // could be used to initialize different resource sets.
}

void ResourceInitializer::registerMetaTypes()
{
  qRegisterMetaType<QList<QKeySequence> >("QList<QKeySequence>");
  qRegisterMetaType<simQt::Settings::MetaData>("simQt::Settings::MetaData");
  qRegisterMetaType<simNotify::NotifySeverity>("simNotify::NotifySeverity");

#ifdef HAVE_SIMDATA
  qRegisterMetaType<simQt::EntityTreeComposite::FilterConfiguration>("simQt::EntityTreeComposite::FilterConfiguration");
  qRegisterMetaType<EntityStateFilter::State>("EntityStateFilter::State");
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  // Register meta types for use in QSettings; Qt6 does not require the qRegisterMetaTypeStreamOperators call
  qRegisterMetaTypeStreamOperators<QList<QKeySequence> >("QList<QKeySequence>");
  qRegisterMetaTypeStreamOperators<simQt::Settings::MetaData>("simQt::Settings::MetaData");
#ifdef HAVE_SIMDATA
  qRegisterMetaTypeStreamOperators<simQt::EntityTreeComposite::FilterConfiguration>("simQt::EntityTreeComposite::FilterConfiguration");
#endif
#endif
}

}
