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
#include <iomanip>
#include <sstream>
#include "osgEarth/VirtualProgram"
#include "simVis/RFProp/GradientColorProvider.h"

namespace {

/** Fixed precision for floating point values written to shader GLSL code */
static const int FLOAT_PRECISION = 8;

std::string colorToVec4(const osg::Vec4& color)
{
  std::stringstream buf;
  buf << "vec4(" << std::fixed << std::setprecision(FLOAT_PRECISION) << color.r() << ", "
    << std::fixed << std::setprecision(FLOAT_PRECISION) << color.g() << ", "
    << std::fixed << std::setprecision(FLOAT_PRECISION) << color.b() << ", "
    << std::fixed << std::setprecision(FLOAT_PRECISION) << color.a() << ")";
  return buf.str();
}

std::string printFloat(float f)
{
  std::stringstream buf;
  buf << std::fixed << std::setprecision(FLOAT_PRECISION) << f;
  return buf.str();
}
}

namespace simRF {

GradientColorProvider::GradientColorProvider()
 : discrete_(true)
{
}

void GradientColorProvider::clear()
{
  colors_.clear();
}

bool GradientColorProvider::getDiscrete() const
{
  return discrete_;
}

void GradientColorProvider::setAlpha(float value)
{
  for (simRF::GradientColorProvider::ColorMap::iterator iter = colors_.begin(); iter != colors_.end(); ++iter)
  {
    iter->second[3] = value;
  }
  reloadShader_();
}

void GradientColorProvider::setDiscrete(bool discrete)
{
  if (discrete_ != discrete)
  {
    discrete_ = discrete;
    reloadShader_();
  }
}

void GradientColorProvider::setColor(float value, const osg::Vec4f& color)
{
  colors_[value] = color;
  reloadShader_();
}

void GradientColorProvider::setColorMap(const ColorMap& colors)
{
  colors_ = colors;
  reloadShader_();
}

std::string GradientColorProvider::buildShader_()
{
  std::stringstream buf;

  buf << "#version " GLSL_VERSION_STR << "\n";
  buf << GLSL_DEFAULT_PRECISION_FLOAT << "\n";
  buf << "vec4 lossToColor(in float loss)\n";
  buf << "{\n";

  // No colors? Always return white
  if (colors_.empty())
  {
    buf << "  return vec4(1.0, 1.0, 1.0, 1.0);\n";
    buf << "}\n";
    return buf.str();
  }

  buf << " float lossA = 0.0;\n";
  buf << " float lossB = 0.0;\n";
  buf << " vec4 colorA = vec4(1.0, 1.0, 1.0, 1.0);\n";
  buf << " vec4 colorB = vec4(1.0, 1.0, 1.0, 1.0);\n";

  ColorMap::const_iterator current = colors_.begin();

  // Special case, if the loss value is less than the first value, return the first value
  buf << "if (loss < " << printFloat(current->first) << ") return " << colorToVec4(current->second) << ";\n";

  for (unsigned int i = 0; i < colors_.size() - 1; i++)
  {
    ColorMap::const_iterator next = current;
    ++next;

    buf << "lossA = " << printFloat(current->first) << ";\n";
    buf << "lossB = " << printFloat(next->first) << ";\n";
    buf << "colorA = " << colorToVec4(current->second) << ";\n";
    buf << "colorB = " << colorToVec4(next->second) << ";\n";
    buf << "if (loss >= lossA && loss <= lossB)\n";
    buf << "{\n";
    if (discrete_)
    {
      buf << "    return colorA;\n";
    }
    else
    {
      buf << "    return mix(colorA, colorB, (loss - lossA)/(lossB - lossA) );\n";
    }
    buf << "}\n";

    ++current;
  }

  // Special case, if the value is greater than the last value, return the last value.
  buf << "return " << colorToVec4(current->second) << ";\n " << std::endl;
  buf << "}\n";
  return buf.str();
}

void GradientColorProvider::reloadShader_()
{
  std::string src = buildShader_();

  if (vertShader_)
  {
    vertShader_->setShaderSource(src);
  }
  if (fragShader_)
  {
    fragShader_->setShaderSource(src);
  }
}

void GradientColorProvider::install(osg::StateSet* stateset)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  std::string src = buildShader_();
  vertShader_ = new osg::Shader(osg::Shader::VERTEX, src);
  fragShader_ = new osg::Shader(osg::Shader::FRAGMENT, src);
  vp->setShader(LOSS_TO_COLOR_VERTEX, vertShader_.get());
  vp->setShader(LOSS_TO_COLOR_FRAGMENT, fragShader_.get());
}

void GradientColorProvider::uninstall(osg::StateSet* stateset)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  vp->removeShader(LOSS_TO_COLOR_VERTEX);
  vp->removeShader(LOSS_TO_COLOR_FRAGMENT);
}

}
