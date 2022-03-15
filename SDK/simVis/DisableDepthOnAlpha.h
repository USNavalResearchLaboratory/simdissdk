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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_DISABLEDEPTHONALPHA_H
#define SIMVIS_DISABLEDEPTHONALPHA_H

#include "osg/StateAttribute"
#include "simCore/Common/Common.h"

namespace osg { class StateSet; }

namespace simVis {

/**
* OpenGL 3.3 shader that disables depth writes when alpha values are below a certain threshold.
*
* This is particularly useful for drawing 3D platform models that may or may not have alpha blended
* portions in the model.  Sometimes the render bins won't be set up properly, or even if they are
* there could be some camera angles where depth testing just isn't right for the transparent portions.
* As a result, parts of the ocean or other models show through where they shouldn't, or don't show
* through when they should.  This class provides a shader that disables depth writes when the alpha
* value is below a certain threshold, which should help correctly draw these models.
*/
class SDKVIS_EXPORT DisableDepthOnAlpha
{
public:
  /**
  * Before using this class, a call to installShaderProgram is required.  This method installs
  * the shader program and default uniform variables/defines for controlling the shader.
  */
  static void installShaderProgram(osg::StateSet* intoStateSet);

  /** Static instance-less setting of alpha test on a state set. */
  static void setValues(osg::StateSet* stateset, int value=osg::StateAttribute::ON);
  /** Change the alpha threshold for rejecting pixels.  By default, pixels are rejected for 0.05 alpha and below. */
  static void setAlphaThreshold(osg::StateSet* stateset, float alphaThreshold, int value = osg::StateAttribute::ON);

protected:
  /** Prevent construction/destruction, since there is no state. */
  DisableDepthOnAlpha();
  virtual ~DisableDepthOnAlpha();
};

}

#endif /* SIMVIS_DISABLEDEPTHONALPHA_H */
