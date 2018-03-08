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
#include "osg/Point"
#include "osg/StateSet"
#include "osgEarth/Capabilities"
#include "osgEarth/Registry"
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/PointSize.h"

namespace simVis {

namespace
{
const std::string POINT_SIZE_UNIFORM = "simvis_pointsize";
}

PointSize::PointSize()
{
}

PointSize::~PointSize()
{
}

void PointSize::installShaderProgram(osg::StateSet* intoStateSet)
{
  // Shader side: Install the shader.  FFP: do nothing
  if (osgEarth::Registry::capabilities().supportsGLSL(3.3f))
  {
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);
    simVis::Shaders shaders;
    shaders.load(vp, shaders.pointSizeVertex());
    intoStateSet->setMode(GL_PROGRAM_POINT_SIZE, osg::StateAttribute::OFF);
    intoStateSet->getOrCreateUniform(POINT_SIZE_UNIFORM, osg::Uniform::FLOAT)->set(1.f);
    // Note that large point "rounding" to circles is not supported at this time
  }
}

void PointSize::setValues(osg::StateSet* stateset, float pointSize, int value)
{
  if (stateset == NULL)
    return;

  // Need GLSL 3.3 to use point size shader, else fall back to FFP and hope for compatibility mode
  const bool useShader = osgEarth::Registry::capabilities().supportsGLSL(3.3f);
  if (useShader)
  {
    // GL 3.3 implementation uses a shader
    stateset->setMode(GL_PROGRAM_POINT_SIZE, value);
    osg::Uniform* uni = new osg::Uniform(POINT_SIZE_UNIFORM.c_str(), pointSize);
    stateset->addUniform(uni, value);
  }
  else
  {
    // Fixed function pipeline; controlled by osg::StateAttribute
    stateset->setAttributeAndModes(new osg::Point(pointSize), value);
  }
}

}
