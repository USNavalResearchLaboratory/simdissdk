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
#ifndef SIMQT_RESOURCE_INITIALIZER_H
#define SIMQT_RESOURCE_INITIALIZER_H

#include "simCore/Common/Export.h"

namespace simQt
{

/** Helper class to initialize icons for your application if you are
 * using EntityTreeComposite or other .ui files that use the simQt .qrc file.
 * Because simQt compiles to a standalone library, the macro
 * Q_INIT_RESOURCE() is needed in order to initialize the .qrc contents.
 * This class does just that.
 */
class SDKQT_EXPORT ResourceInitializer
{
public:
  /// Might need to be called after QApplication is constructed; automatically protected against multiple calls
  static void initialize();

  /**
   * Registers all meta types from simQt; called from initialize() and not protected against multiple calls.
   * This is a subset of initialize(). This is useful for cases where a dynamic library overwrites the
   * metadata registrations, then is unloaded, leading to stale memory in the Qt metatype registry.
   */
  static void registerMetaTypes();
};

}

#endif /* SIMQT_RESOURCE_INITIALIZER_H */

