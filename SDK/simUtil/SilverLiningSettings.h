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
#ifndef SIMUTIL_SILVERLININGSETTINGS_H
#define SIMUTIL_SILVERLININGSETTINGS_H

#include <vector>
#include <string>
#include "osg/Referenced"
#include "osgEarthUtil/Controls"
#include "osgEarthSilverLining/SilverLiningAPIWrapper"
#include "osgEarthSilverLining/SilverLiningCallback"
#include "simCore/Common/Common.h"

namespace simVis { class Registry; }

/**
 * @file SilverLiningSettings.h
 *
 * Provides a set of classes useful for manipulating runtime settings of the
 * SilverLining ocean model.  Recommended use is something like:
 *
 * <code>
 *   osg::ref_ptr<simUtil::SilverLiningSettingsAdapter> slSettings(new simUtil::SilverLiningSettingsAdapter);
 *   SkyNode* sky = new SilverLining::SilverLiningNode(scene->getMapNode()->getMapSRS(), opts, slSettings);
 * </code>
 *
 * From there, you can access settings from the SilverLiningSettingsAdapter.  Changes
 * are queued up until you have a valid SilverLining context.
 *
 * You cannot directly access SilverLining object handles through the API, because that
 * means passing those handles across a DLL boundary and that doesn't work, at
 * least in Windows.  Because osgEarth links to SilverLining, and the SIMDIS SDK links to
 * osgEarth and SilverLining, the same SilverLining is linked to twice, creating an issue with
 * handle access.  So all access to the SilverLining context must go through osgEarth.
 * Furthermore, SilverLining can only be accessed when it is active, which is only during
 * the initialization and the draw phases.
 *
 * This set of classes simplifies the access to SilverLining settings by caching values and
 * applying them at valid times.
 */

namespace simUtil {

/**
 * Represents a single variable in SilverLining; maps to a call in osgEarthSilverLining's
 * Environment or Ocean classes.  Abstract class that provides hooks to apply
 * changes to SilverLining at the times at which SilverLining can be modified.  Due to the
 * architecture of SilverLining and osgEarth and the SIMDIS SDK, you can only change
 * settings in the SilverLining environment from inside osgEarth.  This wrapper class
 * makes configuring SilverLining easier by abstracting away the application of the
 * setting value to a single apply_() method that guarantees a valid SilverLining
 * context.
 */
class SDKUTIL_EXPORT SilverLiningValue : public osg::Referenced
{
public:
  /** Ensures the apply_ method is only called when values change. */
  void apply(osgEarth::SilverLining::Atmosphere& atmosphere);

  /**
   * This method is called when SilverLining is initialized.  You can override this method to
   * do work at this time, such as initializing the environment or ocean, or capturing
   * default values from SilverLining.  Default implementation is a no-op.
   */
  virtual void initialize(osgEarth::SilverLining::Atmosphere& atmosphere);

protected:
  /**
   * Call this method to force an apply on the next redraw, e.g. in a set method.
   */
  void setShouldApply_();

  /**
   * Pure virtual method that applies your internal state to the SilverLining environment
   * and ocean values as needed.  Override this method to modify SilverLining when required.
   * This is the template method design pattern's primitive operation.
   */
  virtual void apply_(osgEarth::SilverLining::Atmosphere& atmosphere) = 0;

  /** Protected destructor due to osg::Referenced inheritance */
  virtual ~SilverLiningValue();
  /** Protected constructor, pure abstract class */
  SilverLiningValue();

private:
  bool shouldApply_;
};

/**
 * Internal helper template class to store a copy of a single data type
 * as a private member.  Typename T could be a simple type or complex type.
 * T must be copy constructible and have an assignment and equality operator.
 */
template <typename T>
class SDKUTIL_EXPORT SilverLiningValueT : public SilverLiningValue
{
public:
  /** Returns a reference back to the previously set value. */
  const T& value() const
  {
    return value_;
  }

  /** Changes the value and flags an apply_() on the next draw */
  void set(const T& value, bool forceApply = false)
  {
    // Ignore the set, if the value matches
    if (!forceApply && value_ == value)
      return;
    value_ = value;
    setShouldApply_();
  }

protected:
  /** Construct a new SilverLiningValueT with the given default value. */
  SilverLiningValueT(const T& defaultValue)
    : SilverLiningValue(),
      value_(defaultValue)
  {
  }

  /** Sets the value without forcing an apply */
  void setValue_(const T& value)
  {
    if (&value != &value_)
      value_ = value;
  }

  /** Protected virtual destructor due to osg::Referenced derivation */
  virtual ~SilverLiningValueT()
  {
  }

private:
  T value_;
};

#define DECLARE_SIMPLE_SETTING(SETTING, TYPE) \
class SDKUTIL_EXPORT SETTING : public SilverLiningValueT<TYPE> \
{ \
public: \
  /** Initializes value */ \
  SETTING(); \
  /** Initializes SilverLining to our value */ \
  virtual void initialize(osgEarth::SilverLining::Atmosphere& atmosphere); \
protected: \
  /** osg::Referenced classes should have a protected destructor. */ \
  virtual ~SETTING(); \
  /** Applies the internal value to SilverLining */ \
  virtual void apply_(osgEarth::SilverLining::Atmosphere& atmosphere); \
};

#define DECLARE_SIMPLE_EVTHANDLER(CLASS, SETTING, VALUETYPE) \
/** Control handler for changing double values */ \
class SDKUTIL_EXPORT CLASS : public osgEarth::Util::Controls::ControlEventHandler \
{ \
public: \
  /** Changes value from control */ \
  CLASS(SETTING* value); \
  /** Apply the value to the SilverLiningValue */ \
  virtual void onValueChanged(osgEarth::Util::Controls::Control* c, VALUETYPE value); \
protected: \
  /** osg::Referenced-derived */ \
  virtual ~CLASS(); \
private: \
  osg::observer_ptr<SETTING> value_; \
};


/**
 * Provides a quick way to set up typical weather conditions.  This method will create
 * "infinite" cloud layers that remain centered at the camera, so there's no need to
 * worry about positioning them.  Typical, realistic values for cloud altitudes will
 * be used.  Finer control is possible.  Existing cloud layers in the scene will be
 * removed.  Visibility, precipitation, and wind would be required to be set too.
 */
DECLARE_SIMPLE_SETTING(SilverLiningConditionPreset, int);

/** Provides an on-click method to change condition to a specific preset */
class SDKUTIL_EXPORT SetConditionPresetEventHandler : public osgEarth::Util::Controls::ControlEventHandler
{
public:
  SetConditionPresetEventHandler(SilverLiningConditionPreset* preset, int value);
  virtual void onClick(osgEarth::Util::Controls::Control* c);
protected:
  virtual ~SetConditionPresetEventHandler();
private:
  osg::observer_ptr<SilverLiningConditionPreset> preset_;
  int value_;
};

/**
 * Enable or disable a big, flashy lens flare effect when sun is visible in scene.
 */
DECLARE_SIMPLE_SETTING(SilverLiningLensFlare, bool);
DECLARE_SIMPLE_EVTHANDLER(LensFlareEventHandler, SilverLiningLensFlare, bool);

/**
 * Sets the value for gamma correction of the display.  Defaults to the sky-box-gamma
 * setting from SilverLining config file.  1.8 works well.  Higher values will yield
 * lighter skies and natural light.
 */
DECLARE_SIMPLE_SETTING(SilverLiningGamma, double);
DECLARE_SIMPLE_EVTHANDLER(GammaEventHandler, SilverLiningGamma, double);

/**
 * Simulates an infrared sensor simulator mode.  Just renders everything black except sun.
 */
DECLARE_SIMPLE_SETTING(SilverLiningInfrared, bool);
DECLARE_SIMPLE_EVTHANDLER(InfraredEventHandler, SilverLiningInfrared, bool);

/**
 * Sets physical model for simulating sky colors.  The Preetham model is simple and fast,
 * but has inaccuracies near horizon.  Newer Hosek-Wilkie model extends Preetham for more
 * accurate colors, especially at very high and low solar angles.  Hosek-Wilkie is only
 * used for daytime lighting; twilight and moonlit conditions still use Preetham since
 * Hosek-Wilkie only simulates colors from positive solar angles.
 */
DECLARE_SIMPLE_SETTING(SilverLiningSkyModel, int);
DECLARE_SIMPLE_EVTHANDLER(HosekWilkieToggleEventHandler, SilverLiningSkyModel, bool);

/**
 * Sets simulated visibility in meters; will affect appearance of clouds in distance.
 * Defaults to 30km.  Is intended only for light haze and to blend clouds into sky in
 * distance.  It does not fog the sky itself.
 */
DECLARE_SIMPLE_SETTING(SilverLiningVisibility, double);
DECLARE_SIMPLE_EVTHANDLER(VisibilityEventHandler, SilverLiningVisibility, double);

/**
 * Sets turbidity of atmosphere, a measure of "haziness."  This simulates the number of
 * particles in the air and is not intended for simulating fog.  Higher values vary the
 * color from a pure light blue to a hazy, darker, yellowing color.  Values are clamped
 * between 1.8 and 8.0; 2 is very clear (range 50km), 3 is clear (range 15km), 7 is a
 * light haze (range 8km).
 */
DECLARE_SIMPLE_SETTING(SilverLiningTurbidity, double);
DECLARE_SIMPLE_EVTHANDLER(TurbidityEventHandler, SilverLiningTurbidity, double);

/**
 * Sets simulated nighttime light pollution in watts per square meter.  Default is 0.0.
 * Reasonable values are on the order of 0.01.
 */
DECLARE_SIMPLE_SETTING(SilverLiningLightPollution, double);
DECLARE_SIMPLE_EVTHANDLER(LightPollutionEventHandler, SilverLiningLightPollution, double);

/**
 * Simulates global precipitation.  Precipitation types can be combined.  To clear all
 * precipitation, set each precipitation type to 0.  Precipitation is limited by the
 * appropriate max-intensity values in your SilverLining configuration file.  Reasonable
 * ranges might be between 1.0 for light rain or 20.0 for heavier rain.  A typical
 * default maximum is 30.0.  Value is in millimeters per hour.
 */
DECLARE_SIMPLE_SETTING(SilverLiningRainRate, double);
// Can only do dry or wet snow -- see class SilverLiningSnowRate for a selector
DECLARE_SIMPLE_SETTING(SilverLiningDrySnowRate, double);
DECLARE_SIMPLE_SETTING(SilverLiningWetSnowRate, double);
DECLARE_SIMPLE_SETTING(SilverLiningSleetRate, double);
DECLARE_SIMPLE_EVTHANDLER(RainRateEventHandler, SilverLiningRainRate, double);
DECLARE_SIMPLE_EVTHANDLER(DrySnowRateEventHandler, SilverLiningDrySnowRate, double);
DECLARE_SIMPLE_EVTHANDLER(WetSnowRateEventHandler, SilverLiningWetSnowRate, double);
DECLARE_SIMPLE_EVTHANDLER(SleetRateEventHandler, SilverLiningSleetRate, double);

/**
 * Composite global precipitation setting.  Combines wet and dry snow and lets you
 * choose between which is shown.  Rate is in millimeters per hour.
 */
class SDKUTIL_EXPORT SilverLiningSnowRate : public SilverLiningValue
{
public:
  /** Initializes the default values for the snow. */
  SilverLiningSnowRate();

  /**
   * Retrieves the snow rate in millimeters per hour; 1.0 is light, 20.0 is heavy.
   */
  double rate() const;

  /** Returns true if wet snow, false if dry snow */
  bool isWet() const;

  /** Sets the snow precipitation rate (mm/hr). */
  void setRate(double rate, bool forceApply = false);

  /** Sets whether the snow is wet (true) or dry (false) */
  void setWet(bool isWet, bool forceApply = false);

  /** Always initialize the currently set wind values. */
  virtual void initialize(osgEarth::SilverLining::Atmosphere& atmosphere);

protected:
  /** osg::Referenced classes should have a protected destructor. */
  virtual ~SilverLiningSnowRate();

  /** Applies the snow values to a valid atmosphere */
  virtual void apply_(osgEarth::SilverLining::Atmosphere& atmosphere);

private:
  double rate_;
  bool isWet_;
};

DECLARE_SIMPLE_EVTHANDLER(SnowRateEventHandler, SilverLiningSnowRate, double);
DECLARE_SIMPLE_EVTHANDLER(SnowIsWetEventHandler, SilverLiningSnowRate, bool);

/**
 * Composite setting that manages the wind direction and speed for SilverLining.
 * Only a single wind value is supported in the SilverLiningValue context for
 * simplicity, although SilverLining itself supports multiple winds at different
 * altitudes.  Wind has an influence on cloud formation and rendering of virga.
 */
class SDKUTIL_EXPORT SilverLiningWind : public SilverLiningValue
{
public:
  /** Initializes the default values for the wind. */
  SilverLiningWind();

  /**
   * Retrieve the previously set wind direction in degrees; e.g. 0 is wind blowing
   * from the north, 90 is wind blowing from the east.
   */
  double direction() const;

  /** Retrieves the previously set wind speed in m/s */
  double speed() const;

  /** Sets the wind direction in degrees. */
  void setDirection(double directionDeg, bool forceApply = false);

  /** Sets the wind speed in m/s */
  void setSpeed(double speedMs, bool forceApply = false);

  /** Always initialize the currently set wind values. */
  virtual void initialize(osgEarth::SilverLining::Atmosphere& atmosphere);

protected:
  /** osg::Referenced classes should have a protected destructor. */
  virtual ~SilverLiningWind();

  /** Applies the wind values to a valid atmosphere */
  virtual void apply_(osgEarth::SilverLining::Atmosphere& atmosphere);

private:
  double directionDeg_;
  double speedMs_;
};

DECLARE_SIMPLE_EVTHANDLER(SlWindDirectionDegEventHandler, SilverLiningWind, double);
DECLARE_SIMPLE_EVTHANDLER(SlWindSpeedEventHandler, SilverLiningWind, double);

#undef DECLARE_SIMPLE_SETTING
#undef DECLARE_SIMPLE_EVTHANDLER

//////////////////////////////////////////////////////////////////////////

/** Strategy for determining the SilverLining display time */
class SilverLiningTimeStrategy : public osg::Referenced
{
public:
  /** @see osgEarth::SilverLining::Callback::getMilliseconds() */
  virtual unsigned long getMilliseconds() const = 0;

protected:
  virtual ~SilverLiningTimeStrategy() {}
};

/** Use the default SilverLining time, based on an ever-increasing real-time timer */
class SDKUTIL_EXPORT SLAlwaysRealTime : public SilverLiningTimeStrategy
{
public:
  virtual unsigned long getMilliseconds() const;

protected:
  virtual ~SLAlwaysRealTime() {}
};

/** Use the scenario time as indicated by the simCore Clock, falling back to system clock if undefined */
class SDKUTIL_EXPORT SLRegistryClockTime : public SilverLiningTimeStrategy
{
public:
  virtual unsigned long getMilliseconds() const;

protected:
  virtual ~SLRegistryClockTime() {}
};

/**
 * SilverLining callback that contains a list of variables that may change during the runtime
 * of the display.  Composite class that combines multiple SilverLiningValue instances and
 * distributes the initialization and on-draw functions of osgEarth::SilverLining::Callback.
 */
class SDKUTIL_EXPORT SilverLiningSettingsCallback : public osgEarth::SilverLining::Callback
{
public:
  /** Add the given value to the list of values being tracked by the settings callback. */
  void addValue(simUtil::SilverLiningValue* value);

  /** Removes the given value from the list of values being tracked by the settings callback. */
  void removeValue(simUtil::SilverLiningValue* value);

  /** Changes the time strategy for clouds updates */
  void setTimeStrategy(SilverLiningTimeStrategy* timeStrategy);

  /** Implement the callback to initialize all registered values */
  virtual void onInitialize(osgEarth::SilverLining::Atmosphere& atmosphere);
  /** Implement the callback to apply all variable states */
  virtual void onDrawSky(osgEarth::SilverLining::Atmosphere& atmosphere);
  /** Implement the callback to return an elapsed time for cloud synchronization */
  virtual unsigned long getMilliseconds() const;

protected:
  /** Destroy the callback */
  virtual ~SilverLiningSettingsCallback();

private:
  std::vector<osg::ref_ptr<SilverLiningValue> > values_;
  osg::ref_ptr<SilverLiningTimeStrategy> timeStrategy_;
};

/**
 * Convenience class that adds all currently defined SilverLining settings to a single callback.
 */
class SDKUTIL_EXPORT SilverLiningSettingsAdapter : public SilverLiningSettingsCallback
{
public:
  /** Initializes the settings to default values */
  SilverLiningSettingsAdapter();

  /** Condition preset -- defaults PARTLY_CLOUDY */
  SilverLiningConditionPreset* conditionPreset() const;
  /** Lens flare -- defaults off */
  SilverLiningLensFlare* lensFlare() const;
  /** Gamma -- defaults to 1.6 */
  SilverLiningGamma* gamma() const;
  /** Infrared mode -- defaults off */
  SilverLiningInfrared* infrared() const;
  /** Sky model -- defaults to PREETHAM (0) */
  SilverLiningSkyModel* skyModel() const;
  /** Visibility -- defaults to 30,000 */
  SilverLiningVisibility* visibility() const;
  /** Turbidity -- defaults to 2.2 */
  SilverLiningTurbidity* turbidity() const;
  /** Light Pollution -- defaults to 0.0 w/m^2 */
  SilverLiningLightPollution* lightPollution() const;
  /** Dry snow rate (mm/hr) -- defaults to dry and off (0.0) */
  SilverLiningSnowRate* snowRate() const;
  /** Rain rate (mm/hr) -- defaults to off (0.0) */
  SilverLiningRainRate* rainRate() const;
  /** Sleet rate (mm/hr) -- defaults to off (0.0) */
  SilverLiningSleetRate* sleetRate() const;
  /** Wind -- defaults to 0 m/s from north */
  SilverLiningWind* wind() const;

protected:
  /** osg::Referenced-derived */
  virtual ~SilverLiningSettingsAdapter();

private:
  osg::ref_ptr<SilverLiningConditionPreset> conditionPreset_;
  osg::ref_ptr<SilverLiningLensFlare> lensFlare_;
  osg::ref_ptr<SilverLiningGamma> gamma_;
  osg::ref_ptr<SilverLiningInfrared> infrared_;
  osg::ref_ptr<SilverLiningSkyModel> skyModel_;
  osg::ref_ptr<SilverLiningVisibility> visibility_;
  osg::ref_ptr<SilverLiningTurbidity> turbidity_;
  osg::ref_ptr<SilverLiningLightPollution> lightPollution_;
  osg::ref_ptr<SilverLiningSnowRate> snowRate_;
  osg::ref_ptr<SilverLiningRainRate> rainRate_;
  osg::ref_ptr<SilverLiningSleetRate> sleetRate_;
  osg::ref_ptr<SilverLiningWind> wind_;
};

}

#endif /* SIMUTIL_SILVERLININGSETTINGS_H */
