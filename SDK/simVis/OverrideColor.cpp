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
#include <cassert>
#include "osgEarth/Registry"
#include "osgEarth/Capabilities"
#include "simVis/Shaders.h"
#include "simVis/OverrideColor.h"

#define LC "simVis::OverrideColor "

namespace simVis
{

const std::string OverrideColor::OVERRIDECOLOR_UNIFORM = "simvis_overridecolor_color";

OverrideColor::OverrideColor(osg::StateSet* stateset)
  :
#ifdef USE_DEPRECATED_SIMDISSDK_API
    color_(simVis::Color::White),
#endif
    shaderCreated_(false)
{
  supported_ = osgEarth::Registry::capabilities().supportsGLSL(110u);
  // Save the stateset if we can run this shader
  if (supported_)
    stateset_ = stateset;
}

OverrideColor::~OverrideColor()
{
  if (supported_ && shaderCreated_)
  {
    osg::ref_ptr<osg::StateSet> stateset;
    if (stateset_.lock(stateset))
    {
      osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::get(stateset);
      if (vp)
      {
        simVis::Shaders shaders;
        shaders.unload(vp, shaders.overrideColorFragment());
      }
      // Remove the uniform variable, no longer used
      stateset->removeUniform(OVERRIDECOLOR_UNIFORM);
    }
  }
}

void OverrideColor::createShader_()
{
  // No need to recreate if already done
  if (shaderCreated_)
    return;

  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
  {
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
    simVis::Shaders shaders;
    shaders.load(vp, shaders.overrideColorFragment());
  }
  shaderCreated_ = true;
}

void OverrideColor::setColor(const simVis::Color& color)
{
  // Exit early if not supported
  if (!supported_)
    return;

  // Exit early if setColor() would have no impact and the shader hasn't been created
  if (color == simVis::Color::White && !shaderCreated_)
    return;

#ifdef USE_DEPRECATED_SIMDISSDK_API
  color_ = color;
#endif

  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
  {
    if (!shaderCreated_)
      createShader_();
    // Assertion failure means the shader creation failed for unknown reason
    assert(shaderCreated_);

    // Pass the color to the shader
    stateset
      ->getOrCreateUniform(OVERRIDECOLOR_UNIFORM, osg::Uniform::FLOAT_VEC4)
      ->set(color);
  }
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
const simVis::Color& OverrideColor::getColor() const
{
  return color_;
}
#endif

}
