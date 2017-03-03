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
#ifndef SIMVIS_OVERRIDE_COLOR_H
#define SIMVIS_OVERRIDE_COLOR_H

#include "osgEarth/VirtualProgram"
#include "simCore/Common/Common.h"
#include "simVis/Types.h"

namespace simVis
{

/**
 * Sets the override color via a shader.  To use this, first install the shader
 * program (via installShaderProgram()) on a node at or above the one you want
 * colored.  Then on node(s) to color, instantiate this class with the state set,
 * using setColor() to change the color.
 */
class SDKVIS_EXPORT OverrideColor : public osg::Referenced
{
public:
  /**
   * Declares uniform variables for using and setting the override color
   */
  explicit OverrideColor(osg::StateSet* stateset);

  /**
   * Sets the override color via uniform variables.
   */
  void setColor(const simVis::Color& color);

  /**
   * Before using this class a call to installShaderProgram is required.  This
   * method installs the shader program and default uniform variables for
   * controlling the shader.
   */
  static void installShaderProgram(osg::StateSet* intoStateSet);

#ifdef USE_DEPRECATED_SIMDISSDK_API
  /**
   * Gets the override color.
   * @deprecated Color may not be stored in future versions of SIMDIS SDK.  If still
   *   required in the future, this may be replaced with a more expensive call into
   *   the shader uniform value, returning a copy of the color instead of a reference.
   */
  const simVis::Color& getColor() const;
#endif

protected:
  /// osg::Referenced-derived
  virtual ~OverrideColor();

private:
  /// Set the use to false and set the color to white
  static void setDefaultValues_(osg::StateSet* stateSet);

  osg::observer_ptr<osg::StateSet> stateset_;
  bool active_;

#ifdef USE_DEPRECATED_SIMDISSDK_API
  simVis::Color color_;
#endif
};

} // namespace simVis

#endif // SIMVIS_OVERRIDE_COLOR_H

