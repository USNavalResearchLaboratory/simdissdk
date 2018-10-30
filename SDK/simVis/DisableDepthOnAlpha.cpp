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
#include "osg/StateSet"
#include "osgEarth/Capabilities"
#include "osgEarth/Registry"
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/DisableDepthOnAlpha.h"

namespace simVis {

namespace
{
const std::string USE_DISABLE_DEPTH_DEFINE = "SV_USE_DISABLE_DEPTH_ON_ALPHA";
}

DisableDepthOnAlpha::DisableDepthOnAlpha()
{
}

DisableDepthOnAlpha::~DisableDepthOnAlpha()
{
}

void DisableDepthOnAlpha::installShaderProgram(osg::StateSet* intoStateSet)
{
  // Shader side: Install the shader.  FFP: do nothing
  if (intoStateSet && osgEarth::Registry::capabilities().supportsGLSL(3.3f))
  {
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);
    simVis::Shaders shaders;
    shaders.load(vp, shaders.disableDepthOnAlphaFragment());
    intoStateSet->setDefine(USE_DISABLE_DEPTH_DEFINE, osg::StateAttribute::OFF);
  }
}

void DisableDepthOnAlpha::setValues(osg::StateSet* stateset, int value)
{
  if (stateset == NULL)
    return;

  int over = value & osg::StateAttribute::OVERRIDE;
  int prot = value & osg::StateAttribute::PROTECTED;

  int negValue = (value & osg::StateAttribute::ON)?
      osg::StateAttribute::OFF | over | prot :
      osg::StateAttribute::ON | over | prot;

  stateset->setDefine(USE_DISABLE_DEPTH_DEFINE, negValue);
}

}
