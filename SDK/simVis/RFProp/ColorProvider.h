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
#ifndef SIMVIS_RFPROP_COLOR_PROVIDER_H
#define SIMVIS_RFPROP_COLOR_PROVIDER_H

#include "osg/Referenced"
#include "osg/StateSet"
#include "simCore/Common/Common.h"

namespace simRF
{

/** Name to use for the vertex shader for lossToColor() method */
static const std::string LOSS_TO_COLOR_VERTEX = "lossToColor_vert";
/** Name to use for the fragment shader for lossToColor() method */
static const std::string LOSS_TO_COLOR_FRAGMENT = "lossToColor_frag";

/** ColorProvider supplies a color based on a given value */
class /*SDKVIS_EXPORT*/ ColorProvider : public osg::Referenced
{
public:
  /// color mode
  enum ColorMode
  {
    COLORMODE_BELOW,           ///< Only colors below or at the threshold will be returned
    COLORMODE_ABOVE,           ///< Only colors above the threshold will be returned
    COLORMODE_ABOVE_AND_BELOW, ///< Colors above and below or at the threshold will be returned
    COLORMODE_GRADIENT         ///< Show data based on a gradient
  };

  /** Gets the display color mode */
  virtual ColorMode getMode() const = 0;

  /**
   * Installs this color provider from the given state set.
   * You are expected to grab the VirtualProgram on this state set and add two named shaders with the same source that define a single function with the definition:
   *   vec4 lossToColor(in float loss);
   * This function should take an RF loss value and map it to a color.
   *
   * For example:
   * osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
   * vp->setShader("lossToColor_vert", new osg::Shader(osg::Shader::VERTEX, myShaderSource));
   * vp->setShader("lossToColor_frag", new osg::Shader(osg::Shader::FRAGMENT, myShaderSource));
   *
   * You should also attach any uniforms you might need here as well
   */
  virtual void install(osg::StateSet* stateset) = 0;

  /**
   * Uninstall this color provider from the given state set.
   * You need to grab the VirtualProgram from this state set and remove any uniforms you installed previously.  It is also a good idea to remove the two functions you defined as well.
   */
  virtual void uninstall(osg::StateSet* stateset) = 0;

protected:
  /// osg::Referenced-derived
  virtual ~ColorProvider() {}
};
}

#endif /* SIMVIS_RFPROP_COLOR_PROVIDER_H */
