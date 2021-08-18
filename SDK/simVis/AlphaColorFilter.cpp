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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "OpenThreads/Atomic"
#include "osg/Program"
#include "osgEarth/VirtualProgram"
#include "osgEarth/Registry"
#include "osgEarth/Capabilities"
#include "simVis/Shaders.h"
#include "simVis/AlphaColorFilter.h"

using namespace osgEarth;

namespace
{
  static OpenThreads::Atomic s_uniformNameGen;
}

//---------------------------------------------------------------------------

#define FUNCTION_PREFIX "simvis_osgearth_alphaColorFilter_"
#define UNIFORM_PREFIX  "simvis_osgearth_u_alpha_"

// This allows for serialization, inclusion in .earth files
OSGEARTH_REGISTER_COLORFILTER(alpha, simVis::AlphaColorFilter);

namespace simVis
{

bool AlphaColorFilter::isSupported()
{
  return osgEarth::Registry::capabilities().supportsGLSL(140u);
}

AlphaColorFilter::AlphaColorFilter()
{
  init_();
}

AlphaColorFilter::AlphaColorFilter(const osgEarth::Config& conf)
{
  init_();
  osg::Vec3f val;
  val[0] = conf.value("clear", 0.0); // clear
  val[1] = conf.value("opaque", 1.0); // opaque
  val[2] = conf.value("enable", 1.0); // enable
  // make sure clear < opaque
  if (val[0] >= val[1])
  {
    // reset to default values
    val[0] = 0.0;
    val[1] = 1.0;
  }
  alpha_->set(val);
}

void AlphaColorFilter::setAlphaOffset(const osg::Vec2f& clearOpaqueValues)
{
  osg::Vec3f tempValues;
  tempValues[2] = (getEnabled() ? 1.0f : 0.0f);
  // make sure clear < opaque
  if (clearOpaqueValues[0] < clearOpaqueValues[1])
  {
    tempValues[0] = clearOpaqueValues[0];
    tempValues[1] = clearOpaqueValues[1];
  }
  else
  {
    // reset to default values
    tempValues[0] = 0.0f;
    tempValues[1] = 1.0f;
  }
  alpha_->set(tempValues);
}

osg::Vec2f AlphaColorFilter::getAlphaOffset() const
{
  osg::Vec3f value;
  alpha_->get(value);
  return osg::Vec2f(value[0], value[1]);
}

void AlphaColorFilter::setEnabled(bool enabled)
{
  osg::Vec2f value = getAlphaOffset();
  alpha_->set(osg::Vec3f(value[0], value[1], (enabled ? 1.0f : 0.0f)));
}

bool AlphaColorFilter::getEnabled() const
{
  osg::Vec3f value;
  alpha_->get(value);
  return (value[2] > 0.5f);
}

std::string AlphaColorFilter::getEntryPointFunctionName() const
{
  return osgEarth::Stringify() << FUNCTION_PREFIX << instanceId_;
}

void AlphaColorFilter::install(osg::StateSet* stateSet) const
{
  // safe: will not add twice.
  stateSet->addUniform(alpha_.get());

  osgEarth::VirtualProgram* vp = dynamic_cast<osgEarth::VirtualProgram*>(stateSet->getAttribute(VirtualProgram::SA_TYPE));
  if (vp)
  {
    // build the local shader (unique per instance). We will
    // use a template with search and replace for this one.
    std::string entryPoint = osgEarth::Stringify() << FUNCTION_PREFIX << instanceId_;
    simVis::Shaders package;
    package.replace("$UNIFORM_NAME", alpha_->getName());
    package.replace("$ENTRY_POINT", entryPoint);
    std::string code = osgEarth::ShaderLoader::load(package.alphaColorFilterFragment(), package);
    osgEarth::replaceIn(code, "$UNIFORM_NAME", alpha_->getName());
    osgEarth::replaceIn(code, "$ENTRY_POINT", entryPoint);

    osg::Shader* main = new osg::Shader(osg::Shader::FRAGMENT, code);
    vp->setShader(entryPoint, main);
  }
}

Config AlphaColorFilter::getConfig() const
{
  osg::Vec3f val;
  alpha_->get(val);
  Config conf("alpha");
  conf.add("clear", val[0]); // clear
  conf.add("opaque", val[1]); // opaque
  conf.add("enable", val[2]); // enable
  return conf;
}

void AlphaColorFilter::init_()
{
  // Generate a unique name for this filter's uniform. This is necessary
  // so that each layer can have a unique uniform and entry point.
  instanceId_ = (++s_uniformNameGen) - 1;
  alpha_ = new osg::Uniform(osg::Uniform::FLOAT_VEC3, (osgEarth::Stringify() << UNIFORM_PREFIX << instanceId_));
  alpha_->set(osg::Vec3f(0.0f, 1.0f, 1.0f));
}

}
