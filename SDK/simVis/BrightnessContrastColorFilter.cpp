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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "OpenThreads/Atomic"
#include "osgEarth/VirtualProgram"
#include "simVis/BrightnessContrastColorFilter.h"

namespace
{
  static OpenThreads::Atomic s_uniformNameGen;

  static const char* s_localShaderSource =
    "#version 140\n"
    "uniform vec2 __UNIFORM_NAME__;\n"

    "void __ENTRY_POINT__(inout vec4 color)\n"
    "{\n"
    "    color.rgb = ((color.rgb - 0.5) * __UNIFORM_NAME__.y + 0.5) * __UNIFORM_NAME__.x; \n"
    "    color.rgb = clamp(color.rgb, 0.0, 1.0); \n"
    "}\n";
}

//---------------------------------------------------------------------------

#define BC_FUNCTION_PREFIX "simvis_osgearth_bcColorFilter_"
#define BC_UNIFORM_PREFIX  "simvis_osgearth_u_bc_"

// This allows for serialization, inclusion in .earth files
OSGEARTH_REGISTER_COLORFILTER(brightness_contrast, simVis::BrightnessContrastColorFilter);

namespace simVis {

BrightnessContrastColorFilter::BrightnessContrastColorFilter()
{
  init_();
}

BrightnessContrastColorFilter::BrightnessContrastColorFilter(const osgEarth::Config& conf)
{
  init_();

  osg::Vec2f val;
  val[0] = conf.value("b", 1.0);
  val[1] = conf.value("c", 1.0);
  setBrightnessContrast(val);
}

void BrightnessContrastColorFilter::init_()
{
  // Generate a unique name for this filter's uniform. This is necessary
  // so that each layer can have a unique uniform and entry point.
  instanceId_ = (++s_uniformNameGen) - 1;
  uniform_ = new osg::Uniform(osg::Uniform::FLOAT_VEC2, (osgEarth::Stringify() << BC_UNIFORM_PREFIX << instanceId_));
  uniform_->set(osg::Vec2f(1.0f, 1.0f));
}

void BrightnessContrastColorFilter::setBrightnessContrast(const osg::Vec2f& value)
{
  uniform_->set(value);
}

osg::Vec2f BrightnessContrastColorFilter::getBrightnessContrast(void) const
{
  osg::Vec2f value;
  uniform_->get(value);
  return (value);
}

std::string BrightnessContrastColorFilter::getEntryPointFunctionName(void) const
{
  return (osgEarth::Stringify() << BC_FUNCTION_PREFIX << instanceId_);
}

void BrightnessContrastColorFilter::install(osg::StateSet* stateSet) const
{
  // safe: will not add twice.
  stateSet->addUniform(uniform_.get());

  osgEarth::VirtualProgram* vp = dynamic_cast<osgEarth::VirtualProgram*>(stateSet->getAttribute(osgEarth::VirtualProgram::SA_TYPE));
  if (vp)
  {
    // build the local shader (unique per instance). We will
    // use a template with search and replace for this one.
    std::string entryPoint = osgEarth::Stringify() << BC_FUNCTION_PREFIX << instanceId_;
    std::string code = s_localShaderSource;
    osgEarth::replaceIn(code, "__UNIFORM_NAME__", uniform_->getName());
    osgEarth::replaceIn(code, "__ENTRY_POINT__", entryPoint);

    osg::Shader* main = new osg::Shader(osg::Shader::FRAGMENT, code);
    //main->setName(entryPoint);
    vp->setShader(entryPoint, main);
  }
}

osgEarth::Config BrightnessContrastColorFilter::getConfig() const
{
  osg::Vec2f val = getBrightnessContrast();
  osgEarth::Config conf("brightness_contrast");
  conf.add("b", val[0]);
  conf.add("c", val[1]);
  return conf;
}

}
