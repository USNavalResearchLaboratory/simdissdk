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
#include <cassert>
#include "osgEarth/VirtualProgram"
#include "simVis/Shaders.h"
#include "simVis/BeamPulse.h"

namespace simVis
{

namespace
{
  const std::string USE_BEAMPULSE_UNIFORM = "simvis_beampulse_enabled";
  const std::string LENGTH_UNIFORM = "simvis_beampulse_length";
  const std::string RATE_UNIFORM = "simvis_beampulse_rate";
  const std::string STIPPLE_PATTERN_UNIFORM = "simvis_beampulse_stipplepattern";

  const float DEFAULT_LENGTH = 100.0f; // meters
  const float DEFAULT_RATE= 1.0f; // hz
  const unsigned int DEFAULT_STIPPLE = 0x0f0fu; // bitmask
}

BeamPulse::BeamPulse(osg::StateSet* stateset)
  : stateSet_(stateset)
{
  if (stateSet_.valid())
  {
    enabled_ = stateSet_->getOrCreateUniform(USE_BEAMPULSE_UNIFORM, osg::Uniform::BOOL);
    enabled_->set(true);
    length_ = stateSet_->getOrCreateUniform(LENGTH_UNIFORM, osg::Uniform::FLOAT);
    length_->set(DEFAULT_LENGTH);
    rate_ = stateSet_->getOrCreateUniform(RATE_UNIFORM, osg::Uniform::FLOAT);
    rate_->set(DEFAULT_RATE);
    stipplePattern_ = stateSet_->getOrCreateUniform(STIPPLE_PATTERN_UNIFORM, osg::Uniform::UNSIGNED_INT);
    stipplePattern_->set(DEFAULT_STIPPLE);
  }
}

BeamPulse::~BeamPulse()
{
  if (stateSet_.valid())
  {
    stateSet_->removeUniform(enabled_);
    stateSet_->removeUniform(length_);
    stateSet_->removeUniform(rate_);
    stateSet_->removeUniform(stipplePattern_);
  }
}

void BeamPulse::installShaderProgram(osg::StateSet* intoStateSet)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);
  simVis::Shaders shaders;
  shaders.load(vp, shaders.beamPulseVertex());
  shaders.load(vp, shaders.beamPulseFragment());
  BeamPulse::setDefaultValues_(intoStateSet);
}

void BeamPulse::setEnabled(bool active)
{
  if (enabled_.valid())
    enabled_->set(active);
}

void BeamPulse::setLength(float length)
{
  if (length_.valid())
    length_->set(length);
}

void BeamPulse::setRate(float rate)
{
  if (rate_.valid())
    rate_->set(rate);
}

void BeamPulse::setStipplePattern(uint16_t pattern)
{
  if (stipplePattern_.valid())
    stipplePattern_->set(static_cast<unsigned int>(pattern));
}

void BeamPulse::setDefaultValues_(osg::StateSet* stateSet)
{
  stateSet
    ->getOrCreateUniform(USE_BEAMPULSE_UNIFORM, osg::Uniform::BOOL)
    ->set(false);
  stateSet
    ->getOrCreateUniform(LENGTH_UNIFORM, osg::Uniform::FLOAT)
    ->set(DEFAULT_LENGTH);
  stateSet
    ->getOrCreateUniform(RATE_UNIFORM, osg::Uniform::FLOAT)
    ->set(DEFAULT_RATE);
  stateSet
    ->getOrCreateUniform(STIPPLE_PATTERN_UNIFORM, osg::Uniform::UNSIGNED_INT)
    ->set(DEFAULT_STIPPLE);
}

}
