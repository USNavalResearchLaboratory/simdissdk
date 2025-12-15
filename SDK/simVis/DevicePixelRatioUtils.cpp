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
#include <functional>
#include <string>
#include "osg/Vec3f"
#include "osgText/TextBase"
#include "osgEarth/Registry"
#include "simCore/Calc/Math.h"
#include "simVis/DevicePixelRatioUtils.h"

namespace {

// Strings representing user data values for DPR
inline const std::string DPR_DISABLED = "dpr_disabled";
inline const std::string DPR_TEXT_SIZE = "dpr_size";
inline const std::string DPR_TEXT_POSITION = "dpr_position";
inline const std::string DPR_PERCENT_SUFFIX = "_%";

/** NodeVisitor to recursively update text */
class DprUpscaleVisitor : public osg::NodeVisitor
{
public:
  explicit DprUpscaleVisitor(double devicePixelRatio);

  /** Extracts TextBase */
  virtual void apply(osg::Drawable& drawable) override;

  /** Performs updates on TextBase */
  void applyText(osgText::TextBase& text);

private:
  double scale_ = 1.0;
};

//////////////////////////////////////////////////////////////////////

/**
 * If a node's user value (getUserValue / setUserValue) of `key` is set, return it.
 * Else, initialize it to `defaultValue` and return `defaultValue`.
 * @param node Node on which to operate. Has a user data container that supports
 *    osg::Node::getUserValue<T> for given T.
 * @param key User data container key value for get/setUserValue
 * @param defaultValue If key is unset, set it to defaultValue and return defaultValue,
 *    else return the set value.
 * @return defaultValue if key is unset; else, whatever the user value is set to.
 */
template <typename T>
T getOrCreateUserValue(osg::Node& node, const std::string& key, T defaultValue)
{
  // defaultValue is unchanged when return value is false
  if (!node.getUserValue(key, defaultValue))
    node.setUserValue(key, defaultValue);
  // Return the value in the key
  return defaultValue;
}

/** Similar to simCore::areEqual(), but for osg::Vec3f */
bool areVecEqual(const osg::Vec3f& left, const osg::Vec3f& right, double t = 1e-3)
{
  return simCore::areEqual(left.x(), right.x(), t) &&
    simCore::areEqual(left.y(), right.y(), t) &&
    simCore::areEqual(left.z(), right.z(), t);
}

/** Helper template function for are-equal, for Vec3f and float */
template<typename T>
bool areFloatValuesEqual(const T& a, const T& b)
{
  if constexpr (std::is_same_v<T, float>)
    return simCore::areEqual(a, b);
  else if constexpr (std::is_same_v<T, osg::Vec3f>)
    return areVecEqual(a, b);
  else
  {
#if defined(__cplusplus) && __cplusplus >= 202302L
    static_assert(false, "Unsupported type for DPR equality comparison");
#endif
    return simCore::areEqual(static_cast<double>(a), static_cast<double>(b));
  }
}

/**
 * Workhorse. Template-like method that sets a single value into an object, that gets scaled
 * by the device pixel ratio appropriately. It stores the raw, 100% original value in the
 * userValueKey, then adds the DPR_PERCENT_SUFFIX to store the DPR used to get current value.
 * This is useful later, so we can detect if the user changed the value externally for correct
 * re-scaling cases.
 * @param object TextBase typically
 * @param value Unscaled 100% value to set
 * @param setter Function that takes object and value, to set the scaled value
 * @param userValueKey Key storing information in user values in the object
 */
void setDprScaledValue(auto& object, const auto& value, auto setter, const std::string& userValueKey)
{
  if (simVis::DevicePixelRatioUtils::isDprDisabled(object))
  {
    setter(object, value);
    return;
  }
  const float dpr = osgEarth::Registry::instance()->getDevicePixelRatio();
  const auto scaledValue = value * dpr;
  setter(object, scaledValue);
  object.setUserValue(userValueKey + DPR_PERCENT_SUFFIX, dpr);
  object.setUserValue(userValueKey, value);
}

/**
 * Secondary workhorse, for passive updating based on updated DPR values. Call this on a single node
 * when the DPR changes. This then up-scales the 100% value (stored in setUserValue) based on the
 * new DPR. If the value is not stored, it assumes the current value is supposed to be 100%. And
 * if the actual current value isn't what we expect based on getUserValue values, we assume also
 * that it's a user value at 100% scaling.
 * @param object TextBase typically
 * @param newDpr Device pixel ratio, typically 1.0, 1.5, 2.0, etc.
 * @param getter Function that takes object and returns the currently set value
 * @param setter Function that takes object and value, to set the scaled value
 * @param userValueKey Key storing information in user values in the object
 */
void upscaleToNewRatio(auto& object, float newDpr, auto getter, auto setter, const std::string& userValueKey)
{
  if (simVis::DevicePixelRatioUtils::isDprDisabled(object))
    return;

  const std::string dprPixelRatio = userValueKey + DPR_PERCENT_SUFFIX;
  const float lastSetDpr = getOrCreateUserValue(object, dprPixelRatio, 1.f);
  const auto currentValue = getter(object);
  const auto unscaledValue = getOrCreateUserValue(object, userValueKey, currentValue);

  // Need to split logic here. Either we know what the current value is (based on last-set
  // values), in which case we can scale up the original. Or if it doesn't match, that means
  // an end user changed the value without updating the last-set, which means we upscale
  // their given value. TODO: Alternatively, we could down-scale it to get the 1.0 value
  const auto expectedCurrent = unscaledValue * lastSetDpr;
  if (areFloatValuesEqual(expectedCurrent, currentValue))
  {
    // The user hasn't changed anything; we upscale
    setter(object, unscaledValue * newDpr);
  }
  else
  {
    // The user has changed something. Upscale it based on DPR and save base value
    const auto newUnscaledValue = currentValue;
    object.setUserValue(userValueKey, newUnscaledValue);
    setter(object, newUnscaledValue * newDpr);
  }
  // Always save off the DPR
  object.setUserValue(dprPixelRatio, newDpr);
}

//////////////////////////////////////////////////////////////////////

DprUpscaleVisitor::DprUpscaleVisitor(double devicePixelRatio)
  : NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
  scale_(devicePixelRatio)
{
}

void DprUpscaleVisitor::apply(osg::Drawable& drawable)
{
  if (osgText::TextBase* text = dynamic_cast<osgText::TextBase*>(&drawable))
    applyText(*text);
  traverse(drawable);
}

void DprUpscaleVisitor::applyText(osgText::TextBase& text)
{
  upscaleToNewRatio(text, scale_,
    [](const auto& obj) { return obj.getCharacterHeight(); },
    [](auto& obj, const auto& val) { obj.setCharacterSize(val); },
    DPR_TEXT_SIZE);
  upscaleToNewRatio(text, scale_,
    [](const auto& obj) { return obj.getPosition(); },
    [](auto& obj, const auto& val) { obj.setPosition(val); },
    DPR_TEXT_POSITION);
}

}

//////////////////////////////////////////////////////////////////////

namespace simVis {

void DevicePixelRatioUtils::setDprDisabled(osg::Node& node)
{
  node.setUserValue(DPR_DISABLED, true);
}

bool DevicePixelRatioUtils::isDprDisabled(osg::Node& node)
{
  if (!node.getUserDataContainer())
    return false;
  // Must have the value DPR_DISABLED and it must be set to true
  bool isDisabled = false;
  return node.getUserValue(DPR_DISABLED, isDisabled) && isDisabled;
}

void DevicePixelRatioUtils::setTextCharacterSize(osgText::TextBase& text, float characterSize)
{
  setDprScaledValue(text, characterSize,
    [](auto& obj, const auto& val) { obj.setCharacterSize(val); },
    DPR_TEXT_SIZE);
}

void DevicePixelRatioUtils::setTextPosition(osgText::TextBase& text, const osg::Vec3f& position)
{
  setDprScaledValue(text, position,
    [](auto& obj, const auto& val) { obj.setPosition(val); },
    DPR_TEXT_POSITION);
}

void DevicePixelRatioUtils::updateScenePixelRatio(osg::Node& root)
{
  DprUpscaleVisitor scaler(osgEarth::Registry::instance()->getDevicePixelRatio());
  root.accept(scaler);
}

}
