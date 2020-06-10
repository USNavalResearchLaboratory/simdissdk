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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <algorithm>
#include "simCore/Calc/Angle.h"
#include "simCore/String/Angle.h"
#include "simUtil/TritonSettings.h"

namespace simUtil {

/** Ensures the apply_ method is only called when values change. */
void TritonValue::apply(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  if (shouldApply_)
  {
    shouldApply_ = false;
    apply_(env, ocean);
  }
}

void TritonValue::setShouldApply_()
{
  shouldApply_ = true;
}

void TritonValue::initialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  // noop
}

TritonValue::TritonValue()
  : shouldApply_(false)
{
}

TritonValue::~TritonValue()
{
}

/////////////////////////////////////////////////////////

#define TRI_IMPL_SIMPLE_SETTING(CLASS, TYPE, DEFAULT_VALUE, SET_METHOD) \
CLASS::CLASS() : TritonValueT<TYPE>(DEFAULT_VALUE) { } \
CLASS::~CLASS() {} \
void CLASS::initialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean) \
{ \
  apply_(env, ocean); \
} \
void CLASS::apply_(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean) \
{ \
  SET_METHOD(value()); \
}

// Various "simple" settings that all follow the same pattern
TRI_IMPL_SIMPLE_SETTING(TritonChoppiness, double, 1.6, ocean.SetChoppiness);
TRI_IMPL_SIMPLE_SETTING(TritonSunIntensity, double, 1.0, env.SetSunIntensity);
TRI_IMPL_SIMPLE_SETTING(TritonEnableSpray, bool, true, ocean.EnableSpray);
TRI_IMPL_SIMPLE_SETTING(TritonEnableWireframe, bool, false, ocean.EnableWireframe);
TRI_IMPL_SIMPLE_SETTING(TritonEnableGodRays, bool, false, ocean.EnableGodRays);
TRI_IMPL_SIMPLE_SETTING(TritonGodRaysFade, double, 0.0, ocean.SetGodRaysFade);

////////////////////////////////////////////////////////////////////

TritonQuality::TritonQuality()
 : TritonValueT<osgEarth::Triton::OceanQuality>(osgEarth::Triton::GOOD)
{
}

TritonQuality::~TritonQuality()
{
}

void TritonQuality::initialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  ocean.SetQuality(value());
}

void TritonQuality::apply_(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  // This is a no-op because changing the quality at runtime seems
  // to cause significant issues, requiring a full reload of the ocean
}

/////////////////////////////////////////////////////////

TritonSeaState::TritonSeaState()
  : TritonValue(),
    seaState_(4.0),
    windDirectionRad_(0.0)
{
}

TritonSeaState::~TritonSeaState()
{
}

double TritonSeaState::windDirection() const
{
  return windDirectionRad_;
}

double TritonSeaState::seaState() const
{
  return seaState_;
}

void TritonSeaState::setWindDirection(double windDirectionRad, bool forceApply)
{
  // Ignore the set, if the value matches
  if (!forceApply && windDirectionRad_ == windDirectionRad)
    return;
  windDirectionRad_ = windDirectionRad;
  setShouldApply_();
}

void TritonSeaState::setSeaState(double seaState, bool forceApply)
{
  // Ignore the set, if the value matches
  if (!forceApply && seaState_ == seaState)
    return;
  seaState_ = seaState;
  setShouldApply_();
}

void TritonSeaState::initialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  apply_(env, ocean);
}

void TritonSeaState::apply_(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  env.SimulateSeaState(seaState(), windDirection());
}

/////////////////////////////////////////////////////////

TritonSettingsCallback::~TritonSettingsCallback()
{
}

void TritonSettingsCallback::addValue(TritonValue* value)
{
  values_.push_back(value);
}

void TritonSettingsCallback::removeValue(TritonValue* value)
{
  values_.erase(std::remove(values_.begin(), values_.end(), value), values_.end());
}

void TritonSettingsCallback::onInitialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  for (std::vector<osg::ref_ptr<TritonValue> >::const_iterator i = values_.begin(); i != values_.end(); ++i)
    (*i)->initialize(env, ocean);
}

void TritonSettingsCallback::onDrawOcean(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean)
{
  for (std::vector<osg::ref_ptr<TritonValue> >::const_iterator i = values_.begin(); i != values_.end(); ++i)
    (*i)->apply(env, ocean);
}

/////////////////////////////////////////////////////////

TritonSettingsAdapter::TritonSettingsAdapter()
  : choppiness_(new TritonChoppiness),
    quality_(new TritonQuality),
    seaState_(new TritonSeaState),
    sunIntensity_(new TritonSunIntensity),
    enableSpray_(new TritonEnableSpray),
    enableWireframe_(new TritonEnableWireframe),
    enableGodRays_(new TritonEnableGodRays),
    godRaysFade_(new TritonGodRaysFade)
{
  addValue(choppiness_.get());
  addValue(quality_.get());
  addValue(seaState_.get());
  addValue(sunIntensity_.get());
  addValue(enableSpray_.get());
  addValue(enableWireframe_.get());
  addValue(enableGodRays_.get());
  addValue(godRaysFade_.get());
}

TritonSettingsAdapter::~TritonSettingsAdapter()
{
}

TritonChoppiness* TritonSettingsAdapter::choppiness() const
{
  return choppiness_.get();
}

TritonQuality* TritonSettingsAdapter::quality() const
{
  return quality_.get();
}

TritonSeaState* TritonSettingsAdapter::seaState() const
{
  return seaState_.get();
}

TritonSunIntensity* TritonSettingsAdapter::sunIntensity() const
{
  return sunIntensity_.get();
}

TritonEnableSpray* TritonSettingsAdapter::enableSpray() const
{
  return enableSpray_.get();
}

TritonEnableWireframe* TritonSettingsAdapter::enableWireframe() const
{
  return enableWireframe_.get();
}

TritonEnableGodRays* TritonSettingsAdapter::enableGodRays() const
{
  return enableGodRays_.get();
}

TritonGodRaysFade* TritonSettingsAdapter::godRaysFade() const
{
  return godRaysFade_.get();
}

/////////////////////////////////////////////////////////

#define TRI_IMPL_SIMPLE_EVTHANDLER(CLASS, SETTING, VALUETYPE) \
CLASS::CLASS(SETTING* value) : value_(value) {} \
CLASS::~CLASS() {} \
void CLASS::onValueChanged(osgEarth::Util::Controls::Control* c, VALUETYPE value) { \
  osg::ref_ptr<SETTING> refValue; \
  if (value_.lock(refValue)) \
    refValue->set(value); \
}

TRI_IMPL_SIMPLE_EVTHANDLER(ChoppinessEventHandler, TritonChoppiness, double);
TRI_IMPL_SIMPLE_EVTHANDLER(SunIntensityEventHandler, TritonSunIntensity, double);
TRI_IMPL_SIMPLE_EVTHANDLER(EnableSprayEventHandler, TritonEnableSpray, bool);
TRI_IMPL_SIMPLE_EVTHANDLER(EnableWireframeEventHandler, TritonEnableWireframe, bool);
TRI_IMPL_SIMPLE_EVTHANDLER(EnableGodRaysEventHandler, TritonEnableGodRays, bool);
TRI_IMPL_SIMPLE_EVTHANDLER(GodRaysFadeEventHandler, TritonGodRaysFade, double);

//////////////////////////////////////////////////////

WindDirectionDegEventHandler::WindDirectionDegEventHandler(TritonSeaState* value)
  : value_(value)
{
}

WindDirectionDegEventHandler::~WindDirectionDegEventHandler()
{
}

void WindDirectionDegEventHandler::onValueChanged(osgEarth::Util::Controls::Control* c, double value)
{
  osg::ref_ptr<TritonSeaState> refValue;
  if (value_.lock(refValue))
    refValue->setWindDirection(value * simCore::DEG2RAD);
}

//////////////////////////////////////////////////////

SeaStateEventHandler::SeaStateEventHandler(TritonSeaState* value)
  : value_(value)
{
}

SeaStateEventHandler::~SeaStateEventHandler()
{
}

void SeaStateEventHandler::onValueChanged(osgEarth::Util::Controls::Control* c, double value)
{
  osg::ref_ptr<TritonSeaState> refValue;
  if (value_.lock(refValue))
    refValue->setSeaState(value);
}

/////////////////////////////////////////////////////////

QualityEventHandler::QualityEventHandler(TritonQuality* value)
  : value_(value)
{
}

QualityEventHandler::~QualityEventHandler()
{
}

void QualityEventHandler::onValueChanged(osgEarth::Util::Controls::Control* c, double value)
{
  osg::ref_ptr<TritonQuality> refValue;
  if (!value_.lock(refValue))
    return;
  if (value < 1.0)
    refValue->set(osgEarth::Triton::GOOD);
  else if (value < 2.0)
    refValue->set(osgEarth::Triton::BETTER);
  else
    refValue->set(osgEarth::Triton::BEST);
}

/////////////////////////////////////////////////////////

QualityTextUpdater::QualityTextUpdater(osgEarth::Util::Controls::LabelControl* label)
  : label_(label)
{
}

QualityTextUpdater::~QualityTextUpdater()
{
}

void QualityTextUpdater::onValueChanged(osgEarth::Util::Controls::Control* c, double value)
{
  osg::ref_ptr<osgEarth::Util::Controls::LabelControl> label;
  if (!label_.lock(label))
    return;
  if (value < 1.0)
    label->setText("Good");
  else if (value < 2.0)
    label->setText("Better");
  else
    label->setText("Best");
}

}
