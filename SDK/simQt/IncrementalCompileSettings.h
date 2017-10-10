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
#ifndef SIMQT_INCREMENTALCOMPILESETTINGS_H
#define SIMQT_INCREMENTALCOMPILESETTINGS_H

#include <QObject>
#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "simCore/Common/Export.h"

namespace osgUtil { class IncrementalCompileOperation; }
namespace osgViewer { class ViewerBase; }
namespace simQt {

class Settings;
class BoundBooleanSetting;
class BoundIntegerSetting;

/** Manages settings for incremental compilation in OpenGL using an osgViewer::ViewerBase */
class SDKQT_EXPORT IncrementalCompileSettings : public QObject
{
  Q_OBJECT;
public:
  IncrementalCompileSettings(simQt::Settings& settings, osgViewer::ViewerBase* viewer, QObject* parent=NULL);
  virtual ~IncrementalCompileSettings();

private slots:
  /** Adds or removes the incremental compile operation to the scene */
  void setIncrementalEnabled_(bool enabled);
  /** Wraps osgUtil::IncrementalCompileOperation::setTargetFrameRate() */
  void setTargetRate_(int fps);
  /** Wraps osgUtil::IncrementalCompileOperation::setMinimumTimeAvailableForGLCompileAndDeletePerFrame() */
  void setMinCompileTime_(int microseconds);
  /** Wraps osgUtil::IncrementalCompileOperation::setMaximumNumOfObjectsToCompilePerFrame() */
  void setMaxCompilesPerFrame_(int numObjects);
  /** Wraps osgUtil::IncrementalCompileOperation::setFlushTimeRatio() */
  void setFlushTimeRatio_(int percent);
  /** Wraps osgUtil::IncrementalCompileOperation::setConservativeTimeRatio() */
  void setConservativeTimeRatio_(int percent);

private:
  /** Initializes the ICO and puts it onto the viewer */
  void initialize_();

  osg::observer_ptr<osgViewer::ViewerBase> viewer_;
  osg::ref_ptr<osgUtil::IncrementalCompileOperation> ico_;
  simQt::BoundBooleanSetting* icoEnabled_;
  simQt::BoundIntegerSetting* targetRate_;
  simQt::BoundIntegerSetting* minimumTimeForCompileUS_;
  simQt::BoundIntegerSetting* maxCompilesPerFrame_;
  simQt::BoundIntegerSetting* flushTimeRatio_;
  simQt::BoundIntegerSetting* conservativeTimeRatio_;
};

}

#endif /* SIMQT_INCREMENTALCOMPILESETTINGS_H */
