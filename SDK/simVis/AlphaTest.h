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
#ifndef SIMVIS_ALPHATEST_H
#define SIMVIS_ALPHATEST_H

#include "osg/StateAttribute"
#include "simCore/Common/Common.h"

namespace osg { class StateSet; }

namespace simVis {

/**
* OpenGL 3.3 shader implementation of AlphaTest.
*
* Alpha Test is not supported in the GL Core profile, so this is a simplified implementation of an
* alpha test shader that matches behavior in the fixed function pipeline (FFP) version of SIMDIS.
*/
class SDKVIS_EXPORT AlphaTest
{
public:
  /**
  * Before using this class, a call to installShaderProgram is required.  This method installs
  * the shader program and default uniform variables/defines for controlling the shader.
  */
  static void installShaderProgram(osg::StateSet* intoStateSet);

  /** Static instance-less setting of alpha test on a state set. */
  static void setValues(osg::StateSet* stateset, float threshold, int value=osg::StateAttribute::ON);

protected:
  /** Prevent construction/destruction, since there is no state. */
  AlphaTest();
  virtual ~AlphaTest();
};

}

#endif /* SIMVIS_ALPHATEST_H */
