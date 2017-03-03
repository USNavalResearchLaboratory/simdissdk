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

namespace
{
  const std::string OVERRIDECOLOR_UNIFORM = "simvis_overridecolor_color";
  const std::string USE_OVERRIDECOLOR_UNIFORM = "simvis_use_overridecolor";
}

OverrideColor::OverrideColor(osg::StateSet* stateset)
  :
    active_(false),
#ifdef USE_DEPRECATED_SIMDISSDK_API
    color_(simVis::Color::White)
#endif
{
  stateset_ = stateset;
  OverrideColor::setDefaultValues_(stateset_.get());
}

OverrideColor::~OverrideColor()
{
}

void OverrideColor::installShaderProgram(osg::StateSet* intoStateSet)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);
  simVis::Shaders shaders;
  shaders.load(vp, shaders.overrideColorFragment());
  OverrideColor::setDefaultValues_(intoStateSet);
}

void OverrideColor::setColor(const simVis::Color& color)
{
#ifdef USE_DEPRECATED_SIMDISSDK_API
  color_ = color;
#endif

  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
  {
    if (color == simVis::Color::White)
    {
      if (active_)
      {
        stateset
          ->getOrCreateUniform(USE_OVERRIDECOLOR_UNIFORM, osg::Uniform::BOOL)
          ->set(false);
        active_ = false;
      }

      return;
    }

    if (!active_)
    {
      stateset
        ->getOrCreateUniform(USE_OVERRIDECOLOR_UNIFORM, osg::Uniform::BOOL)
        ->set(true);
      active_ = true;
    }

    // Pass the color to the shader
    stateset
      ->getOrCreateUniform(OVERRIDECOLOR_UNIFORM, osg::Uniform::FLOAT_VEC4)
      ->set(color);
  }
}

void OverrideColor::setDefaultValues_(osg::StateSet* stateSet)
{
  stateSet
    ->getOrCreateUniform(USE_OVERRIDECOLOR_UNIFORM, osg::Uniform::BOOL)
    ->set(false);
  stateSet
    ->getOrCreateUniform(OVERRIDECOLOR_UNIFORM, osg::Uniform::FLOAT_VEC4)
    ->set(simVis::Color::White);
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
const simVis::Color& OverrideColor::getColor() const
{
  return color_;
}
#endif

}
