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
#include "osg/AlphaFunc"
#include "osg/StateSet"
#include "osgEarth/Capabilities"
#include "osgEarth/Registry"
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/AlphaTest.h"

namespace simVis {

namespace
{
const std::string USE_ALPHA_TEST_DEFINE = "SIMVIS_USE_ALPHA_TEST";
const std::string ALPHA_THRESHOLD_UNIFORM = "simvis_alpha_threshold";
}

AlphaTest::AlphaTest()
{
}

AlphaTest::~AlphaTest()
{
}

void AlphaTest::installShaderProgram(osg::StateSet* intoStateSet)
{
  // Shader side: Install the shader.  FFP: do nothing
  if (osgEarth::Registry::capabilities().supportsGLSL(3.3f))
  {
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);
    simVis::Shaders shaders;
    shaders.load(vp, shaders.alphaTestFragment());
    intoStateSet->setDefine(USE_ALPHA_TEST_DEFINE, osg::StateAttribute::OFF);
    intoStateSet->getOrCreateUniform(ALPHA_THRESHOLD_UNIFORM, osg::Uniform::FLOAT)->set(0.5f);
  }
}

void AlphaTest::setValues(osg::StateSet* stateset, float threshold, int value)
{
  if (stateset == nullptr)
    return;

  // Need GLSL 3.3 to use alpha test shader, else fall back to FFP and hope for compatibility mode
  const bool useShader = osgEarth::Registry::capabilities().supportsGLSL(3.3f);
  if (useShader)
  {
    // GL 3.3 implementation uses a shader
    stateset->setDefine(USE_ALPHA_TEST_DEFINE, value);
    osg::Uniform* uni = new osg::Uniform(ALPHA_THRESHOLD_UNIFORM.c_str(), threshold);
    stateset->addUniform(uni, value);
  }
  else
  {
    // Fixed function pipeline; controlled by osg::StateAttribute
    stateset->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GREATER, threshold), value);
    stateset->setMode(GL_ALPHA_TEST, value);
  }
}

}
