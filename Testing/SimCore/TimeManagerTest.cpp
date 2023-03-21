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
#include <algorithm>
#include <deque>
#include <set>
#include <cmath>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Time.h"
#include "simCore/Calc/Math.h"
#include "simCore/Time/Utils.h"
#include "simCore/Time/Clock.h"
#include "simCore/Time/ClockImpl.h"

namespace
{

bool almostEqual(const simCore::TimeStamp& a, const simCore::TimeStamp& b)
{
  static const double EPSILON = 1e-3;
  return fabs(a - b) < EPSILON;
}

class TestTimeObserver : public simCore::Clock::TimeObserver
{
public:
  TestTimeObserver()
  : expectLoop_(false),
    errorCount_(0)
  {
  }

  void setExpectLoop()
  {
    expectLoop_ = true;
  }

  void addExpectTime(const simCore::TimeStamp &t, bool expectJump)
  {
    // silently drop set's to the same time
    bool add = true;
    if (!expectTime_.empty())
    {
      simCore::TimeStamp f = expectTime_.back();
      add = !almostEqual(f, t);
    }

    if (add)
    {
      expectTime_.push_back(t);
      expectJump_.push_back(expectJump);
    }
  }

  unsigned errorCount() const
  {
    return errorCount_;
  }

  // from Notify
  /// time has been changed
  virtual void onSetTime(const simCore::TimeStamp &t, bool isJump)
  {
    SDK_ASSERT(!expectTime_.empty());
    SDK_ASSERT(!expectJump_.empty());
    if (expectTime_.empty() || expectJump_.empty())
    {
      std::cout << "Expect underflow" << std::endl;
      ++errorCount_;
      return;
    }

    simCore::TimeStamp f = expectTime_.front();
    expectTime_.pop_front();
    bool expectJump = expectJump_.front();
    expectJump_.pop_front();

    if (!almostEqual(f, t))
      std::cout << "Expected time " << f.secondsSinceRefYear(1970).Double() << " got time " << t.secondsSinceRefYear(1970).Double() << std::endl;
    errorCount_ += SDK_ASSERT(almostEqual(f, t));
    errorCount_ += SDK_ASSERT(expectJump == isJump);
  }

  /// time has looped
  virtual void onTimeLoop()
  {
    errorCount_ += SDK_ASSERT(expectLoop_);
    expectLoop_ = false;
  }

  virtual void adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
  { // no-op
  }

protected:
  std::deque<simCore::TimeStamp> expectTime_;
  std::deque<bool> expectJump_;
  bool expectLoop_;
  unsigned errorCount_;
};

int observerTest()
{
  int rv = 0;
  simCore::Clock *t = new simCore::ClockImpl;
  TestTimeObserver* obs = new TestTimeObserver;
  simCore::Clock::TimeObserverPtr observer(obs);
  t->registerTimeCallback(observer);

  // start time should default to simCore::MIN_TIME_STAMP
  simCore::TimeStamp tmp = t->startTime();
  rv += SDK_ASSERT(tmp == simCore::MIN_TIME_STAMP);

  // end time should default to simCore::INFINITE_TIME_STAMP
  tmp = t->endTime();
  rv += SDK_ASSERT(tmp == simCore::INFINITE_TIME_STAMP);

  // currentTime should default to simCore::MIN_TIME_STAMP
  tmp = t->currentTime();
  rv += SDK_ASSERT(tmp == simCore::MIN_TIME_STAMP);

  rv += SDK_ASSERT(t->canLoop() == true); // loop defaults to true

  // shift the boundaries
  // setStartTime will also setTime to begin time
  obs->addExpectTime(simCore::TimeStamp(1970, 1.0), true); // setTime, jump
  t->setStartTime(simCore::TimeStamp(1970, 1.0));

  // there will be no setTime, so no addExpectTime
  t->setEndTime(simCore::TimeStamp(1970, 10.0));

  // current time already set to start time, so no set will occur, so no addExpectTime
  t->setTime(simCore::TimeStamp(1970, 1.0));

  // add
  obs->addExpectTime(simCore::TimeStamp(1970, 2.0), false); // step, no jump
  t->setTimeScale(1.0);
  t->stepForward();

  // subtract
  obs->addExpectTime(simCore::TimeStamp(1970, 1.5), false); // step, no jump
  t->setTimeScale(0.5);
  t->stepBackward();

  // wrapping
  std::cout << "Beginning wrap tests" << std::endl;
  obs->addExpectTime(simCore::TimeStamp(1970, 9.0), true); // set time, jump
  t->setTime(simCore::TimeStamp(1970, 9.0));
  //9.0 to 10.0
  obs->addExpectTime(simCore::TimeStamp(1970, 10.0), false); // step forward, no jump
  t->setTimeScale(1.5);
  t->stepForward();
  // 10 to 1.0
  obs->addExpectTime(simCore::TimeStamp(1970, 1.0), true); // step forward, jump
  obs->setExpectLoop();
  t->setTimeScale(1.5);
  t->stepForward();

  std::cout << "Wrap Begin to End" << std::endl;
  obs->addExpectTime(simCore::TimeStamp(1970, 2.5), true); // set time, jump
  t->setTime(simCore::TimeStamp(1970, 2.5)); // begin to end
  obs->setExpectLoop();
  obs->addExpectTime(simCore::TimeStamp(1970, 1.0), false); // loop with step backward, no jump
  t->setTimeScale(2.0);
  t->stepBackward();
  obs->addExpectTime(simCore::TimeStamp(1970, 10.0), true); // loop with step backward, jump
  t->stepBackward();
  obs->addExpectTime(simCore::TimeStamp(1970, 8.0), false); // loop with step backward, no jump
  t->stepBackward();

  std::cout << "Setup for wrap blocked" << std::endl;
  t->setCanLoop(false); // prevent
  obs->addExpectTime(simCore::TimeStamp(1970, 9.0), true); // set time, jump
  t->setTime(simCore::TimeStamp(1970, 9.0));
  std::cout << "Block test" << std::endl;
  obs->addExpectTime(simCore::TimeStamp(1970, 10.0), false); // step forward
  t->setTimeScale(2.0);
  t->stepForward(); // end to begin
  std::cout << "Final time: " << t->currentTime().secondsSinceRefYear().Double() << std::endl;
  rv += obs->errorCount();

  t->removeTimeCallback(observer);
  delete t;
  return rv;
}

/// Empty callbacks
class Empty : public simCore::Clock::ModeChangeObserver
{
public:
  virtual void onModeChange(simCore::Clock::Mode newMode) {}
  virtual void onDirectionChange(simCore::TimeDirection newDirection) {}
  virtual void onScaleChange(double newValue) {}
  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end) {}
  virtual void onCanLoopChange(bool newVal) {}
  virtual void onUserEditableChanged(bool userCanEdit) {}
};

/// A class that removes some else during a mode change
class RemoveSomeone : public simCore::Clock::ModeChangeObserver
{
public:
  RemoveSomeone(simCore::ClockImpl& t, std::shared_ptr<Empty> empty)
    : t_(t),
      empty_(empty)
  {
  }
  virtual void onModeChange(simCore::Clock::Mode newMode) { t_.removeModeChangeCallback(empty_); }
  virtual void onDirectionChange(simCore::TimeDirection newDirection) {}
  virtual void onScaleChange(double newValue) {}
  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end) {}
  virtual void onCanLoopChange(bool newVal) {}
  virtual void onUserEditableChanged(bool userCanEdit) {}
private:
  simCore::ClockImpl& t_;
  std::shared_ptr<Empty> empty_;
};

/// A class that adds some else during a mode change
class AddSomeone : public simCore::Clock::ModeChangeObserver
{
public:
  AddSomeone(simCore::ClockImpl& t, std::shared_ptr<Empty> empty)
    : t_(t),
    empty_(empty)
  {
  }
  virtual void onModeChange(simCore::Clock::Mode newMode) { t_.registerModeChangeCallback(empty_); }
  virtual void onDirectionChange(simCore::TimeDirection newDirection) {}
  virtual void onScaleChange(double newValue) {}
  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end) {}
  virtual void onCanLoopChange(bool newVal) {}
  virtual void onUserEditableChanged(bool userCanEdit) {}
private:
  simCore::ClockImpl& t_;
  std::shared_ptr<Empty> empty_;
};

int modeObserverTest()
{
  // Make sure the code does not crash when an observer removes an observer

  simCore::ClockImpl t;
  t.registerModeChangeCallback(std::make_shared<Empty>());
  auto empty = std::make_shared<Empty>();
  t.registerModeChangeCallback(empty);
  t.registerModeChangeCallback(std::make_shared<RemoveSomeone>(t, empty));
  t.registerModeChangeCallback(std::make_shared<Empty>());
  empty = std::make_shared<Empty>();
  t.registerModeChangeCallback(std::make_shared<AddSomeone>(t, empty));
  t.registerModeChangeCallback(std::make_shared<Empty>());
  t.setMode(simCore::Clock::MODE_FREEWHEEL);

  return 0;
}

int stepTest()
{
  int rv = 0;
  simCore::ClockImpl clock;
  clock.setStartTime(simCore::TimeStamp(1970, 5.0));
  clock.setEndTime(simCore::TimeStamp(1970, 30.0));
  clock.setMode(simCore::Clock::MODE_STEP);
  rv += SDK_ASSERT(clock.currentTime() >= simCore::TimeStamp(1970, 5.0));
  rv += SDK_ASSERT(clock.currentTime() <= simCore::TimeStamp(1970, 30.0));
  clock.setTime(simCore::TimeStamp(1970, 10.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.0));
  // Step mode should default to a scale of 0.1
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  clock.setMode(simCore::Clock::MODE_REALTIME);
  // Realtime mode should default to a scale of 0.1
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 1.0));
  clock.setMode(simCore::Clock::MODE_STEP);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  clock.decreaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.05));
  clock.decreaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.01));
  clock.increaseScale(); // 0.05
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.05));
  clock.increaseScale(); // 0.1
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  clock.increaseScale(); // 0.25
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.25));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.25));
  // Test clamp
  clock.setTime(simCore::TimeStamp(1970, 40));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 30.0));
  clock.setTime(simCore::TimeStamp(1970, 1));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 5.0));
  // Test wrap
  clock.setTimeScale(2.0);
  clock.setTime(simCore::TimeStamp(1970, 28));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 30.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 5.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 7.0));
  clock.setTime(simCore::TimeStamp(1970, 29));
  clock.stepForward();
  // Clamp to the end before wrapping
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 30.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 5.0));

  // Test playing forward
  clock.setTimeScale(0.1);
  clock.setTime(simCore::TimeStamp(1970, 10));
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(!clock.isPlaying());
  clock.playForward();
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.0));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  // Advance frames, checking the direction and time as we go
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.1));
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  clock.decreaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.05));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.15));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  clock.increaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);

  // Now test stepForward while playing forward
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.25));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  rv += SDK_ASSERT(clock.isPlaying());
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.35));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::STOP);
  rv += SDK_ASSERT(!clock.isPlaying());
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  // Continue playing, make sure we're still going forward
  clock.playForward();
  clock.idle();
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.45));

  // Now test stepBackward while playing forward
  clock.stepBackward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.35));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::STOP);
  rv += SDK_ASSERT(!clock.isPlaying());
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  // Continue playing, make sure we're still going forward
  clock.playForward();
  clock.idle();
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.45));

  // Test multiple stops
  rv += SDK_ASSERT(clock.isPlaying());
  clock.stop();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(!clock.isPlaying());
  clock.stop();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(!clock.isPlaying());

  // If we're this far, then forward playing has passed.  Do same tests for playing backward
  clock.setTimeScale(0.1);
  clock.setTime(simCore::TimeStamp(1970, 10));
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(!clock.isPlaying());
  clock.playReverse();
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.0));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  // Advance frames, checking the direction and time as we go
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.9));
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  clock.decreaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.05));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.85));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  clock.increaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);

  // Now test stepForward while playing backward
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.75));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  rv += SDK_ASSERT(clock.isPlaying());
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.85));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::STOP);
  rv += SDK_ASSERT(!clock.isPlaying());
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  // Continue playing, make sure we're still going forward
  clock.playReverse();
  clock.idle();
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.75));

  // Now test stepBackward while playing forward
  clock.stepBackward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.65));
  rv += SDK_ASSERT(clock.timeDirection() == simCore::STOP);
  rv += SDK_ASSERT(!clock.isPlaying());
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  // Continue playing, make sure we're still going forward
  clock.playReverse();
  clock.idle();
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.55));

  // Test stop from reverse
  rv += SDK_ASSERT(clock.isPlaying());
  clock.stop();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(!clock.isPlaying());

  // Swap to realtime
  clock.playReverse();
  clock.setMode(simCore::Clock::MODE_REALTIME);
  rv += SDK_ASSERT(clock.mode() == simCore::Clock::MODE_REALTIME);
  rv += SDK_ASSERT(!simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.55));
  clock.setMode(simCore::Clock::MODE_STEP);
  rv += SDK_ASSERT(clock.mode() == simCore::Clock::MODE_STEP);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.55));
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.45));
  clock.stop();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 9.45));

  return rv;
}

int realtimeTest()
{
  int rv = 0;
  simCore::ClockImpl clock;
  clock.setStartTime(simCore::TimeStamp(1970, 0.0));
  clock.setEndTime(simCore::TimeStamp(1970, 100000.0)); // very large end time
  clock.setCanLoop(false);

  // Make sure that setting the scale in realtime doesn't affect the scale in step mode
  rv += SDK_ASSERT(clock.mode() == simCore::Clock::MODE_STEP);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  clock.setMode(simCore::Clock::MODE_REALTIME);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 1.0));
  clock.increaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 2.0));
  clock.decreaseScale();
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 1.0));
  clock.setTimeScale(5.0);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 5.0));
  clock.setRealTime(false);
  rv += SDK_ASSERT(clock.mode() == simCore::Clock::MODE_STEP);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 0.1));
  clock.setRealTime(true);
  rv += SDK_ASSERT(clock.mode() == simCore::Clock::MODE_REALTIME);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 5.0));

  // Step forward and back and make sure those work correctly
  clock.setTime(simCore::TimeStamp(1970, 15.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 15.0));
  clock.stepBackward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 10.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 15.0));

  // Attempt to play forwards
  rv += SDK_ASSERT(clock.timeDirection() == simCore::STOP);
  rv += SDK_ASSERT(!clock.isPlaying());
  clock.playForward();
  rv += SDK_ASSERT(clock.isPlaying());
  // Validate that time doesn't update until after we idle()
  Sleep(1);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 15.0));
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() > simCore::TimeStamp(1970, 15.0));
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 5.0));
  // Reset the time and make sure we're still playing
  clock.setTime(simCore::TimeStamp(1970, 15.0));
  rv += SDK_ASSERT(clock.isPlaying());

  // Increment the time step and verify we're still playing
  clock.setTimeScale(25.0);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 25.0));
  rv += SDK_ASSERT(clock.isPlaying());
  Sleep(1);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 15.0));
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() > simCore::TimeStamp(1970, 15.0));

  // Decrement time step and verify we're still playing
  clock.setTime(simCore::TimeStamp(1970, 15.0));
  clock.decreaseScale();
  Sleep(1);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 15.0));
  clock.idle();
  simCore::TimeStamp timeStamp = clock.currentTime();
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(timeStamp > simCore::TimeStamp(1970, 15.0));
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 24.0));
  // Step forward and ensure our new time is what we expect
  clock.stepForward();
  rv += SDK_ASSERT(!clock.isPlaying());
  simCore::TimeStamp laterTime1 = timeStamp + 24.0;  // 24 instead of 25 because of decreaseScale() above
  simCore::TimeStamp laterTime2 = clock.currentTime();
  rv += SDK_ASSERT(laterTime1 == laterTime2);
  // Step back and ensure we're still fine too
  clock.stepBackward();
  rv += SDK_ASSERT(!clock.isPlaying());
  rv += SDK_ASSERT(timeStamp == clock.currentTime());
  // Idle and make sure it didn't change
  rv += SDK_ASSERT(clock.currentTime() == timeStamp);
  Sleep(1);
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == timeStamp);

  // Do some playing backwards
  clock.stop();
  clock.setTime(simCore::TimeStamp(1970, 1500.0));
  clock.setRealTime(true);
  clock.setTimeScale(10.0);
  clock.playReverse();
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 10.0));
  // Idle and make sure we're less than we started
  Sleep(1);
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() < simCore::TimeStamp(1970, 1500.0));

  // Update time, decrease step, and try again
  clock.setTime(simCore::TimeStamp(1970, 1500.0));
  clock.decreaseScale();
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 9.0));
  Sleep(1);
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() < simCore::TimeStamp(1970, 1500.0));

  // Update time, increase step, and try again
  clock.setTime(simCore::TimeStamp(1970, 1500.0));
  clock.increaseScale();
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(clock.timeDirection() == simCore::REVERSE);
  rv += SDK_ASSERT(simCore::areEqual(clock.timeScale(), 10.0));
  Sleep(1);
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() < simCore::TimeStamp(1970, 1500.0));

  // SIM-12714 - test that scale resets to 1 when entering MODE_FREEWHEEL
  rv += SDK_ASSERT(clock.timeScale() != 1.0);
  clock.setMode(simCore::Clock::MODE_FREEWHEEL, simCore::TimeStamp(1970, 15.0));
  rv += SDK_ASSERT(clock.timeScale() == 1.0);

  return rv;
}

int freewheelTest()
{
  int rv = 0;
  simCore::ClockImpl clock;
  clock.setStartTime(simCore::TimeStamp(1970, 0.0));
  clock.setEndTime(simCore::TimeStamp(1970, 100000.0)); // very large end time
  clock.setCanLoop(false);
  clock.setTime(simCore::TimeStamp(1970, 5.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 5.0));
  rv += SDK_ASSERT(!clock.isPlaying());
  clock.setMode(simCore::Clock::MODE_FREEWHEEL, simCore::TimeStamp(1970, 15.0));
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1970, 15.0));

  // Idle the clock and play forward
  Sleep(1);
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() > simCore::TimeStamp(1970, 15.0));
  // Make sure we cannot stop the clock
  rv += SDK_ASSERT(clock.isPlaying());
  clock.stop();
  rv += SDK_ASSERT(clock.isPlaying());
  // We can't play in reverse
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  clock.playReverse();
  rv += SDK_ASSERT(clock.isPlaying());
  rv += SDK_ASSERT(clock.timeDirection() == simCore::FORWARD);
  // It should be realtime
  rv += SDK_ASSERT(clock.realTime());
  // Step forward/reverse should not work
  simCore::TimeStamp newTime = clock.currentTime();
  clock.stepForward();
  rv += SDK_ASSERT(newTime == clock.currentTime());
  clock.stepBackward();
  rv += SDK_ASSERT(newTime == clock.currentTime());
  rv += SDK_ASSERT(clock.isPlaying());

  // Test that we can move to a different time frame with setMode()
  clock.setMode(simCore::Clock::MODE_FREEWHEEL, simCore::TimeStamp(1972, 0.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(1972, 0.0));
  rv += SDK_ASSERT(clock.isPlaying());

  // Test swapping to a step mode
  clock.setRealTime(false);
  rv += SDK_ASSERT(!clock.isPlaying());
  rv += SDK_ASSERT(clock.currentTime() <= clock.endTime());
  rv += SDK_ASSERT(clock.currentTime() >= clock.startTime());
  rv += SDK_ASSERT(clock.mode() == simCore::Clock::MODE_STEP);
  newTime = clock.currentTime();
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == newTime);

  // test simulation mode, which forces start time to be minimum possible time stamp and end time to be maximum possible time stamp
  clock.setMode(simCore::Clock::MODE_SIMULATION, simCore::TimeStamp(1970, 25.0));

  // check that begin and end time are not changed by passed in values
  rv += SDK_ASSERT(clock.startTime() < simCore::TimeStamp(1970, 25.0));
  rv += SDK_ASSERT(clock.endTime() > simCore::TimeStamp(1970, 25.0));
  clock.setEndTime(simCore::TimeStamp(2035, 25.0));
  rv += SDK_ASSERT(clock.endTime() > simCore::TimeStamp(2035, 25.0));

  // move forward in time with simulation, past the specified end time
  clock.setTime(simCore::TimeStamp(2035, 26.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2035, 26.0));
  clock.setTime(simCore::TimeStamp(2035, 36.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2035, 36.0));

  // move backwards in time with simulation, prior to specified start time
  clock.setStartTime(simCore::TimeStamp(2012, 44.0));
  clock.setTime(simCore::TimeStamp(2012, 43.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2012, 43.0));
  clock.setTime(simCore::TimeStamp(2012, 42.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2012, 42.0));

  return rv;
}

int simulationTest()
{
  int rv = 0;
  simCore::ClockImpl clock1;
  clock1.setMode(simCore::Clock::MODE_SIMULATION, simCore::TimeStamp(1970, 5.0));
  rv += SDK_ASSERT(clock1.currentTime() == simCore::TimeStamp(1970, 5.0));

  // Validate that we can idle the clock and time doesn't move
  clock1.idle();
  rv += SDK_ASSERT(clock1.currentTime() == simCore::TimeStamp(1970, 5.0));
  Sleep(1);
  clock1.idle();
  rv += SDK_ASSERT(clock1.currentTime() == simCore::TimeStamp(1970, 5.0));

  // Set up a second clock for the next test
  simCore::ClockImpl clock2;
  clock2.setMode(simCore::Clock::MODE_SIMULATION, simCore::TimeStamp(1970, 5.0));

  // Verify that we can set a time scale and the clock will start to idle
  const double now1 = simCore::getSystemTime();
  clock2.setTimeScale(50.0);
  clock1.setTimeScale(1.0);
  Sleep(1);
  clock2.idle();
  clock1.idle();
  // Time should be greater than the 5.0 time, but less than 5.0 + (now-then)
  const double now2 = simCore::getSystemTime();
  const double timeSinceSet = now2 - now1;
  rv += SDK_ASSERT(clock1.currentTime() > simCore::TimeStamp(1970, 5.0));
  rv += SDK_ASSERT(clock2.currentTime() > simCore::TimeStamp(1970, 5.0));
  rv += SDK_ASSERT(clock1.currentTime() <= simCore::TimeStamp(1970, 5.0 + timeSinceSet));
  rv += SDK_ASSERT(clock2.currentTime() <= simCore::TimeStamp(1970, 5.0 + timeSinceSet * 50.0));
  // Because clock2 was at a scale of 50, it should be higher than the time on clock1 even though it updated first
  rv += SDK_ASSERT(clock2.currentTime() > clock1.currentTime());

  // Setting the time to 25 should work
  clock1.setTime(simCore::TimeStamp(1970, 25.0));
  rv += SDK_ASSERT(clock1.currentTime() == simCore::TimeStamp(1970, 25.0));
  // Sleep and idle and we should get a higher time
  Sleep(1);
  clock1.idle();
  rv += SDK_ASSERT(clock1.currentTime() > simCore::TimeStamp(1970, 25.0));

  // Reset and make sure that scale of 0 still works
  clock1.setTime(simCore::TimeStamp(1970, 45.0));
  clock1.setTimeScale(0.0);
  rv += SDK_ASSERT(clock1.currentTime() == simCore::TimeStamp(1970, 45.0));
  // Sleep and idle and we should get a lower time
  Sleep(1);
  clock1.idle();
  rv += SDK_ASSERT(clock1.currentTime() == simCore::TimeStamp(1970, 45.0));

  return rv;
}

/// An observer to stop time at the given times
class AdjustTimeObserver : public simCore::Clock::TimeObserver
{
public:
  AdjustTimeObserver()
    : breakCount_(0),
      callbackCount_(0)
  {
  }

  void addExpectedPauseTime(const simCore::TimeStamp &t)
  {
    pauseTimes_.insert(t);
  }

  unsigned int breakCount(bool clearCount = true)
  {
    unsigned int rv = breakCount_;
    if (clearCount)
      breakCount_ = 0;
    return rv;
  }

  unsigned int callbackCount(bool clearCount = true)
  {
    unsigned int rv = callbackCount_;
    if (clearCount)
      callbackCount_ = 0;
    return rv;
  }

  virtual void onSetTime(const simCore::TimeStamp &t, bool isJump)
  {
    if (isJump || pauseTimes_.empty())
      return;

    std::set<simCore::TimeStamp>::const_iterator it = std::upper_bound(pauseTimes_.begin(), pauseTimes_.end(), t);

    // If no breaks before the time then return
    if (it == pauseTimes_.begin())
      return;

    // backup to the break at or before the time
    --it;

    // If time matches the break then increase count
    if (*it == t)
      ++breakCount_;
  }

  virtual void onTimeLoop()
  {
  }

  virtual void adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
  {
    ++callbackCount_;
    // Make sure there is something to do
    if (pauseTimes_.empty())
      return;

    std::set<simCore::TimeStamp>::const_iterator it = std::upper_bound(pauseTimes_.begin(), pauseTimes_.end(), oldTime);

    // If no breaks after the old time then return
    if (it == pauseTimes_.end())
      return;

    // If new time matches the break then return
    if (*it == newTime)
      return;

    // If the break time is between the oldTime and the newTime adjust the newTime to the break time
    if ((*it > oldTime) && (*it < newTime))
      newTime = *it;
  }

protected:
  std::set<simCore::TimeStamp> pauseTimes_;
  unsigned int breakCount_;
  unsigned int callbackCount_;
};

int adjustTimeTest()
{
  int rv = 0;

  simCore::ClockImpl clock;
  AdjustTimeObserver* obs = new AdjustTimeObserver;
  simCore::Clock::TimeObserverPtr observer(obs);
  clock.registerTimeCallback(observer);

  clock.setMode(simCore::Clock::MODE_STEP);
  clock.setTime(simCore::TimeStamp(2016, 5.0));
  clock.setTimeScale(1.0);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 5.0));
  clock.playForward();
  // Do a step without a break to make sure everything is configured correctly
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 6.0));
  rv += SDK_ASSERT(obs->breakCount() == 0);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // add a break
  obs->addExpectedPauseTime(simCore::TimeStamp(2016, 6.5));

  // Should stop at the break
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 6.5));
  rv += SDK_ASSERT(obs->breakCount() == 1);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // Should not break
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 7.5));
  rv += SDK_ASSERT(obs->breakCount() == 0);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // add a break
  obs->addExpectedPauseTime(simCore::TimeStamp(2016, 8.0));

  // A jump over should not trigger the break
  clock.setTime(simCore::TimeStamp(2016, 9.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 9.0));
  rv += SDK_ASSERT(obs->breakCount() == 0);
  rv += SDK_ASSERT(obs->callbackCount() == 0);

  // A jump back should not trigger the break
  clock.setTime(simCore::TimeStamp(2016, 7.0));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 7.0));
  rv += SDK_ASSERT(obs->breakCount() == 0);
  rv += SDK_ASSERT(obs->callbackCount() == 0);

  // Idle on to a break
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 8.0));
  rv += SDK_ASSERT(obs->breakCount() == 1);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // Idle to 9 seconds
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 9.0));
  rv += SDK_ASSERT(obs->breakCount() == 0);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // Add two break very close to each other
  obs->addExpectedPauseTime(simCore::TimeStamp(2016, 9.001));
  obs->addExpectedPauseTime(simCore::TimeStamp(2016, 9.002));

  // Idle to 9.001 seconds
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 9.001));
  rv += SDK_ASSERT(obs->breakCount() == 1);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // Idle to 9.002 seconds
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 9.002));
  rv += SDK_ASSERT(obs->breakCount() == 1);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // Idle to 10.002 seconds
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 10.002));
  rv += SDK_ASSERT(obs->breakCount() == 0);
  rv += SDK_ASSERT(obs->callbackCount() == 1);

  // Playing backwards should not result in any callbacks
  clock.playReverse();

  // Idle to 9.002 seconds
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 9.002));
  rv += SDK_ASSERT(obs->callbackCount() == 0);

  // Idle to 8.002 seconds
  clock.idle();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 8.002));
  rv += SDK_ASSERT(obs->callbackCount() == 0);

  clock.removeTimeCallback(observer);
  return rv;
}

int userTimeBoundsTest()
{
  int rv = 0;

  simCore::ClockImpl clock;
  clock.setMode(simCore::Clock::MODE_STEP);
  clock.setTimeScale(1.0);
  clock.setStartTime(simCore::TimeStamp(2016, 5.0));
  clock.setEndTime(simCore::TimeStamp(2016, 30.0));
  clock.setCanLoop(true);
  clock.setTime(simCore::TimeStamp(2016, 29.0));
  clock.stepForward();

  // Test rollover in step mode
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 30.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 5.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 6.0));
  clock.stepBackward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 5.0));
  clock.stepBackward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 30.0));

  simCore::Optional<simCore::TimeStamp> userStart;
  userStart = simCore::TimeStamp(2016, 10);
  simCore::Optional<simCore::TimeStamp> userEnd;
  userEnd = simCore::TimeStamp(2016, 20);
  rv += SDK_ASSERT(clock.setUserTimeBounds(userStart, userEnd) == 0);

  // Test rollover in step mode with custom time bounds
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 20.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 10.0));
  clock.stepBackward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 20.0));
  clock.setTime(simCore::TimeStamp(2016, 8.00));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 10.0));
  clock.setTime(simCore::TimeStamp(2016, 22.00));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 20.0));

  // Test reseting time bounds. Clock should work with previously configured start/end times
  userStart.reset();
  userEnd.reset();
  rv += SDK_ASSERT(clock.setUserTimeBounds(userStart, userEnd) == 0);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 20.0));
  clock.setTime(simCore::TimeStamp(2016, 30.00));
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 30.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 5.0));
  clock.stepBackward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 30.0));

  // Test real time mode
  clock.setMode(simCore::Clock::MODE_REALTIME);
  userStart = simCore::TimeStamp(2016, 10);
  userEnd = simCore::TimeStamp(2016, 20);
  rv += SDK_ASSERT(clock.setUserTimeBounds(userStart, userEnd) == 0);
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 20.0));
  clock.stepForward();
  rv += SDK_ASSERT(clock.currentTime() == simCore::TimeStamp(2016, 10.0));

  // User time bounds do not work in live mode
  clock.setMode(simCore::Clock::MODE_FREEWHEEL);
  rv += SDK_ASSERT(clock.setUserTimeBounds(userStart, userEnd) != 0);
  clock.setMode(simCore::Clock::MODE_SIMULATION);
  rv += SDK_ASSERT(clock.setUserTimeBounds(userStart, userEnd) != 0);

  return rv;
}

}

int TimeManagerTest(int argc, char* argv[])
{
  int rv = 0;
  rv += modeObserverTest();
  rv += observerTest();
  rv += stepTest();
  rv += realtimeTest();
  rv += freewheelTest();
  rv += simulationTest();
  rv += adjustTimeTest();
  rv += userTimeBoundsTest();
  return rv;
}
