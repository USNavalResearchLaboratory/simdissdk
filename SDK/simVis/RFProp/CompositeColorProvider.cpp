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
#include <cassert>
#include "simVis/Types.h"
#include "simVis/RFProp/CompositeColorProvider.h"

namespace simRF
{

/// Above threshold color; use red for above, i.e. the area where host can detect
const osg::Vec4f DEFAULT_ABOVE_COLOR = simVis::Color::Red;
/// Below threshold color; use green for below, i.e. the area where host cannot detect
const osg::Vec4f DEFAULT_BELOW_COLOR = simVis::Color::Green;
/// assumes default type is Loss threshold, values are 0-300 dBsm
const float DEFAULT_THRESHOLD = 150.0f;

CompositeColorProvider::CompositeColorProvider()
 : colorMode_(COLORMODE_ABOVE_AND_BELOW),
   transparency_(0),
   gradientProvider_(new GradientColorProvider()),
   thresholdProvider_(new ThresholdColorProvider(DEFAULT_BELOW_COLOR,
                                                 DEFAULT_ABOVE_COLOR,
                                                 DEFAULT_THRESHOLD,
                                                 colorMode_))
{
}

CompositeColorProvider::ColorMode CompositeColorProvider::getMode() const
{
  return colorMode_;
}

void CompositeColorProvider::setMode(ColorMode mode)
{
  if (colorMode_ == mode)
    return;

  // if swapping color providers, clean out the other color provider
  if (lastStateSet_.valid())
  {
    if (colorMode_ == simRF::ColorProvider::COLORMODE_GRADIENT)
      gradientProvider_->uninstall(lastStateSet_.get());
    else
      thresholdProvider_->uninstall(lastStateSet_.get());
  }

  colorMode_ = mode;

  /// initialize the gradient color provider with the current color map
  if (mode == simRF::ColorProvider::COLORMODE_GRADIENT)
  {
    updateGradientColorMap_();
    if (lastStateSet_.valid())
      gradientProvider_->install(lastStateSet_.get());
  }
  else
  {
    thresholdProvider_->setMode(mode);
    if (lastStateSet_.valid())
      thresholdProvider_->install(lastStateSet_.get());
  }
}

const osg::Vec4f& CompositeColorProvider::getBelowColor() const
{
  return thresholdProvider_->getBelowColor();
}

void CompositeColorProvider::setBelowColor(const osg::Vec4f& belowColor)
{
  osg::Vec4f adjustedColor = belowColor;
  adjustedColor[3] = (100.0f - transparency_) / 100.0f;
  thresholdProvider_->setBelowColor(adjustedColor);
}

const osg::Vec4f& CompositeColorProvider::getAboveColor() const
{
  return thresholdProvider_->getAboveColor();
}

void CompositeColorProvider::setAboveColor(const osg::Vec4f& aboveColor)
{
  osg::Vec4f adjustedColor = aboveColor;
  adjustedColor[3] = (100.0f - transparency_) / 100.0f;
  thresholdProvider_->setAboveColor(adjustedColor);
}


float CompositeColorProvider::getThreshold() const
{
  return thresholdProvider_->getThreshold();
}

void CompositeColorProvider::setThreshold(float threshold)
{
  thresholdProvider_->setThreshold(threshold);
}

void CompositeColorProvider::setGradientColor(float value, const osg::Vec4f& color)
{
  osg::Vec4f adjustedColor = color;
  adjustedColor[3] = (100.0f - transparency_) / 100.0f;
  if (colorMode_ == simRF::ColorProvider::COLORMODE_GRADIENT)
    gradientProvider_->setColor(value, adjustedColor);
  // store the color mapping locally
  gradientColors_[value] = adjustedColor;
}

void CompositeColorProvider::setGradientColorMap(const simRF::GradientColorProvider::ColorMap& colors)
{
  gradientColors_ = colors;
  // adjust for transparency
  updateGradientColorMap_();
}

bool CompositeColorProvider::getGradientDiscrete() const
{
  return gradientProvider_->getDiscrete();
}

void CompositeColorProvider::setGradientDiscrete(bool discrete)
{
  gradientProvider_->setDiscrete(discrete);
}

void CompositeColorProvider::clearGradient()
{
  gradientProvider_->clear();
}

void CompositeColorProvider::setTransparency(int transparency)
{
  transparency_ = transparency;
  float alpha = (100.0f - transparency_) / 100.0f;

  if (colorMode_ == simRF::ColorProvider::COLORMODE_GRADIENT)
    gradientProvider_->setAlpha(alpha);

  // now set above/below alpha in threshold provider
  osg::Vec4f color = thresholdProvider_->getAboveColor();
  color[3] = alpha;
  thresholdProvider_->setAboveColor(color);

  color = thresholdProvider_->getBelowColor();
  color[3] = alpha;
  thresholdProvider_->setBelowColor(color);
}

int CompositeColorProvider::transparency() const
{
  return transparency_;
}

void CompositeColorProvider::install(osg::StateSet* stateset)
{
  // We only support installing to a single state set; if multiples are added, code needs updating
  assert(!lastStateSet_.valid() || lastStateSet_.get() == stateset);

  lastStateSet_ = stateset;
  if (colorMode_ == COLORMODE_GRADIENT)
    gradientProvider_->install(stateset);
  else
    thresholdProvider_->install(stateset);
}

void CompositeColorProvider::uninstall(osg::StateSet* stateset)
{
  lastStateSet_ = nullptr;
  if (colorMode_ == COLORMODE_GRADIENT)
    gradientProvider_->uninstall(stateset);
  else
    thresholdProvider_->uninstall(stateset);
}

void CompositeColorProvider::updateGradientColorMap_()
{
  // adjust for transparency
  for (simRF::GradientColorProvider::ColorMap::iterator iter = gradientColors_.begin(); iter != gradientColors_.end(); ++iter)
  {
    iter->second[3] = (100.0f - transparency_) / 100.0f;
  }

  if (colorMode_ == simRF::ColorProvider::COLORMODE_GRADIENT)
    gradientProvider_->setColorMap(gradientColors_);
}

}
