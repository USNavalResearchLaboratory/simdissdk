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
  /// Might need to be called after QApplication is constructed
  static void initialize();
};

}

#endif /* SIMQT_RESOURCE_INITIALIZER_H */

