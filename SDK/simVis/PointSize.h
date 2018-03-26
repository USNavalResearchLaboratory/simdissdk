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
#ifndef SIMVIS_POINTSIZE_H
#define SIMVIS_POINTSIZE_H

#include "osg/StateAttribute"
#include "simCore/Common/Common.h"

namespace osg { class StateSet; }

namespace simVis {

/**
* OpenGL 3.3 shader implementation of PointSize.
*
* PointSize is not supported in the GL Core profile, so this is a simplified implementation of an
* point size shader that matches behavior in the fixed function pipeline (FFP) version of SIMDIS.
*/
class SDKVIS_EXPORT PointSize
{
public:
  /**
  * Before using this class, a call to installShaderProgram is required.  This method installs
  * the shader program and default uniform variables/defines for controlling the shader.
  */
  static void installShaderProgram(osg::StateSet* intoStateSet);

  /** Static instance-less setting of point size on a state set. */
  static void setValues(osg::StateSet* stateset, float pointSize, int value = osg::StateAttribute::ON);

protected:
  /** Prevent construction/destruction, since there is no state. */
  PointSize();
  virtual ~PointSize();
};

}

#endif /* SIMVIS_POINTSIZE_H */
