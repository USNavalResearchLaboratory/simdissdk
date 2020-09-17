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
#ifndef SIMQT_INCREMENTALCOMPILESETTINGS_H
#define SIMQT_INCREMENTALCOMPILESETTINGS_H

#ifdef USE_DEPRECATED_SIMDISSDK_API

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

/**
 * @deprecated
 *
 * Manages settings for incremental compilation in OpenGL using an osgViewer::ViewerBase.
 *
 * This class is deprecated because incremental compilation in OpenSceneGraph 3.6.5+ causes
 * intermittent problems, especially with text displays.  Incremental Compilation (ICO) has
 * been tracked to cause flickering red squares with incrementally compiled text objects,
 * including osgEarth MGRS/UTM/GARS grids.  It has also been tracked to an intermittent
 * issue where textures are reported as immutable, causing runtime GL errors and possibly
 * display anomalies.  As a result of the instability of ICO, this class is deprecated.
 */
class SDKQT_EXPORT IncrementalCompileSettings : public QObject
{
  Q_OBJECT;
public:
  IncrementalCompileSettings(simQt::Settings& settings, osgViewer::ViewerBase* viewer, QObject* parent=nullptr);
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

#endif /* USE_DEPRECATED_SIMDISSDK_API */

#endif /* SIMQT_INCREMENTALCOMPILESETTINGS_H */
