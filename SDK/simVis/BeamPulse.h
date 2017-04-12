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
#ifndef SIMVIS_BEAMPULSE_H
#define SIMVIS_BEAMPULSE_H

#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "simCore/Common/Common.h"

namespace osg {
  class Uniform;
  class StateSet;
}

namespace simVis
{

/**
 * Sets the override color via a shader.  To use this, first install the shader
 * program (via installShaderProgram()) on a node at or above the one you want
 * colored.  Then on node(s) to color, instantiate this class with the state set,
 * using setColor() to change the color.
 */
class SDKVIS_EXPORT BeamPulse : public osg::Referenced
{
public:
  /**
   * Declares uniform variables for using and setting the override color
   */
  explicit BeamPulse(osg::StateSet* stateset);

  /** Turns the pulse effect on or off */
  void setEnabled(bool active);
  /**
   * Sets the range between start and stop of a pulse in meters from the origin.  The pattern
   * repeats every (length) meters.  The animation loops (rate) times per real second.
   */
  void setLength(float length);
  /** Sets the rate for a beam to complete the animation pattern, in Hz; inverse of interval. */
  void setRate(float rate);
  /** Changes the stipple pattern (16 bit mask of beam parts to have on) */
  void setStipplePattern(uint16_t pattern);

  /**
   * Before using this class a call to installShaderProgram is required.  This
   * method installs the shader program and default uniform variables for
   * controlling the shader.
   */
  static void installShaderProgram(osg::StateSet* intoStateSet);

protected:
  /// osg::Referenced-derived
  virtual ~BeamPulse();

private:
  /// Set the use to false and set the color to white
  static void setDefaultValues_(osg::StateSet* stateSet);

  /// Hold onto the state set so we can remove the uniforms on destruction
  osg::observer_ptr<osg::StateSet> stateSet_;

  /// Toggle beam pulse animation (bool)
  osg::ref_ptr<osg::Uniform> enabled_;
  /// Length of beam to use for a pulse, in meters (float)
  osg::ref_ptr<osg::Uniform> length_;
  /// Rate for a beam to complete the animation pattern, in hertz (inverse of interval) (float)
  osg::ref_ptr<osg::Uniform> rate_;
  /// Stipple pattern (16 bits) defining the on/off pattern (uint)
  osg::ref_ptr<osg::Uniform> stipplePattern_;
};

} // namespace simVis

#endif // SIMVIS_BEAMPULSE_H
