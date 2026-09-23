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
#include <string>
#include <vector>

#include "osg/ref_ptr"
#include "simCore/Common/SDKAssert.h"
#include "simUtil/HudPositionManager.h"

namespace {

/** Registered callback that records setPosition() calls and number of times it's called */
class TestRepositionCallback : public simUtil::HudPositionManager::RepositionCallback
{
public:
  void setPosition(const std::string& name, const osg::Vec2d& positionPct) override
  {
    ++numCalls_;
    lastName_ = name;
    lastPosition_ = positionPct;
  }

  unsigned int numCalls() const
  {
    return numCalls_;
  }

  const std::string& lastName() const
  {
    return lastName_;
  }

  const osg::Vec2d& lastPosition() const
  {
    return lastPosition_;
  }

protected:
  virtual ~TestRepositionCallback() = default;

private:
  unsigned int numCalls_ = 0;
  std::string lastName_;
  osg::Vec2d lastPosition_;
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

}

int HudPositionManagerTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testUnknownWindow() == 0);
  rv += SDK_ASSERT(testRegistrationPositionAndSize() == 0);
  rv += SDK_ASSERT(testAlignment() == 0);
  rv += SDK_ASSERT(testCachedStateAndRemoveReadd() == 0);

  return rv;
}
