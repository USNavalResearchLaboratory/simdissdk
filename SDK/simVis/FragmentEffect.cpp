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
#include "osg/StateSet"
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/FragmentEffect.h"

namespace simVis
{

/** Name of the uniform to edit when changing the fragment effect */
static const std::string FRAGEFFECT_UNIFORM = "svfe_effect";

void FragmentEffect::set(osg::StateSet& stateSet, simData::FragmentEffect effect)
{
  stateSet.getOrCreateUniform(FRAGEFFECT_UNIFORM, osg::Uniform::INT)->set(effect);
}

void FragmentEffect::installShaderProgram(osg::StateSet& stateSet)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(&stateSet);
  simVis::Shaders shaders;
  shaders.load(vp, shaders.fragmentEffect());

  // Set a default value of off
  FragmentEffect::set(stateSet, simData::FE_NONE);
}

}
