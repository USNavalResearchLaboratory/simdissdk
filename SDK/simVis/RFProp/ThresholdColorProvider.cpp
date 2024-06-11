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
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/RFProp/ThresholdColorProvider.h"

namespace simRF {

ThresholdColorProvider::ThresholdColorProvider()
 : belowColor_(0, 1, 0, 1),
   aboveColor_(1, 0, 0, 1),
   threshold_(0),
   mode_(COLORMODE_ABOVE_AND_BELOW)
{
  init_();
}

ThresholdColorProvider::ThresholdColorProvider(const osg::Vec4f& belowColor, const osg::Vec4f& aboveColor, float threshold, simRF::ColorProvider::ColorMode mode)
 : belowColor_(belowColor),
   aboveColor_(aboveColor),
   threshold_(threshold),
   mode_(mode)
{
  init_();
}

void ThresholdColorProvider::init_()
{
  belowColorUniform_ = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "belowColor");
  belowColorUniform_->set(belowColor_);
  aboveColorUniform_ = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "aboveColor");
  aboveColorUniform_->set(aboveColor_);
  thresholdUniform_ = new osg::Uniform(osg::Uniform::FLOAT, "threshold");
  thresholdUniform_->set(threshold_);
  modeUniform_ = new osg::Uniform(osg::Uniform::INT, "mode");
  modeUniform_->set(static_cast<int>(mode_));
}

ThresholdColorProvider::ColorMode ThresholdColorProvider::getMode() const
{
  return mode_;
}

void ThresholdColorProvider::setMode(ColorMode mode)
{
  mode_ = mode;
  modeUniform_->set(static_cast<int>(mode_));
}

const osg::Vec4f& ThresholdColorProvider::getBelowColor() const
{
  return belowColor_;
}

void ThresholdColorProvider::setBelowColor(const osg::Vec4f& belowColor)
{
  belowColor_ = belowColor;
  belowColorUniform_->set(belowColor_);
}

const osg::Vec4f& ThresholdColorProvider::getAboveColor() const
{
  return aboveColor_;
}

void ThresholdColorProvider::setAboveColor(const osg::Vec4f& aboveColor)
{
  aboveColor_ = aboveColor;
  aboveColorUniform_->set(aboveColor_);
}


float ThresholdColorProvider::getThreshold() const
{
  return threshold_;
}

void ThresholdColorProvider::setThreshold(float threshold)
{
  threshold_ = threshold;
  thresholdUniform_->set(threshold_);
}

void ThresholdColorProvider::install(osg::StateSet* stateset)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  simVis::Shaders package;
  const std::string src = osgEarth::Util::ShaderLoader::load(package.rfPropLossToColorThreshold(), package);
  vp->setShader(LOSS_TO_COLOR_VERTEX, new osg::Shader(osg::Shader::VERTEX, src));
  vp->setShader(LOSS_TO_COLOR_FRAGMENT, new osg::Shader(osg::Shader::FRAGMENT, src));
  stateset->addUniform(belowColorUniform_.get());
  stateset->addUniform(aboveColorUniform_.get());
  stateset->addUniform(thresholdUniform_.get());
  stateset->addUniform(modeUniform_.get());
}

void ThresholdColorProvider::uninstall(osg::StateSet* stateset)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  vp->removeShader(LOSS_TO_COLOR_VERTEX);
  vp->removeShader(LOSS_TO_COLOR_FRAGMENT);
  stateset->removeUniform(belowColorUniform_.get());
  stateset->removeUniform(aboveColorUniform_.get());
  stateset->removeUniform(thresholdUniform_.get());
  stateset->removeUniform(modeUniform_.get());
}

}
