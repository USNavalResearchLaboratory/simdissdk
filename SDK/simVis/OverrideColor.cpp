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
  const std::string OVERRIDECOLOR_COMBINEMODE_UNIFORM = "simvis_overridecolor_combinemode";
}

OverrideColor::OverrideColor(osg::StateSet* stateset)
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
  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
    stateset->getOrCreateUniform(OVERRIDECOLOR_UNIFORM, osg::Uniform::FLOAT_VEC4)->set(color);
}

void OverrideColor::setCombineMode(CombineMode combineMode)
{
  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
    stateset->getOrCreateUniform(OVERRIDECOLOR_COMBINEMODE_UNIFORM, osg::Uniform::INT)->set(combineMode);
}

void OverrideColor::setDefaultValues_(osg::StateSet* stateSet)
{
  stateSet
    ->getOrCreateUniform(OVERRIDECOLOR_COMBINEMODE_UNIFORM, osg::Uniform::INT)
    ->set(OFF);
  stateSet
    ->getOrCreateUniform(OVERRIDECOLOR_UNIFORM, osg::Uniform::FLOAT_VEC4)
    ->set(simVis::Color::White);
}

}
