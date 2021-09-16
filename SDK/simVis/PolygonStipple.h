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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_POLYGONSTIPPLE_H
#define SIMVIS_POLYGONSTIPPLE_H

#include "simCore/Common/Common.h"

namespace osg { class StateSet; }

namespace simVis {

/**
 * OpenGL 3.3 shader implementation of PolygonStipple.
 *
 * Polygon stipple is not supported in the GL Core profile, so this is a simplified implementation of
 * a polygon stipple shader that matches the behavior in the fixed function pipeline (FFP) version of
 * SIMDIS.  There are 9 unique patterns that this class can represent.  Each pattern alternates rows
 * (even and odd rows have different patterns), and each row has a uniform bit mask across the entire
 * row.  The patterns are, by index:
 *
 *  +----------------------------+
 *  | Index | Even Row | Odd Row |
 *  |   0   |  0x4444  |  0x9999 |
 *  |   1   |  0x4444  |  0x6666 |
 *  |   2   |  0x4444  |  0x3333 |
 *  |   3   |  0xAAAA  |  0x9999 |
 *  |   4   |  0xAAAA  |  0x6666 |
 *  |   5   |  0xAAAA  |  0x3333 |
 *  |   6   |  0xDDDD  |  0x9999 |
 *  |   7   |  0xDDDD  |  0x6666 |
 *  |   8   |  0xDDDD  |  0x3333 |
 *  +----------------------------+
 */
class SDKVIS_EXPORT PolygonStipple
{
public:
  /**
   * Before using this class, a call to installShaderProgram is required.  This method installs
   * the shader program and default uniform variables/defines for controlling the shader.
   */
  static void installShaderProgram(osg::StateSet* intoStateSet);

  /** Static instance-less setting of polygon stipple on a state set. */
  static void setValues(osg::StateSet* stateset, bool enabled, unsigned int patternIndex);

protected:
  /** Prevent construction/destruction, since there is no state. */
  PolygonStipple();
  virtual ~PolygonStipple();

private:
  /** Helper method to choose the right pattern array for the fixed function pipeline (FFP) version */
  static void setFfpStipplePattern_(osg::StateSet* stateSet, unsigned int patternIndex);
};

}

#endif /* SIMVIS_POLYGONSTIPPLE_H */
