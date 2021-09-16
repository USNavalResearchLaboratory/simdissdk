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
#include <iomanip>
#include <sstream>
#include "osgEarth/VirtualProgram"
#include "simVis/RFProp/GradientColorProvider.h"

namespace simRF {

GradientColorProvider::GradientColorProvider()
{
  // Initialize the gradient generator
  gradient_.setFunctionName("lossToColor");
  gradient_.setDiscrete(true);
  // Cover the special case, if the loss value is invalid/no-data
  gradient_.setSpecialCaseCode("  if (value < -32765.0) return vec4(0.0, 0.0, 0.0, 0.0);\n");
}

void GradientColorProvider::clear()
{
  gradient_.clear();
}

bool GradientColorProvider::getDiscrete() const
{
  return gradient_.isDiscrete();
}

void GradientColorProvider::setAlpha(float value)
{
  gradient_.setAlpha(value);
  reloadShader_();
}

void GradientColorProvider::setDiscrete(bool discrete)
{
  if (gradient_.isDiscrete() != discrete)
  {
    gradient_.setDiscrete(discrete);
    reloadShader_();
  }
}

void GradientColorProvider::setColor(float value, const osg::Vec4f& color)
{
  gradient_.setColor(value, color);
  reloadShader_();
}

void GradientColorProvider::setColorMap(const ColorMap& colors)
{
  gradient_.setColorMap(colors);
  reloadShader_();
}

void GradientColorProvider::reloadShader_()
{
  const std::string& src = gradient_.buildShader();

  if (vertShader_)
    vertShader_->setShaderSource(src);
  if (fragShader_)
    fragShader_->setShaderSource(src);
}

void GradientColorProvider::install(osg::StateSet* stateset)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  std::string src = gradient_.buildShader();
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
