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
#include <cassert>
#include <set>
#include "osgUtil/IncrementalCompileOperation"
#include "osgViewer/View"
#include "osgViewer/ViewerBase"
#include "simQt/BoundSettings.h"
#include "simQt/IncrementalCompileSettings.h"

namespace simQt {

static const simQt::Settings::MetaData MD_ENABLED(simQt::Settings::MetaData::makeBoolean(true, QObject::tr("Enable incremental compile options"), simQt::Settings::ADVANCED));
static const simQt::Settings::MetaData MD_TGTRATE(simQt::Settings::MetaData::makeInteger(20, QObject::tr("Guides max time to allocate to compile operations"), simQt::Settings::ADVANCED, 1, 240));
static const simQt::Settings::MetaData MD_MINTIME_US(simQt::Settings::MetaData::makeInteger(1000, QObject::tr("Minimum time in microseconds for compile operations"), simQt::Settings::ADVANCED, 1, 1000000));
static const simQt::Settings::MetaData MD_MAXCOMPILES(simQt::Settings::MetaData::makeInteger(15, QObject::tr("Max number of objects to compile per frame"), simQt::Settings::ADVANCED, 1, 999999));
static const simQt::Settings::MetaData MD_FLUSHPCT(simQt::Settings::MetaData::makeInteger(50, QObject::tr("Percent time spent flushing deleted GL objects; higher means more frame drops"), simQt::Settings::ADVANCED, 10, 90));
static const simQt::Settings::MetaData MD_CONSERVATIVEPCT(simQt::Settings::MetaData::makeInteger(50, QObject::tr("Percent time spent flushing deleted and compiling new GL objects; higher means more frame drops"), simQt::Settings::ADVANCED, 10, 90));

IncrementalCompileSettings::IncrementalCompileSettings(simQt::Settings& settings, osgViewer::ViewerBase* viewer, QObject* parent)
  : QObject(parent),
    viewer_(viewer),
    ico_(new osgUtil::IncrementalCompileOperation),
    icoEnabled_(new simQt::BoundBooleanSetting(this, settings, "Incremental Compile/Enabled", MD_ENABLED)),
    targetRate_(new simQt::BoundIntegerSetting(this, settings, "Incremental Compile/Target FPS", MD_TGTRATE)),
    minimumTimeForCompileUS_(new simQt::BoundIntegerSetting(this, settings, "Incremental Compile/Min Frame Time (microseconds)", MD_MINTIME_US)),
    maxCompilesPerFrame_(new simQt::BoundIntegerSetting(this, settings, "Incremental Compile/Max Compiles Per Frame", MD_MAXCOMPILES)),
    flushTimeRatio_(new simQt::BoundIntegerSetting(this, settings, "Incremental Compile/Flush Time Ratio", MD_FLUSHPCT)),
    conservativeTimeRatio_(new simQt::BoundIntegerSetting(this, settings, "Incremental Compile/Conservative Time Ratio", MD_CONSERVATIVEPCT))
{
  // An invalid viewer or one that is not realized will have no graphics contexts, which
  // are required for configuring and enabling the ICO
  assert(viewer_.valid());

  // Add the graphics contexts for the viewer
  if (viewer_.valid())
  {
    osgViewer::ViewerBase::Contexts contexts;
    viewer_->getContexts(contexts);
    // Assertion failure means we have no graphics contexts, which means ICO will do nothing
    assert(!contexts.empty());
    for (osgViewer::ViewerBase::Contexts::const_iterator i = contexts.begin(); i != contexts.end(); ++i)
      ico_->addGraphicsContext(*i);
  }

  connect(icoEnabled_, SIGNAL(valueChanged(bool)), this, SLOT(setIncrementalEnabled_(bool)));
  connect(targetRate_, SIGNAL(valueChanged(int)), this, SLOT(setTargetRate_(int)));
  connect(minimumTimeForCompileUS_, SIGNAL(valueChanged(int)), this, SLOT(setMinCompileTime_(int)));
  connect(maxCompilesPerFrame_, SIGNAL(valueChanged(int)), this, SLOT(setMaxCompilesPerFrame_(int)));
  connect(flushTimeRatio_, SIGNAL(valueChanged(int)), this, SLOT(setFlushTimeRatio_(int)));
  connect(conservativeTimeRatio_, SIGNAL(valueChanged(int)), this, SLOT(setConservativeTimeRatio_(int)));

  // For the next 5 frames (arbitrary), compile all objects
  ico_->compileAllForNextFrame(5);
  initialize_();
}

IncrementalCompileSettings::~IncrementalCompileSettings()
{
  delete conservativeTimeRatio_;
  conservativeTimeRatio_ = NULL;
  delete flushTimeRatio_;
  flushTimeRatio_ = NULL;
  delete maxCompilesPerFrame_;
  maxCompilesPerFrame_ = NULL;
  delete minimumTimeForCompileUS_;
  minimumTimeForCompileUS_ = NULL;
  delete targetRate_;
  targetRate_ = NULL;
  delete icoEnabled_;
  icoEnabled_ = NULL;
}

void IncrementalCompileSettings::initialize_()
{
  setIncrementalEnabled_(icoEnabled_->value());
  setTargetRate_(targetRate_->value());
  setMinCompileTime_(targetRate_->value());
  setMaxCompilesPerFrame_(targetRate_->value());
  setFlushTimeRatio_(flushTimeRatio_->value());
  setConservativeTimeRatio_(conservativeTimeRatio_->value());
}

void IncrementalCompileSettings::setIncrementalEnabled_(bool enabled)
{
  if (!viewer_.valid())
    return;
  viewer_->setIncrementalCompileOperation(enabled ? ico_ : NULL);

  // Iterate through all views and grab their database pagers
  osgViewer::ViewerBase::Views views;
  viewer_->getViews(views);
  std::set<osgDB::DatabasePager*> pagers;
  for (osgViewer::ViewerBase::Views::const_iterator i = views.begin(); i != views.end(); ++i)
  {
    if (*i != NULL && (*i)->getDatabasePager() != NULL)
      pagers.insert((*i)->getDatabasePager());
  }
  // Apply the ICO to each database pager we found (likely only one shared amongst views)
  for (std::set<osgDB::DatabasePager*>::const_iterator i = pagers.begin(); i != pagers.end(); ++i)
  {
    (*i)->setIncrementalCompileOperation(enabled ? ico_ : NULL);
  }
}

void IncrementalCompileSettings::setTargetRate_(int fps)
{
  ico_->setTargetFrameRate(fps);
}

void IncrementalCompileSettings::setMinCompileTime_(int microseconds)
{
  ico_->setMinimumTimeAvailableForGLCompileAndDeletePerFrame(microseconds * 1e6);
}

void IncrementalCompileSettings::setMaxCompilesPerFrame_(int numObjects)
{
  ico_->setMaximumNumOfObjectsToCompilePerFrame(numObjects);
}

void IncrementalCompileSettings::setFlushTimeRatio_(int percent)
{
  ico_->setFlushTimeRatio(percent * 0.01);
}

void IncrementalCompileSettings::setConservativeTimeRatio_(int percent)
{
  ico_->setConservativeTimeRatio(percent * 0.01);
}

}
