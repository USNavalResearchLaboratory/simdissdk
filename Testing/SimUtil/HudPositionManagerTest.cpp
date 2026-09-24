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
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "osg/MatrixTransform"
#include "osg/ref_ptr"
#include "simCore/Common/SDKAssert.h"
#include "simUtil/HudPositionManager.h"

namespace {

/** Registered callback that records position and alignment-offset calls. */
class TestRepositionCallback : public simUtil::HudPositionManager::RepositionCallback
{
public:
  void setPosition(const std::string& name, const osg::Vec2d& positionPct) override
  {
    ++numCalls_;
    lastName_ = name;
    lastPosition_ = positionPct;
  }

  void setAlignmentOffsetPx(const std::string& name, const osg::Vec2d& offsetPx) override
  {
    ++numAlignmentOffsetCalls_;
    lastAlignmentOffsetName_ = name;
    lastAlignmentOffsetPx_ = offsetPx;
  }

  void alignmentOverrideChanged(const std::string& name, std::optional<simUtil::HudWindowAlignment> alignmentOverride) override
  {
    ++numAlignmentOverrideCalls_;
    lastAlignmentOverrideName_ = name;
    lastAlignmentOverride_ = alignmentOverride;
  }

  void placementReset(const std::string& name) override
  {
    ++numPlacementResetCalls_;
    lastPlacementResetName_ = name;
  }

  unsigned int numCalls() const
  {
    return numCalls_;
  }

  unsigned int numAlignmentOffsetCalls() const
  {
    return numAlignmentOffsetCalls_;
  }

  const std::string& lastName() const
  {
    return lastName_;
  }

  const std::string& lastAlignmentOffsetName() const
  {
    return lastAlignmentOffsetName_;
  }

  const osg::Vec2d& lastPosition() const
  {
    return lastPosition_;
  }

  const osg::Vec2d& lastAlignmentOffsetPx() const
  {
    return lastAlignmentOffsetPx_;
  }

  unsigned int numAlignmentOverrideCalls() const
  {
    return numAlignmentOverrideCalls_;
  }

  unsigned int numPlacementResetCalls() const
  {
    return numPlacementResetCalls_;
  }

  const std::string& lastAlignmentOverrideName() const
  {
    return lastAlignmentOverrideName_;
  }

  std::optional<simUtil::HudWindowAlignment> lastAlignmentOverride() const
  {
    return lastAlignmentOverride_;
  }

  const std::string& lastPlacementResetName() const
  {
    return lastPlacementResetName_;
  }

protected:
  virtual ~TestRepositionCallback() = default;

private:
  unsigned int numCalls_ = 0;
  unsigned int numAlignmentOffsetCalls_ = 0;
  unsigned int numAlignmentOverrideCalls_ = 0;
  unsigned int numPlacementResetCalls_ = 0;
  std::string lastName_;
  std::string lastAlignmentOffsetName_;
  std::string lastAlignmentOverrideName_;
  std::string lastPlacementResetName_;
  osg::Vec2d lastPosition_;
  osg::Vec2d lastAlignmentOffsetPx_;
  std::optional<simUtil::HudWindowAlignment> lastAlignmentOverride_;
};
int testUnknownWindow()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;

  osg::Vec2d position;
  rv += SDK_ASSERT(hud->getPosition("Unknown", position) != 0);
  osg::Vec2d minSize;
  osg::Vec2d maxSize;
  rv += SDK_ASSERT(hud->getSize("Unknown", minSize, maxSize) != 0);
  rv += SDK_ASSERT(!hud->getAlignment("Unknown").has_value());
  rv += SDK_ASSERT(hud->removeWindow("Unknown") != 0);

  return rv;
}

int testRegistrationPositionAndSize()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<TestRepositionCallback> callback = new TestRepositionCallback;

  const std::string name = "Test Window";
  const osg::Vec2d defaultPosition(0.25, 0.75);
  hud->addWindow(name, defaultPosition, callback.get());

  // Registration applies the default position through the callback.
  rv += SDK_ASSERT(callback->numCalls() == 1);
  rv += SDK_ASSERT(callback->lastName() == name);
  rv += SDK_ASSERT(callback->lastPosition() == defaultPosition);

  osg::Vec2d position;
  rv += SDK_ASSERT(hud->getPosition(name, position) == 0);
  rv += SDK_ASSERT(position == defaultPosition);

  const osg::Vec2d minSize(-10.0, -20.0);
  const osg::Vec2d maxSize(30.0, 40.0);
  rv += SDK_ASSERT(hud->setSize(name, minSize, maxSize) == 0);

  osg::Vec2d actualMinSize;
  osg::Vec2d actualMaxSize;
  rv += SDK_ASSERT(hud->getSize(name, actualMinSize, actualMaxSize) == 0);
  rv += SDK_ASSERT(actualMinSize == minSize);
  rv += SDK_ASSERT(actualMaxSize == maxSize);

  const osg::Vec2d newPosition(0.5, 0.6);
  rv += SDK_ASSERT(hud->setPosition(name, newPosition) == 0);
  rv += SDK_ASSERT(callback->numCalls() == 2);
  rv += SDK_ASSERT(callback->lastPosition() == newPosition);

  // Setting an unchanged position does not emit another callback.
  rv += SDK_ASSERT(hud->setPosition(name, newPosition) == 0);
  rv += SDK_ASSERT(callback->numCalls() == 2);

  rv += SDK_ASSERT(hud->resetPosition(name) == 0);
  rv += SDK_ASSERT(callback->numCalls() == 3);
  rv += SDK_ASSERT(callback->lastPosition() == defaultPosition);

  return rv;
}

int testAlignment()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<TestRepositionCallback> callback = new TestRepositionCallback;

  const std::string name = "Aligned Window";
  hud->addWindow(name, osg::Vec2d(0.1, 0.2), callback.get());

  // Newly registered windows retain legacy behavior until an alignment is selected.
  rv += SDK_ASSERT(!hud->getAlignment(name).has_value());

  rv += SDK_ASSERT(hud->setAlignment(
    name,
    simUtil::HudWindowAlignment::LEFT_TOP) == 0);

  std::optional<simUtil::HudWindowAlignment> alignment = hud->getAlignment(name);
  rv += SDK_ASSERT(alignment.has_value());
  rv += SDK_ASSERT(*alignment == simUtil::HudWindowAlignment::LEFT_TOP);

  // Clearing the optional returns to no user-selected alignment.
  rv += SDK_ASSERT(hud->setAlignment(name, std::nullopt) == 0);
  rv += SDK_ASSERT(!hud->getAlignment(name).has_value());

  return rv;
}

int testCachedStateAndRemoveReadd()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;

  const std::string name = "Future Window";
  const osg::Vec2d savedPosition(0.15, 0.85);
  const osg::Vec2d defaultPosition(0.8, 0.2);

  // Position and alignment can be loaded before the actual HUD window exists.
  rv += SDK_ASSERT(hud->setPosition(name, savedPosition) == 0);
  rv += SDK_ASSERT(hud->setAlignment(name,
    simUtil::HudWindowAlignment::RIGHT_BOTTOM) == 0);

  osg::ref_ptr<TestRepositionCallback> firstCallback = new TestRepositionCallback;
  hud->addWindow(name, defaultPosition, firstCallback.get());

  // Cached position is preferred over the registration default.
  rv += SDK_ASSERT(firstCallback->numCalls() == 1);
  rv += SDK_ASSERT(firstCallback->lastPosition() == savedPosition);

  std::optional<simUtil::HudWindowAlignment> alignment = hud->getAlignment(name);
  rv += SDK_ASSERT(alignment.has_value());
  rv += SDK_ASSERT(*alignment == simUtil::HudWindowAlignment::RIGHT_BOTTOM);

  // Reset returns to the registration default, not the cached position.
  rv += SDK_ASSERT(hud->resetPosition(name) == 0);
  rv += SDK_ASSERT(firstCallback->lastPosition() == defaultPosition);

  // Removing a window retains its stored state but disables its callback.
  rv += SDK_ASSERT(hud->removeWindow(name) == 0);
  const unsigned int numCallsBeforeSet = firstCallback->numCalls();
  rv += SDK_ASSERT(hud->setPosition(name, savedPosition) == 0);
  rv += SDK_ASSERT(firstCallback->numCalls() == numCallsBeforeSet);

  // Re-registration restores the retained position and alignment.
  osg::ref_ptr<TestRepositionCallback> secondCallback = new TestRepositionCallback;
  hud->addWindow(name, defaultPosition, secondCallback.get());
  rv += SDK_ASSERT(secondCallback->numCalls() == 1);
  rv += SDK_ASSERT(secondCallback->lastPosition() == savedPosition);

  alignment = hud->getAlignment(name);
  rv += SDK_ASSERT(alignment.has_value());
  rv += SDK_ASSERT(*alignment == simUtil::HudWindowAlignment::RIGHT_BOTTOM);

  std::vector<std::string> names;
  rv += SDK_ASSERT(hud->getAllWindows(names, true) == 0);
  rv += SDK_ASSERT(names.size() == 1);
  rv += SDK_ASSERT(names.front() == name);

  return rv;
}

int testInitialOriginPosition()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<TestRepositionCallback> callback = new TestRepositionCallback;

  // Regression test: initial (0,0) position must still notify the callback.
  hud->addWindow("Origin Window", osg::Vec2d(), callback.get());

  rv += SDK_ASSERT(callback->numCalls() == 1);
  rv += SDK_ASSERT(callback->lastName() == "Origin Window");
  rv += SDK_ASSERT(callback->lastPosition() == osg::Vec2d());

  return rv;
}

int testAlignmentOffsets()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<TestRepositionCallback> callback = new TestRepositionCallback;

  const std::string name = "Offset Window";
  const osg::Vec2d minSize(-100.0, -50.0);
  const osg::Vec2d maxSize(0.0, 0.0);

  hud->addWindow(name, osg::Vec2d(0.2, 0.3), callback.get());
  rv += SDK_ASSERT(hud->setSize(name, minSize, maxSize) == 0);

  const unsigned int positionCalls = callback->numCalls();

  const auto checkAlignment = [&rv, &hud, &callback, &name, &minSize, &maxSize, positionCalls](
    simUtil::HudWindowAlignment alignment,
    const osg::Vec2d& expectedOffset)
    {
      const unsigned int offsetCalls = callback->numAlignmentOffsetCalls();
      const unsigned int overrideCalls = callback->numAlignmentOverrideCalls();

      rv += SDK_ASSERT(hud->setAlignment(name, alignment) == 0);
      rv += SDK_ASSERT(callback->numCalls() == positionCalls);
      rv += SDK_ASSERT(callback->numAlignmentOffsetCalls() == offsetCalls + 1);
      rv += SDK_ASSERT(callback->lastAlignmentOffsetName() == name);
      rv += SDK_ASSERT(callback->lastAlignmentOffsetPx() == expectedOffset);

      const osg::Vec2d managerOffset = hud->getAlignmentOffsetPx(name);
      rv += SDK_ASSERT(managerOffset == expectedOffset);

      rv += SDK_ASSERT(callback->numAlignmentOverrideCalls() == overrideCalls + 1);
      rv += SDK_ASSERT(callback->lastAlignmentOverrideName() == name);
      const auto lastOverride = callback->lastAlignmentOverride();
      rv += SDK_ASSERT(lastOverride.has_value());
      rv += SDK_ASSERT(*lastOverride == alignment);

      // Alignment changes must not modify the native bounds or their signs.
      osg::Vec2d actualMinSize;
      osg::Vec2d actualMaxSize;
      rv += SDK_ASSERT(hud->getSize(name, actualMinSize, actualMaxSize) == 0);
      rv += SDK_ASSERT(actualMinSize == minSize);
      rv += SDK_ASSERT(actualMaxSize == maxSize);
    };

  checkAlignment(simUtil::HudWindowAlignment::LEFT_TOP, osg::Vec2d(100.0, 0.0));
  checkAlignment(simUtil::HudWindowAlignment::LEFT_CENTER, osg::Vec2d(100.0, 25.0));
  checkAlignment(simUtil::HudWindowAlignment::LEFT_BOTTOM, osg::Vec2d(100.0, 50.0));

  checkAlignment(simUtil::HudWindowAlignment::CENTER_TOP, osg::Vec2d(50.0, 0.0));
  checkAlignment(simUtil::HudWindowAlignment::CENTER_CENTER, osg::Vec2d(50.0, 25.0));
  checkAlignment(simUtil::HudWindowAlignment::CENTER_BOTTOM, osg::Vec2d(50.0, 50.0));

  checkAlignment(simUtil::HudWindowAlignment::RIGHT_TOP, osg::Vec2d(0.0, 0.0));
  checkAlignment(simUtil::HudWindowAlignment::RIGHT_CENTER, osg::Vec2d(0.0, 25.0));
  checkAlignment(simUtil::HudWindowAlignment::RIGHT_BOTTOM, osg::Vec2d(0.0, 50.0));

  // Setting an unchanged alignment does not emit another offset callback.
  const unsigned int unchangedAlignmentCalls = callback->numAlignmentOffsetCalls();
  const unsigned int unchangedOverrideCalls = callback->numAlignmentOverrideCalls();
  rv += SDK_ASSERT(hud->setAlignment(name, simUtil::HudWindowAlignment::RIGHT_BOTTOM) == 0);
  rv += SDK_ASSERT(callback->numAlignmentOffsetCalls() == unchangedAlignmentCalls);
  rv += SDK_ASSERT(callback->numAlignmentOverrideCalls() == unchangedOverrideCalls);

  // Dynamic bounds changes update the offset without generating a position callback.
  rv += SDK_ASSERT(hud->setAlignment(name, simUtil::HudWindowAlignment::LEFT_TOP) == 0);
  rv += SDK_ASSERT(callback->numAlignmentOverrideCalls() == unchangedOverrideCalls + 1);
  const unsigned int offsetCalls = callback->numAlignmentOffsetCalls();

  rv += SDK_ASSERT(hud->setSize(name, osg::Vec2d(-250.0, -50.0), maxSize) == 0);
  rv += SDK_ASSERT(callback->numCalls() == positionCalls);
  rv += SDK_ASSERT(callback->numAlignmentOffsetCalls() == offsetCalls + 1);
  rv += SDK_ASSERT(callback->lastAlignmentOffsetPx() == osg::Vec2d(250.0, 0.0));

  rv += SDK_ASSERT(hud->setAlignment(name, std::nullopt) == 0);
  rv += SDK_ASSERT(callback->numAlignmentOverrideCalls() == unchangedOverrideCalls + 2);
  rv += SDK_ASSERT(callback->numCalls() == positionCalls);
  rv += SDK_ASSERT(callback->lastAlignmentOffsetPx() == osg::Vec2d());
  rv += SDK_ASSERT(!callback->lastAlignmentOverride().has_value());

  const osg::Vec2d clearedManagerOffset = hud->getAlignmentOffsetPx(name);
  rv += SDK_ASSERT(clearedManagerOffset == osg::Vec2d());

  // Clearing an already-clear override is also a no-op.
  const unsigned int clearedAlignmentCalls = callback->numAlignmentOffsetCalls();
  rv += SDK_ASSERT(hud->setAlignment(name, std::nullopt) == 0);
  rv += SDK_ASSERT(callback->numAlignmentOffsetCalls() == clearedAlignmentCalls);

  return rv;
}

int testAlignmentBeforeRegistration()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<TestRepositionCallback> callback = new TestRepositionCallback;

  const std::string name = "Future Aligned Window";
  const osg::Vec2d defaultPosition(0.8, 0.2);

  // This creates WindowData without setting a position.
  rv += SDK_ASSERT(hud->setAlignment(
    name,
    simUtil::HudWindowAlignment::RIGHT_BOTTOM) == 0);

  hud->addWindow(name, defaultPosition, callback.get());

  // Registration must apply its default position to an alignment-only record.
  rv += SDK_ASSERT(callback->numCalls() == 1);
  rv += SDK_ASSERT(callback->lastPosition() == defaultPosition);

  const auto alignment = hud->getAlignment(name);
  rv += SDK_ASSERT(alignment.has_value());
  rv += SDK_ASSERT(*alignment == simUtil::HudWindowAlignment::RIGHT_BOTTOM);

  // Default alignment can also create a future window record. Registration
  // must later apply its normal default position.
  const std::string defaultOnlyName = "Future Default Alignment Window";
  const osg::Vec2d defaultOnlyPosition(0.3, 0.7);

  rv += SDK_ASSERT(hud->setDefaultAlignment(defaultOnlyName, simUtil::HudWindowAlignment::LEFT_TOP) == 0);
  rv += SDK_ASSERT(hud->getDefaultAlignment(defaultOnlyName).has_value());
  rv += SDK_ASSERT(*hud->getDefaultAlignment(defaultOnlyName) == simUtil::HudWindowAlignment::LEFT_TOP);

  rv += SDK_ASSERT(hud->getAlignment(defaultOnlyName).has_value());
  rv += SDK_ASSERT(*hud->getAlignment(defaultOnlyName) == simUtil::HudWindowAlignment::LEFT_TOP);

  osg::ref_ptr<TestRepositionCallback> defaultOnlyCallback = new TestRepositionCallback;
  hud->addWindow(defaultOnlyName, defaultOnlyPosition, defaultOnlyCallback.get());
  rv += SDK_ASSERT(defaultOnlyCallback->numCalls() == 1);
  rv += SDK_ASSERT(defaultOnlyCallback->lastPosition() == defaultOnlyPosition);

  return rv;
}

int testMatrixCallbackAlignmentOffset()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
  osg::ref_ptr<simUtil::RepositionMatrixPxCallback> callback =
    new simUtil::RepositionMatrixPxCallback(xform.get());

  const std::string name = "Matrix Window";
  hud->addWindow(name, osg::Vec2d(), callback.get());
  rv += SDK_ASSERT(hud->setSize(
    name,
    osg::Vec2d(-100.0, -50.0),
    osg::Vec2d()) == 0);

  rv += SDK_ASSERT(hud->setAlignment(
    name,
    simUtil::HudWindowAlignment::LEFT_TOP) == 0);

  osg::Vec3d translation = xform->getMatrix().getTrans();
  rv += SDK_ASSERT(translation.x() == 100.0);
  rv += SDK_ASSERT(translation.y() == 0.0);

  rv += SDK_ASSERT(hud->setAlignment(
    name,
    simUtil::HudWindowAlignment::LEFT_BOTTOM) == 0);

  translation = xform->getMatrix().getTrans();
  rv += SDK_ASSERT(translation.x() == 100.0);
  rv += SDK_ASSERT(translation.y() == 50.0);

  rv += SDK_ASSERT(hud->setAlignment(name, std::nullopt) == 0);

  translation = xform->getMatrix().getTrans();
  rv += SDK_ASSERT(translation.x() == 0.0);
  rv += SDK_ASSERT(translation.y() == 0.0);

  return rv;
}

int testAlignmentOffsetBeforePosition()
{
  int rv = 0;

  osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
  osg::ref_ptr<simUtil::RepositionMatrixPxCallback> callback =
    new simUtil::RepositionMatrixPxCallback(xform.get());

  const std::string name = "Offset Before Position";

  // Storing an offset before a position should not move the transform yet.
  callback->setAlignmentOffsetPx(name, osg::Vec2d(10.0, 20.0));

  osg::Vec3d translation = xform->getMatrix().getTrans();
  rv += SDK_ASSERT(translation.x() == 0.0);
  rv += SDK_ASSERT(translation.y() == 0.0);

  // The later position update should apply the previously stored offset.
  // No viewport has been observed in this unit test, so percentage position
  // converts to (0,0) pixels and the resulting translation is the offset.
  callback->setPosition(name, osg::Vec2d());

  translation = xform->getMatrix().getTrans();
  rv += SDK_ASSERT(translation.x() == 10.0);
  rv += SDK_ASSERT(translation.y() == 20.0);

  // A later offset replaces the original offset and reapplies immediately.
  callback->setAlignmentOffsetPx(name, osg::Vec2d(30.0, 40.0));

  translation = xform->getMatrix().getTrans();
  rv += SDK_ASSERT(translation.x() == 30.0);
  rv += SDK_ASSERT(translation.y() == 40.0);

  return rv;
}

int testDefaultAlignmentAndPlacementReset()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<TestRepositionCallback> callback = new TestRepositionCallback;

  const std::string name = "Default Alignment Window";
  const osg::Vec2d defaultPosition(0.8, 0.9);
  const osg::Vec2d movedPosition(0.2, 0.3);

  hud->addWindow(name, defaultPosition, callback.get());
  rv += SDK_ASSERT(hud->setSize(name, osg::Vec2d(-100.0, -50.0), osg::Vec2d()) == 0);

  // Default alignment takes effect when no override exists.
  rv += SDK_ASSERT(hud->setDefaultAlignment(name, simUtil::HudWindowAlignment::RIGHT_TOP) == 0);
  rv += SDK_ASSERT(!hud->getAlignmentOverride(name).has_value());

  // Setting an unchanged default alignment is a no-op.
  const unsigned int unchangedDefaultCalls = callback->numAlignmentOffsetCalls();
  rv += SDK_ASSERT(hud->setDefaultAlignment(name, simUtil::HudWindowAlignment::RIGHT_TOP) == 0);
  rv += SDK_ASSERT(callback->numAlignmentOffsetCalls() == unchangedDefaultCalls);

  rv += SDK_ASSERT(hud->getDefaultAlignment(name).has_value());
  rv += SDK_ASSERT(*hud->getDefaultAlignment(name) == simUtil::HudWindowAlignment::RIGHT_TOP);

  rv += SDK_ASSERT(hud->getAlignment(name).has_value());
  rv += SDK_ASSERT(*hud->getAlignment(name) == simUtil::HudWindowAlignment::RIGHT_TOP);

  // Override the default, then move the window.
  rv += SDK_ASSERT(hud->setAlignment(name, simUtil::HudWindowAlignment::LEFT_TOP) == 0);
  rv += SDK_ASSERT(hud->getAlignmentOverride(name).has_value());
  rv += SDK_ASSERT(*hud->getAlignmentOverride(name) == simUtil::HudWindowAlignment::LEFT_TOP);
  rv += SDK_ASSERT(hud->setPosition(name, movedPosition) == 0);
  rv += SDK_ASSERT(*hud->getAlignment(name) == simUtil::HudWindowAlignment::LEFT_TOP);

  // Changing the default while an override exists must preserve the override and avoid applying a new offset
  const unsigned int defaultChangeWithOverrideCalls = callback->numAlignmentOffsetCalls();
  rv += SDK_ASSERT(hud->setDefaultAlignment(name, simUtil::HudWindowAlignment::CENTER_TOP) == 0);
  rv += SDK_ASSERT(callback->numAlignmentOffsetCalls() == defaultChangeWithOverrideCalls);
  rv += SDK_ASSERT(*hud->getDefaultAlignment(name) == simUtil::HudWindowAlignment::CENTER_TOP);
  rv += SDK_ASSERT(*hud->getAlignment(name) == simUtil::HudWindowAlignment::LEFT_TOP);

  // Position-only reset preserves the alignment override.
  const unsigned int placementResetCallsBeforePositionReset = callback->numPlacementResetCalls();
  rv += SDK_ASSERT(hud->resetPosition(name) == 0);
  // Position-only reset does not emit the placement-reset notification.
  rv += SDK_ASSERT(callback->numPlacementResetCalls() == placementResetCallsBeforePositionReset);
  rv += SDK_ASSERT(*hud->getAlignmentOverride(name) == simUtil::HudWindowAlignment::LEFT_TOP);

  osg::Vec2d position;
  rv += SDK_ASSERT(hud->getPosition(name, position) == 0);
  rv += SDK_ASSERT(position == defaultPosition);
  rv += SDK_ASSERT(*hud->getAlignment(name) == simUtil::HudWindowAlignment::LEFT_TOP);

  // Clearing the alignment override restores the default alignment.
  rv += SDK_ASSERT(hud->setAlignment(name, std::nullopt) == 0);
  rv += SDK_ASSERT(*hud->getAlignment(name) == simUtil::HudWindowAlignment::CENTER_TOP);
  rv += SDK_ASSERT(!hud->getAlignmentOverride(name).has_value());

  // Reset placement restores both values together.
  rv += SDK_ASSERT(hud->setAlignment(name, simUtil::HudWindowAlignment::LEFT_TOP) == 0);
  rv += SDK_ASSERT(hud->setPosition(name, movedPosition) == 0);

  const unsigned int placementResetCallsBeforeReset = callback->numPlacementResetCalls();
  rv += SDK_ASSERT(hud->resetPlacement(name) == 0);
  rv += SDK_ASSERT(callback->numPlacementResetCalls() == placementResetCallsBeforeReset + 1);
  rv += SDK_ASSERT(callback->lastPlacementResetName() == name);

  rv += SDK_ASSERT(hud->getPosition(name, position) == 0);
  rv += SDK_ASSERT(position == defaultPosition);
  rv += SDK_ASSERT(*hud->getAlignment(name) == simUtil::HudWindowAlignment::CENTER_TOP);

  rv += SDK_ASSERT(hud->setAlignment(name, simUtil::HudWindowAlignment::LEFT_TOP) == 0);
  rv += SDK_ASSERT(hud->setPosition(name, movedPosition) == 0);

  const unsigned int placementResetCallsBeforeAllReset = callback->numPlacementResetCalls();
  hud->resetAllPlacements();
  rv += SDK_ASSERT(callback->numPlacementResetCalls() == placementResetCallsBeforeAllReset + 1);

  rv += SDK_ASSERT(hud->getPosition(name, position) == 0);
  rv += SDK_ASSERT(position == defaultPosition);

  const auto resetAllAlignment = hud->getAlignment(name);
  rv += SDK_ASSERT(resetAllAlignment.has_value());
  rv += SDK_ASSERT(*resetAllAlignment == simUtil::HudWindowAlignment::CENTER_TOP);

  return rv;
}

int testAllowedAlignments()
{
  int rv = 0;
  osg::ref_ptr<simUtil::HudPositionManager> hud = new simUtil::HudPositionManager;
  osg::ref_ptr<TestRepositionCallback> callback = new TestRepositionCallback;

  const std::string name = "Restricted Alignment Window";
  hud->addWindow(name, osg::Vec2d(0.8, 0.9), callback.get());

  // Newly registered windows allow all nine alignments.
  rv += SDK_ASSERT(hud->getAllowedAlignments(name).size() == 9);

  const std::set<simUtil::HudWindowAlignment> topAlignments = {
    simUtil::HudWindowAlignment::LEFT_TOP,
    simUtil::HudWindowAlignment::CENTER_TOP,
    simUtil::HudWindowAlignment::RIGHT_TOP
  };

  // Default get/set
  rv += SDK_ASSERT(hud->setAllowedAlignments(name, topAlignments) == 0);
  rv += SDK_ASSERT(hud->getAllowedAlignments(name) == topAlignments);
  // Setting an unchanged allowed-alignment set is a no-op.
  rv += SDK_ASSERT(hud->setAllowedAlignments(name, topAlignments) == 0);
  rv += SDK_ASSERT(hud->getAllowedAlignments(name) == topAlignments);

  // Valid default/regular alignments succeed
  rv += SDK_ASSERT(hud->setDefaultAlignment(name, simUtil::HudWindowAlignment::RIGHT_TOP) == 0);
  // Unsupported default alignments are rejected.
  rv += SDK_ASSERT(hud->setDefaultAlignment(name, simUtil::HudWindowAlignment::LEFT_CENTER) != 0);
  rv += SDK_ASSERT(*hud->getDefaultAlignment(name) == simUtil::HudWindowAlignment::RIGHT_TOP);
  rv += SDK_ASSERT(hud->setAlignment(name, simUtil::HudWindowAlignment::LEFT_TOP) == 0);

  // Unsupported alignments are rejected and leave the current override intact.
  rv += SDK_ASSERT(hud->setAlignment(name, simUtil::HudWindowAlignment::LEFT_CENTER) != 0);

  const auto override = hud->getAlignmentOverride(name);
  rv += SDK_ASSERT(override.has_value());
  rv += SDK_ASSERT(*override == simUtil::HudWindowAlignment::LEFT_TOP);

  // Empty restrictions are invalid.
  rv += SDK_ASSERT(hud->setAllowedAlignments(name, {}) != 0);
  // Invalid enum values are rejected.
  const auto invalidAlignment = static_cast<simUtil::HudWindowAlignment>(999);
  rv += SDK_ASSERT(hud->setAllowedAlignments(name, { invalidAlignment }) != 0);

  // Cannot remove the current default or override from the allowed set.
  rv += SDK_ASSERT(hud->setAllowedAlignments(name, { simUtil::HudWindowAlignment::RIGHT_TOP }) != 0);

  // Clear the override, leaving only the RIGHT_TOP default.
  rv += SDK_ASSERT(hud->setAlignment(name, std::nullopt) == 0);
  // This set retains LEFT_TOP but excludes the current RIGHT_TOP default.
  rv += SDK_ASSERT(hud->setAllowedAlignments(name, { simUtil::HudWindowAlignment::LEFT_TOP }) != 0);
  // Failed updates leave the previous allowed set unchanged.
  rv += SDK_ASSERT(hud->getAllowedAlignments(name) == topAlignments);

  // Confirm default fallback explicitly
  const auto effectiveAlignment = hud->getAlignment(name);
  rv += SDK_ASSERT(effectiveAlignment.has_value());
  rv += SDK_ASSERT(*effectiveAlignment == simUtil::HudWindowAlignment::RIGHT_TOP);

  return rv;
}

}

int HudPositionManagerTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testUnknownWindow() == 0);
  rv += SDK_ASSERT(testRegistrationPositionAndSize() == 0);
  rv += SDK_ASSERT(testAlignment() == 0);
  rv += SDK_ASSERT(testCachedStateAndRemoveReadd() == 0);
  rv += SDK_ASSERT(testInitialOriginPosition() == 0);
  rv += SDK_ASSERT(testAlignmentOffsets() == 0);
  rv += SDK_ASSERT(testAlignmentBeforeRegistration() == 0);
  rv += SDK_ASSERT(testMatrixCallbackAlignmentOffset() == 0);
  rv += SDK_ASSERT(testAlignmentOffsetBeforePosition() == 0);
  rv += SDK_ASSERT(testDefaultAlignmentAndPlacementReset() == 0);
  rv += SDK_ASSERT(testAllowedAlignments() == 0);

  return rv;
}
