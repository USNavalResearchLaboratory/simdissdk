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
#include <iostream>
#include "simCore/Time/Clock.h"
#include "simVis/Registry.h"
#include "simUtil/SilverLiningSettings.h"

namespace simUtil {

/** Ensures the apply_ method is only called when values change. */
void SilverLiningValue::apply(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  if (shouldApply_)
  {
    shouldApply_ = false;
    apply_(atmosphere);
  }
}

void SilverLiningValue::setShouldApply_()
{
  shouldApply_ = true;
}

void SilverLiningValue::initialize(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  // noop
}

SilverLiningValue::SilverLiningValue()
  : shouldApply_(false)
{
}

SilverLiningValue::~SilverLiningValue()
{
}

/////////////////////////////////////////////////////////

#define SL_IMPL_SETTING(CLASS, TYPE, DEFAULT_VALUE, SET_METHOD) \
CLASS::CLASS() : SilverLiningValueT<TYPE>(DEFAULT_VALUE) { } \
CLASS::~CLASS() {} \
void CLASS::initialize(osgEarth::SilverLining::Atmosphere& atmosphere) \
{ \
  apply_(atmosphere); \
} \
void CLASS::apply_(osgEarth::SilverLining::Atmosphere& atmosphere) \
{ \
  atmosphere.SET_METHOD; \
}

#define SL_IMPL_SIMPLE_SETTING(CLASS, TYPE, DEFAULT_VALUE, SET_METHOD) SL_IMPL_SETTING(CLASS, TYPE, DEFAULT_VALUE, SET_METHOD(value()))

/** Local enumeration for precipitation type matching */
enum PrecipitationType
{
  NONE,
  RAIN,
  DRY_SNOW,
  WET_SNOW,
  SLEET
};

// Various "simple" settings that all follow the same pattern
SL_IMPL_SIMPLE_SETTING(SilverLiningLensFlare, bool, false, EnableLensFlare);
SL_IMPL_SIMPLE_SETTING(SilverLiningVisibility, double, 30000, GetConditions().SetVisibility);
SL_IMPL_SETTING(SilverLiningRainRate, double, 0.0, GetConditions().SetPrecipitation(RAIN, value()));
SL_IMPL_SETTING(SilverLiningDrySnowRate, double, 0.0, GetConditions().SetPrecipitation(DRY_SNOW, value()));
SL_IMPL_SETTING(SilverLiningWetSnowRate, double, 0.0, GetConditions().SetPrecipitation(WET_SNOW, value()));
SL_IMPL_SETTING(SilverLiningSleetRate, double, 0.0, GetConditions().SetPrecipitation(SLEET, value()));
SL_IMPL_SIMPLE_SETTING(SilverLiningTurbidity, double, 2.2, GetConditions().SetTurbidity);
SL_IMPL_SIMPLE_SETTING(SilverLiningLightPollution, double, 0.0, GetConditions().SetLightPollution);
SL_IMPL_SIMPLE_SETTING(SilverLiningGamma, double, 1.8, SetGamma);
SL_IMPL_SIMPLE_SETTING(SilverLiningInfrared, bool, false, SetInfraRedMode);
SL_IMPL_SETTING(SilverLiningSkyModel, int, osgEarth::SilverLining::Atmosphere::PREETHAM, SetSkyModel(static_cast<osgEarth::SilverLining::Atmosphere::SkyModel>(value())));
SL_IMPL_SETTING(SilverLiningConditionPreset, int, osgEarth::SilverLining::AtmosphericConditions::PARTLY_CLOUDY,
  GetConditions().SetPresetConditions(static_cast<osgEarth::SilverLining::AtmosphericConditions::ConditionPresets>(value()), atmosphere));

/////////////////////////////////////////////////////////

SilverLiningSnowRate::SilverLiningSnowRate()
  : SilverLiningValue(),
    rate_(0.0),
    isWet_(false)
{
}

SilverLiningSnowRate::~SilverLiningSnowRate()
{
}

double SilverLiningSnowRate::rate() const
{
  return rate_;
}

bool SilverLiningSnowRate::isWet() const
{
  return isWet_;
}

void SilverLiningSnowRate::setRate(double rate, bool forceApply)
{
  // Ignore the set, if the value matches
  if (!forceApply && rate_ == rate)
    return;
  rate_ = rate;
  setShouldApply_();
}

void SilverLiningSnowRate::setWet(bool isWet, bool forceApply)
{
  // Ignore the set, if the value matches
  if (!forceApply && isWet_ == isWet)
    return;
  isWet_ = isWet;
  setShouldApply_();
}

void SilverLiningSnowRate::initialize(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  apply_(atmosphere);
}

void SilverLiningSnowRate::apply_(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  if (isWet_)
  {
    atmosphere.GetConditions().SetPrecipitation(osgEarth::SilverLining::CloudLayer::DRY_SNOW, 0.0);
    atmosphere.GetConditions().SetPrecipitation(osgEarth::SilverLining::CloudLayer::WET_SNOW, rate_);
  }
  else
  {
    // Cannot have dry snow if wet snow already exists, in SL 4.058
    atmosphere.GetConditions().SetPrecipitation(osgEarth::SilverLining::CloudLayer::NONE, 0.0);
    atmosphere.GetConditions().SetPrecipitation(osgEarth::SilverLining::CloudLayer::DRY_SNOW, rate_);
  }
}

/////////////////////////////////////////////////////////

SilverLiningWind::SilverLiningWind()
  : SilverLiningValue(),
    directionDeg_(0.0),
    speedMs_(0.0)
{
}

SilverLiningWind::~SilverLiningWind()
{
}

double SilverLiningWind::direction() const
{
  return directionDeg_;
}

double SilverLiningWind::speed() const
{
  return speedMs_;
}

void SilverLiningWind::setDirection(double directionDeg, bool forceApply)
{
  // Ignore the set, if the value matches
  if (!forceApply && directionDeg_ == directionDeg)
    return;
  directionDeg_ = directionDeg;
  setShouldApply_();
}

void SilverLiningWind::setSpeed(double speedMs, bool forceApply)
{
  // Ignore the set, if the value matches
  if (!forceApply && speedMs_ == speedMs)
    return;
  speedMs_ = speedMs;
  setShouldApply_();
}

void SilverLiningWind::initialize(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  apply_(atmosphere);
}

void SilverLiningWind::apply_(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  atmosphere.GetConditions().ClearWindVolumes();
  // In SL, 0.0 is east-blowing (from west)
  atmosphere.GetConditions().SetWind(speedMs_, directionDeg_ + 90.0);
}

/////////////////////////////////////////////////////////

unsigned long SLAlwaysRealTime::getMilliseconds() const
{
  // Use the osgEarth built-in return of current system time
  return 0;
}

/////////////////////////////////////////////////////////

unsigned long SLRegistryClockTime::getMilliseconds() const
{
  const simCore::Clock* clock = simVis::Registry::instance()->getClock();
  // Fall back to default dispaly if clock is nullptr
  if (clock == nullptr)
    return 0;
  // Avoid negative values
  const simCore::Seconds& elapsed = clock->currentTime() - clock->startTime();
  // Avoid returning 0, to avoid the default behavior
  return (elapsed < 0.0) ? 1 : 1 + static_cast<unsigned long>(elapsed.Double() * 1000);
}

/////////////////////////////////////////////////////////

SilverLiningSettingsCallback::~SilverLiningSettingsCallback()
{
}

void SilverLiningSettingsCallback::addValue(SilverLiningValue* value)
{
  if (!timeStrategy_.valid())
    timeStrategy_ = new SLRegistryClockTime;
  values_.push_back(value);
}

void SilverLiningSettingsCallback::removeValue(SilverLiningValue* value)
{
  values_.erase(std::remove(values_.begin(), values_.end(), value), values_.end());
}

void SilverLiningSettingsCallback::setTimeStrategy(SilverLiningTimeStrategy* timeStrategy)
{
  timeStrategy_ = timeStrategy;
}

void SilverLiningSettingsCallback::onInitialize(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  for (std::vector<osg::ref_ptr<SilverLiningValue> >::const_iterator i = values_.begin(); i != values_.end(); ++i)
    (*i)->initialize(atmosphere);
}

void SilverLiningSettingsCallback::onDrawSky(osgEarth::SilverLining::Atmosphere& atmosphere)
{
  for (std::vector<osg::ref_ptr<SilverLiningValue> >::const_iterator i = values_.begin(); i != values_.end(); ++i)
    (*i)->apply(atmosphere);
}

unsigned long SilverLiningSettingsCallback::getMilliseconds() const
{
  if (timeStrategy_.valid())
    return timeStrategy_->getMilliseconds();
  // Fall back to 0, using the default osgEarth built-in way
  return 0;
}

/////////////////////////////////////////////////////////

SilverLiningSettingsAdapter::SilverLiningSettingsAdapter()
  : conditionPreset_(new SilverLiningConditionPreset),
    lensFlare_(new SilverLiningLensFlare),
    gamma_(new SilverLiningGamma),
    infrared_(new SilverLiningInfrared),
    skyModel_(new SilverLiningSkyModel),
    visibility_(new SilverLiningVisibility),
    turbidity_(new SilverLiningTurbidity),
    lightPollution_(new SilverLiningLightPollution),
    snowRate_(new SilverLiningSnowRate),
    rainRate_(new SilverLiningRainRate),
    sleetRate_(new SilverLiningSleetRate),
    wind_(new SilverLiningWind)
{
  addValue(conditionPreset_.get());
  addValue(lensFlare_.get());
  addValue(gamma_.get());
  addValue(infrared_.get());
  addValue(skyModel_.get());
  addValue(visibility_.get());
  addValue(turbidity_.get());
  addValue(lightPollution_.get());
  // Snow rate should come first since it can reset conditions on wet/dry change
  addValue(snowRate_.get());
  addValue(rainRate_.get());
  addValue(sleetRate_.get());
  addValue(wind_.get());
}

SilverLiningSettingsAdapter::~SilverLiningSettingsAdapter()
{
}

SilverLiningConditionPreset* SilverLiningSettingsAdapter::conditionPreset() const
{
  return conditionPreset_.get();
}

SilverLiningLensFlare* SilverLiningSettingsAdapter::lensFlare() const
{
  return lensFlare_.get();
}

SilverLiningGamma* SilverLiningSettingsAdapter::gamma() const
{
  return gamma_.get();
}

SilverLiningInfrared* SilverLiningSettingsAdapter::infrared() const
{
  return infrared_.get();
}

SilverLiningSkyModel* SilverLiningSettingsAdapter::skyModel() const
{
  return skyModel_.get();
}

SilverLiningVisibility* SilverLiningSettingsAdapter::visibility() const
{
  return visibility_.get();
}

SilverLiningTurbidity* SilverLiningSettingsAdapter::turbidity() const
{
  return turbidity_.get();
}

SilverLiningLightPollution* SilverLiningSettingsAdapter::lightPollution() const
{
  return lightPollution_.get();
}

SilverLiningRainRate* SilverLiningSettingsAdapter::rainRate() const
{
  return rainRate_.get();
}

SilverLiningSleetRate* SilverLiningSettingsAdapter::sleetRate() const
{
  return sleetRate_.get();
}

SilverLiningSnowRate* SilverLiningSettingsAdapter::snowRate() const
{
  return snowRate_.get();
}

SilverLiningWind* SilverLiningSettingsAdapter::wind() const
{
  return wind_.get();
}

/////////////////////////////////////////////////////////

#define SL_IMPL_EVTHANDLER(CLASS, SETTING, VALUETYPE, SETMETHOD) \
CLASS::CLASS(SETTING* value) : value_(value) {} \
CLASS::~CLASS() {} \
void CLASS::onValueChanged(osgEarth::Util::Controls::Control* c, VALUETYPE value) { \
  osg::ref_ptr<SETTING> refValue; \
  if (value_.lock(refValue)) \
    refValue->SETMETHOD; \
}

#define SL_IMPL_SIMPLE_EVTHANDLER(CLASS, SETTING, VALUETYPE) SL_IMPL_EVTHANDLER(CLASS, SETTING, VALUETYPE, set(value))

SL_IMPL_SIMPLE_EVTHANDLER(LensFlareEventHandler, SilverLiningLensFlare, bool);
SL_IMPL_SIMPLE_EVTHANDLER(GammaEventHandler, SilverLiningGamma, double);
SL_IMPL_SIMPLE_EVTHANDLER(InfraredEventHandler, SilverLiningInfrared, bool);
SL_IMPL_SIMPLE_EVTHANDLER(VisibilityEventHandler, SilverLiningVisibility, double);
SL_IMPL_SIMPLE_EVTHANDLER(TurbidityEventHandler, SilverLiningTurbidity, double);
SL_IMPL_SIMPLE_EVTHANDLER(LightPollutionEventHandler, SilverLiningLightPollution, double);
SL_IMPL_SIMPLE_EVTHANDLER(RainRateEventHandler, SilverLiningRainRate, double);
SL_IMPL_SIMPLE_EVTHANDLER(DrySnowRateEventHandler, SilverLiningDrySnowRate, double);
SL_IMPL_SIMPLE_EVTHANDLER(WetSnowRateEventHandler, SilverLiningWetSnowRate, double);
SL_IMPL_SIMPLE_EVTHANDLER(SleetRateEventHandler, SilverLiningSleetRate, double);

SL_IMPL_EVTHANDLER(SnowRateEventHandler, SilverLiningSnowRate, double, setRate(value));
SL_IMPL_EVTHANDLER(SnowIsWetEventHandler, SilverLiningSnowRate, bool, setWet(value));

SL_IMPL_EVTHANDLER(SlWindDirectionDegEventHandler, SilverLiningWind, double, setDirection(value));
SL_IMPL_EVTHANDLER(SlWindSpeedEventHandler, SilverLiningWind, double, setSpeed(value));

SL_IMPL_EVTHANDLER(HosekWilkieToggleEventHandler, SilverLiningSkyModel, bool,
  set(value ? osgEarth::SilverLining::Atmosphere::HOSEK_WILKIE : osgEarth::SilverLining::Atmosphere::PREETHAM));

SetConditionPresetEventHandler::SetConditionPresetEventHandler(SilverLiningConditionPreset* preset, int value)
  : preset_(preset),
    value_(value)
{
}

SetConditionPresetEventHandler::~SetConditionPresetEventHandler()
{
}

void SetConditionPresetEventHandler::onClick(osgEarth::Util::Controls::Control* c)
{
  osg::ref_ptr<SilverLiningConditionPreset> preset;
  if (preset_.lock(preset))
    preset->set(value_);
}

}
