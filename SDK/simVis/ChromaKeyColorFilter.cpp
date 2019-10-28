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
#include "OpenThreads/Atomic"
#include "osgEarth/VirtualProgram"
#include "simVis/ChromaKeyColorFilter.h"

namespace
{
  static OpenThreads::Atomic s_uniformNameGen;

  static const char* s_localShaderSource =

    "#version 110\n"

    "uniform vec3  __COLOR_UNIFORM_NAME__;\n"
    "uniform float __DISTANCE_UNIFORM_NAME__;\n"

    "void __ENTRY_POINT__(inout vec4 color)\n"
    "{ \n"
    "    float dist = distance(color.rgb, __COLOR_UNIFORM_NAME__); \n"
    "    if (dist <= __DISTANCE_UNIFORM_NAME__) color.a = 0.0;\n"
    // feathering, if desired in future:
    //"    if (dist <= __DISTANCE_UNIFORM_NAME__) color.a = (dist/__DISTANCE_UNIFORM_NAME__); \n"
    "} \n";
}


//---------------------------------------------------------------------------

#define FUNCTION_PREFIX "osgearthutil_chromakeyColorFilter_"
#define COLOR_UNIFORM_PREFIX  "osgearthutil_u_chromakey_color_"
#define DISTANCE_UNIFORM_PREFIX  "osgearthutil_u_chromakey_distance_"

// This allows for serialization, inclusion in .earth files
OSGEARTH_REGISTER_COLORFILTER(chroma_key, simVis::ChromaKeyColorFilter);

namespace simVis {

ChromaKeyColorFilter::ChromaKeyColorFilter()
{
  init_();
}

ChromaKeyColorFilter::ChromaKeyColorFilter(const osgEarth::Config& conf)
{
  init_();

  osg::Vec3f val;
  val[0] = conf.value("r", 0.0);
  val[1] = conf.value("g", 0.0);
  val[2] = conf.value("b", 0.0);
  setColor(val);

  float distance = 0.0f;
  distance = conf.value("distance", 0.0f);
  setDistance(distance);
}

void ChromaKeyColorFilter::init_()
{
  // Generate a unique name for this filter's uniform. This is necessary
  // so that each layer can have a unique uniform and entry point.
  instanceId_ = (++s_uniformNameGen) - 1;
  colorUniform_ = new osg::Uniform(osg::Uniform::FLOAT_VEC3, (osgEarth::Stringify() << COLOR_UNIFORM_PREFIX << instanceId_));
  //Default to black
  colorUniform_->set(osg::Vec3(0.0f, 0.0f, 0.0f));
  distanceUniform_ = new osg::Uniform(osg::Uniform::FLOAT, (osgEarth::Stringify() << DISTANCE_UNIFORM_PREFIX << instanceId_));
  distanceUniform_->set(0.0f);
}

void ChromaKeyColorFilter::setColor(const osg::Vec3f& color)
{
  colorUniform_->set(color);
}

osg::Vec3f ChromaKeyColorFilter::getColor() const
{
  osg::Vec3f value;
  colorUniform_->get(value);
  return value;
}

void ChromaKeyColorFilter::setDistance(float distance)
{
  distanceUniform_->set(distance);
}

float ChromaKeyColorFilter::getDistance() const
{
  float value;
  distanceUniform_->get(value);
  return value;
}

std::string ChromaKeyColorFilter::getEntryPointFunctionName() const
{
  return (osgEarth::Stringify() << FUNCTION_PREFIX << instanceId_);
}

void ChromaKeyColorFilter::install(osg::StateSet* stateSet) const
{
  // safe: will not add twice.
  stateSet->addUniform(colorUniform_.get());
  stateSet->addUniform(distanceUniform_.get());

  osgEarth::VirtualProgram* vp = dynamic_cast<osgEarth::VirtualProgram*>(stateSet->getAttribute(osgEarth::VirtualProgram::SA_TYPE));
  if (vp)
  {
    // build the local shader (unique per instance). We will
    // use a template with search and replace for this one.
    std::string entryPoint = osgEarth::Stringify() << FUNCTION_PREFIX << instanceId_;
    std::string code = s_localShaderSource;
    osgEarth::replaceIn(code, "__COLOR_UNIFORM_NAME__", colorUniform_->getName());
    osgEarth::replaceIn(code, "__DISTANCE_UNIFORM_NAME__", distanceUniform_->getName());
    osgEarth::replaceIn(code, "__ENTRY_POINT__", entryPoint);

    osg::Shader* main = new osg::Shader(osg::Shader::FRAGMENT, code);
    vp->setShader(entryPoint, main);
  }
}

osgEarth::Config ChromaKeyColorFilter::getConfig() const
{
  osg::Vec3f val = getColor();
  osgEarth::Config conf("chroma_key");
  conf.add("r", val[0]);
  conf.add("g", val[1]);
  conf.add("b", val[2]);

  if (getDistance() != 0.0f)
    conf.add("distance", getDistance());

  return conf;
}

}
