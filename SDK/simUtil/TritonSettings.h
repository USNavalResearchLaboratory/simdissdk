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
#ifndef SIMUTIL_TRITONSETTINGS_H
#define SIMUTIL_TRITONSETTINGS_H

#include <vector>
#include <string>
#include "osg/Referenced"
#include "osgEarth/Controls"
#include "osgEarthTriton/TritonAPIWrapper"
#include "osgEarthTriton/TritonCallback"
#include "simCore/Common/Common.h"

/**
 * @file TritonSettings.h
 *
 * Provides a set of classes useful for manipulating runtime settings of the
 * Triton ocean model.  Recommended use is something like:
 *
 * <code>
 *   osg::ref_ptr<simUtil::TritonSettingsAdapter> tritonSettings(new simUtil::TritonSettingsAdapter);
 *   TritonLayer* layer = new Triton::TritonLayer(opts, tritonSettings);
 *   scene->getMap()->addLayer(layer);
 * </code>
 *
 * From there, you can access settings from the TritonSettingsAdapter.  Changes
 * are queued up until you have a valid Triton context.
 *
 * You cannot directly access Triton object handles through the API, because that
 * means passing those handles across a DLL boundary and that doesn't work, at
 * least in Windows.  Because osgEarth links to Triton, and the SIMDIS SDK links to
 * osgEarth and Triton, the same Triton is linked to twice, creating an issue with
 * handle access.  So all access to the Triton context must go through osgEarth.
 * Furthermore, Triton can only be accessed when it is active, which is only during
 * the initialization and the draw phases.
 *
 * This set of classes simplifies the access to Triton settings by caching values and
 * applying them at valid times.
 */

namespace simUtil {

/**
 * Represents a single variable in Triton; maps to a call in osgEarthTriton's
 * Environment or Ocean classes.  Abstract class that provides hooks to apply
 * changes to Triton at the times at which Triton can be modified.  Due to the
 * architecture of Triton and osgEarth and the SIMDIS SDK, you can only change
 * settings in the Triton environment from inside osgEarth.  This wrapper class
 * makes configuring Triton easier by abstracting away the application of the
 * setting value to a single apply_() method that guarantees a valid Triton
 * context.
 */
class SDKUTIL_EXPORT TritonValue : public osg::Referenced
{
public:
  /** Ensures the apply_ method is only called when values change. */
  void apply(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean);

  /**
   * This method is called when Triton is initialized.  You can override this method to
   * do work at this time, such as initializing the environment or ocean, or capturing
   * default values from Triton.  Default implementation is a no-op.
   */
  virtual void initialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean);

protected:
  /**
   * Call this method to force an apply on the next redraw, e.g. in a set method.
   */
  void setShouldApply_();

  /**
   * Pure virtual method that applies your internal state to the Triton environment
   * and ocean values as needed.  Override this method to modify Triton when required.
   * This is the template method design pattern's primitive operation.
   */
  virtual void apply_(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean) = 0;

  /** Protected destructor due to osg::Referenced inheritance */
  virtual ~TritonValue();
  /** Protected constructor, pure abstract class */
  TritonValue();

private:
  bool shouldApply_;
};

/**
 * Internal helper template class to store a copy of a single data type
 * as a private member.  Typename T could be a simple type or complex type.
 * T must be copy constructible and have an assignment and equality operator.
 */
template <typename T>
class SDKUTIL_EXPORT TritonValueT : public TritonValue
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
  /** Construct a new TritonValueT with the given default value. */
  TritonValueT(const T& defaultValue)
    : TritonValue(),
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
  virtual ~TritonValueT()
  {
  }

private:
  T value_;
};

#define DECLARE_SIMPLE_SETTING(SETTING, TYPE) \
class SDKUTIL_EXPORT SETTING : public TritonValueT<TYPE> \
{ \
public: \
  /** Initializes value */ \
  SETTING(); \
  /** Initializes Triton to our value */ \
  virtual void initialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean); \
protected: \
  /** osg::Referenced classes should have a protected destructor. */ \
  virtual ~SETTING(); \
  /** Applies the internal value to Triton */ \
  virtual void apply_(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean); \
};

/**
 * Triton value that manages ocean choppiness, which controls how peaked the waves are.
 * The value 0.0 yields no chop, 3.0 yields strong chop. Values that are too high may
 * result in wave geometry folding over itself, so take care to set reasonable values.
 */
DECLARE_SIMPLE_SETTING(TritonChoppiness, double);

/**
 * Triton value that manages intensity of sunlight visible at the ocean surface.  Modulates
 * specular highlights of the sun on water surface.  Normally 1.0, but could be decreased
 * if, for example, sun is obscured by clouds.
 */
DECLARE_SIMPLE_SETTING(TritonSunIntensity, double);

/**
 * Enables or disables spray particle effects on breaking waves.  This does incur a
 * performance penalty, so disabling spray effects can improve performance.
 */
DECLARE_SIMPLE_SETTING(TritonEnableSpray, bool);

/**
 * Enables or disables wireframe rendering of the ocean's mesh.
 */
DECLARE_SIMPLE_SETTING(TritonEnableWireframe, bool);

/**
 * Turns the underwater crepuscular rays effect on and off.  Has no impact if
 * underwater-god-rays-enabled is disabled in Triton.config.  Defaults off.
 */
DECLARE_SIMPLE_SETTING(TritonEnableGodRays, bool);

/**
 * Fades out the underwater crepuscular rays effect by specified amount (0.0 = no fading,
 * 1.0 = completely faded)
 */
DECLARE_SIMPLE_SETTING(TritonGodRaysFade, double);

/**
 * Changes the rendering quality of Triton.  Unlike other Triton values, this one cannot be
 * changed while Triton is active, and must be set prior to initialization.  Higher quality
 * will result in finer wave resolution, but at lower performance.  The default value is GOOD.
 */
DECLARE_SIMPLE_SETTING(TritonQuality, osgEarth::Triton::OceanQuality)

#undef DECLARE_SIMPLE_SETTING

/**
 * Composite setting that manages the wind direction and sea state.  Simulates a specific sea
 * state on the Beaufort scale.  See http://en.wikipedia.org/wiki/Beaufort_scale for detailed
 * descriptions of Beaufort numbers and the wave conditions they specify. At a high level:
 *       0:  Calm
 *       1:  Light air
 *       2:  Light breeze
 *       3:  Gentle breeze
 *       4:  Moderate breeze
 *       5:  Fresh breeze
 *       6:  Strong breeze
 *       7:  High wind
 *       8:  Gale
 *       9:  Storm
 *       10: Strong Storm
 *       11: Violent Storm
 *       12: Hurricane
 */
class SDKUTIL_EXPORT TritonSeaState : public TritonValue
{
public:
  /** Initializes the default values for the sea state. */
  TritonSeaState();

  /**
   * Retrieve the previously set wind direction in radians; e.g. 0 is wind blowing
   * from the north, M_PI_2 is wind blowing from the east.
   */
  double windDirection() const;

  /** Retrieves the previously set sea state on the Beaufort scale */
  double seaState() const;

  /** Sets the wind direction in radians. */
  void setWindDirection(double windDirectionRad, bool forceApply = false);

  /** Sets the sea state on the Beaufort scale (from 0 to 12) */
  void setSeaState(double seaState, bool forceApply = false);

  /** Always initialize the currently set sea state. */
  virtual void initialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean);

protected:
  /** osg::Referenced classes should have a protected destructor. */
  virtual ~TritonSeaState();

  /** Applies the sea state to a valid environment and ocean */
  virtual void apply_(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean);

private:
  double seaState_;
  double windDirectionRad_;
};

/**
 * Triton callback that contains a list of variables that may change during the runtime
 * of the display.  Composite class that combines multiple TritonValue instances and
 * distributes the initialization and on-draw functions of osgEarth::Triton::Callback.
 */
class SDKUTIL_EXPORT TritonSettingsCallback : public osgEarth::Triton::Callback
{
public:
  /** Add the given value to the list of values being tracked by the settings callback. */
  void addValue(simUtil::TritonValue* value);

  /** Removes the given value from the list of values being tracked by the settings callback. */
  void removeValue(simUtil::TritonValue* value);

  /** Implement the callback to initialize all registered values */
  virtual void onInitialize(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean);
  /** Implement the callback to apply all variable states */
  virtual void onDrawOcean(osgEarth::Triton::Environment& env, osgEarth::Triton::Ocean& ocean);

protected:
  /** Destroy the callback */
  virtual ~TritonSettingsCallback();

private:
  std::vector<osg::ref_ptr<TritonValue> > values_;
};

/**
 * Convenience class that adds all currently defined Triton settings to a single callback.
 */
class SDKUTIL_EXPORT TritonSettingsAdapter : public TritonSettingsCallback
{
public:
  /** Initializes the settings to default values */
  TritonSettingsAdapter();

  /** Sea choppiness -- typically 0 to 3, default of 1.6 */
  TritonChoppiness* choppiness() const;
  /** Display quality -- defaults to GOOD */
  TritonQuality* quality() const;
  /** Sea state -- defaults to 4.0 and wind from north. */
  TritonSeaState* seaState() const;
  /** Sun intensity -- typically 1.0, but can be decreased based on cloud cover. */
  TritonSunIntensity* sunIntensity() const;
  /** Spray -- turn on and off ocean spray */
  TritonEnableSpray* enableSpray() const;
  /** Wireframe -- turn on and off wireframe rendering */
  TritonEnableWireframe* enableWireframe() const;
  /** God rays -- turn on and off crepuscular rays effect */
  TritonEnableGodRays* enableGodRays() const;
  /** God ray fade -- Change fade amount of god rays (0.0 for no fading, 1.0 completely faded */
  TritonGodRaysFade* godRaysFade() const;

protected:
  /** osg::Referenced-derived */
  virtual ~TritonSettingsAdapter();

private:
  osg::ref_ptr<TritonChoppiness> choppiness_;
  osg::ref_ptr<TritonQuality> quality_;
  osg::ref_ptr<TritonSeaState> seaState_;
  osg::ref_ptr<TritonSunIntensity> sunIntensity_;
  osg::ref_ptr<TritonEnableSpray> enableSpray_;
  osg::ref_ptr<TritonEnableWireframe> enableWireframe_;
  osg::ref_ptr<TritonEnableGodRays> enableGodRays_;
  osg::ref_ptr<TritonGodRaysFade> godRaysFade_;
};

//////////////////////////////////////////////////////////////////////

#define DECLARE_SIMPLE_EVTHANDLER(CLASS, SETTING, VALUETYPE) \
/** Control handler for changing double values */ \
class SDKUTIL_EXPORT CLASS : public osgEarth::Util::Controls::ControlEventHandler \
{ \
public: \
  /** Changes value from control */ \
  CLASS(SETTING* value); \
  /** Apply the value to the TritonValue */ \
  virtual void onValueChanged(osgEarth::Util::Controls::Control* c, VALUETYPE value); \
protected: \
  /** osg::Referenced-derived */ \
  virtual ~CLASS(); \
private: \
  osg::observer_ptr<SETTING> value_; \
};

DECLARE_SIMPLE_EVTHANDLER(ChoppinessEventHandler, TritonChoppiness, double);
DECLARE_SIMPLE_EVTHANDLER(SunIntensityEventHandler, TritonSunIntensity, double);
DECLARE_SIMPLE_EVTHANDLER(EnableSprayEventHandler, TritonEnableSpray, bool);
DECLARE_SIMPLE_EVTHANDLER(EnableWireframeEventHandler, TritonEnableWireframe, bool);
DECLARE_SIMPLE_EVTHANDLER(EnableGodRaysEventHandler, TritonEnableGodRays, bool);
DECLARE_SIMPLE_EVTHANDLER(GodRaysFadeEventHandler, TritonGodRaysFade, double);
DECLARE_SIMPLE_EVTHANDLER(WindDirectionDegEventHandler, TritonSeaState, double);
DECLARE_SIMPLE_EVTHANDLER(SeaStateEventHandler, TritonSeaState, double);
DECLARE_SIMPLE_EVTHANDLER(QualityEventHandler, TritonQuality, double);

#undef DECLARE_SIMPLE_EVTHANDLER

/** When attached to a Quality slider, will update the label provided with Quality text */
class SDKUTIL_EXPORT QualityTextUpdater : public osgEarth::Util::Controls::ControlEventHandler
{
public:
  /** Changes value from control */
  QualityTextUpdater(osgEarth::Util::Controls::LabelControl* label);
  /** Apply the value to the TritonValue */
  virtual void onValueChanged(osgEarth::Util::Controls::Control* c, double value);

protected:
  /** osg::Referenced-derived */
  virtual ~QualityTextUpdater();

private:
  osg::observer_ptr<osgEarth::Util::Controls::LabelControl> label_;
};

}

#endif /* SIMUTIL_TRITONSETTINGS_H */
