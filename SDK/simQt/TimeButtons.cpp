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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simQt/QtFormatting.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/TimeButtons.h"
#include "ui_TimeButtons.h"

//----------------------------------------------------------------------------
namespace simQt {

/// Observer of clock mode changes (not clock time changes)
class ButtonActions::TimeModeObserver : public simCore::Clock::ModeChangeObserver
{
public:
  /** Constructor */
  explicit TimeModeObserver(simQt::ButtonActions *w)
    : w_(w)
  {
  }

  /** @see simCore::Clock::ModeChangeObserver::onModeChange() */
  virtual void onModeChange(simCore::Clock::Mode newMode)
  {
    // Mode change can affect real-time checkbox
    w_->updateCheckedState_();
  }

  /** @see simCore::Clock::ModeChangeObserver::onDirectionChange() */
  virtual void onDirectionChange(simCore::TimeDirection newDirection)
  {
    // Direction change affects which play/stop buttons are pressed in
    w_->updateCheckedState_();
  }

  /** @see simCore::Clock::ModeChangeObserver::onUserEditableChanged() */
  virtual void onUserEditableChanged(bool userCanEdit)
  {
    // A change in editable state could be the result of changing between data clock and visualization clock
    w_->updateCheckedState_();
    // Enable/disable changes
    w_->updateEnabledState_();
  }

  /** @see simCore::Clock::ModeChangeObserver::onCanLoopChange() */
  virtual void onCanLoopChange(bool newVal)
  {
    // Can change whether Loop is checked or not
    w_->updateCheckedState_();
  }

  /** @see simCore::Clock::ModeChangeObserver::onScaleChange() */
  virtual void onScaleChange(double newValue) {}

  /** @see simCore::Clock::ModeChangeObserver::onBoundsChange() */
  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end) {}

private:
  simQt::ButtonActions *w_;
};

ButtonActions::ButtonActions(QWidget *parent)
: QObject(parent),
  clock_(nullptr),
  stepDecrease_(new QAction(QIcon(":/simQt/images/Navigation Blue Left.png"), "Decrease Rate", parent)),
  stepBack_(new QAction(QIcon(":/simQt/images/Navigation Blue First.png"), "Step Back", parent)),
  playReverse_(new QAction(QIcon(":/simQt/images/Navigation Blue Previous.png"), "Play Backward", parent)),
  stop_(new QAction(QIcon(":/simQt/images/Navigation Blue Stop.png"), "Stop", parent)),
  play_(new QAction(QIcon(":/simQt/images/Navigation Blue Next.png"), "Play Forward", parent)),
  stepForward_(new QAction(QIcon(":/simQt/images/Navigation Blue Last.png"), "Step Forward", parent)),
  stepIncrease_(new QAction(QIcon(":/simQt/images/Navigation Blue Right.png"), "Increase Rate", parent)),
  realTime_(new QAction(QIcon(":/simQt/images/Symbol Clock.png"), "Real Time", parent)),
  toggleLoop_(new QAction(QIcon(":/simQt/images/Loop.png"), "Toggle Looping", parent))
{
  // set default shortcuts
  stepDecrease_->setShortcut(QKeySequence(Qt::Key_Minus));
  playReverse_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
  stepForward_->setShortcut(QKeySequence(Qt::Key_Space));
  stepIncrease_->setShortcut(QKeySequence(Qt::Key_Equal));
  realTime_->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_R));
  // set tooltips
  stepDecrease_->setToolTip(formatTooltip(tr("Decrease Rate"), tr("Slow down the rate of playback.")));
  stepBack_->setToolTip(formatTooltip(tr("Step Back"), tr("Move the scenario one time-step backward.")));
  playReverse_->setToolTip(formatTooltip(tr("Play Backward"), tr("Play the scenario backward.")));
  stop_->setToolTip(formatTooltip(tr("Stop"), tr("Stop the scenario playback.")));
  play_->setToolTip(formatTooltip(tr("Play"), tr("Play the scenario forward.")));
  stepForward_->setToolTip(formatTooltip(tr("Step Forward"), tr("Move the scenario one time-step forward.")));
  stepIncrease_->setToolTip(formatTooltip(tr("Increase Rate"), tr("Speed up the rate of playback.")));
  realTime_->setToolTip(formatTooltip(tr("Real Time"), tr("Set the scenario playback to real time.")));
  // connect all the buttons to our slots
  connect(stepDecrease_, SIGNAL(triggered()), this, SLOT(clockStepDecrease_()));
  connect(stepBack_, SIGNAL(triggered()), this, SLOT(clockStepBack_()));
  connect(playReverse_, SIGNAL(triggered()), this, SLOT(clockPlayBackwards_()));
  connect(stop_, SIGNAL(triggered()), this, SLOT(clockStop_()));
  connect(play_, SIGNAL(triggered()), this, SLOT(clockPlay_()));
  connect(stepForward_, SIGNAL(triggered()), this, SLOT(clockStepForward_()));
  connect(stepIncrease_, SIGNAL(triggered()), this, SLOT(clockStepIncrease_()));
  connect(realTime_, SIGNAL(triggered(bool)), this, SLOT(clockRealTime_(bool)));
  connect(toggleLoop_, SIGNAL(triggered(bool)), this, SLOT(clockToggleLoop_(bool)));
  playReverse_->setCheckable(true);
  stop_->setCheckable(true);
  play_->setCheckable(true);
  realTime_->setCheckable(true);
  toggleLoop_->setCheckable(true);

  observer_.reset(new TimeModeObserver(this));
}

ButtonActions::~ButtonActions()
{
  setClockManager(nullptr);
  qDeleteAll(actions());
}

QList<QAction*> ButtonActions::actions() const
{
  QList<QAction*> rv;
  rv.push_back(stepDecrease_);
  rv.push_back(stepBack_);
  rv.push_back(playReverse_);
  rv.push_back(stop_);
  rv.push_back(play_);
  rv.push_back(stepForward_);
  rv.push_back(stepIncrease_);
  rv.push_back(realTime_);
  rv.push_back(toggleLoop_);
  return rv;
}

QAction* ButtonActions::stepDecreaseAction() const
{
  return stepDecrease_;
}

QAction* ButtonActions::stepBackAction() const
{
  return stepBack_;
}

QAction* ButtonActions::playReverseAction() const
{
  return playReverse_;
}

QAction* ButtonActions::stopAction() const
{
  return stop_;
}

QAction* ButtonActions::playAction() const
{
  return play_;
}

QAction* ButtonActions::stepForwardAction() const
{
  return stepForward_;
}

QAction* ButtonActions::stepIncreaseAction() const
{
  return stepIncrease_;
}

QAction* ButtonActions::realTimeAction() const
{
  return realTime_;
}

QAction* ButtonActions::toggleLoopAction() const
{
  return toggleLoop_;
}

void ButtonActions::setClockManager(simCore::Clock *clock)
{
  if (clock_)
    clock_->removeModeChangeCallback(observer_);

  clock_ = clock;
  if (clock_)
  {
    clock_->registerModeChangeCallback(observer_);

    // pull all the state from the clock
    updateCheckedState_();
    updateEnabledState_();
  }
}

void ButtonActions::updateEnabledState_()
{
  bool enable = (clock_ != nullptr && clock_->isUserEditable());

  // Emit a signal to indicate that the state has changed
  if (toggleLoop_->isEnabled() != enable)
  {
    auto actionsList = actions();
    for (auto it = actionsList.begin(); it != actionsList.end(); ++it)
      (*it)->setEnabled(enable);

    // Toggle loop is enabled when controls are not disabled
    toggleLoop_->setEnabled(enable);

    // Alert anyone bound to our signal
    emit timeControlEnableStateChanged(enable);
  }
}

void ButtonActions::updateCheckedState_()
{
  simQt::ScopedSignalBlocker blockPlayReverse(*playReverse_);
  simQt::ScopedSignalBlocker blockPlayForward(*play_);
  simQt::ScopedSignalBlocker blockStop(*stop_);
  // do not block toggleLoop_ signals
  simQt::ScopedSignalBlocker blockRealTime(*realTime_);

  // play, stop, and reverse are exclusive
  const simCore::TimeDirection d = (clock_ != nullptr) ? clock_->timeDirection() : simCore::STOP;
  playReverse_->setChecked(d == simCore::REVERSE);
  play_->setChecked(d == simCore::FORWARD);
  stop_->setChecked(d == simCore::STOP);
  if (clock_)
    toggleLoop_->setChecked(clock_->canLoop());

  // free wheel mode is real time driven by a plug-in
  const simCore::Clock::Mode m = (clock_ != nullptr) ? clock_->mode() : simCore::Clock::MODE_STEP;
  realTime_->setChecked(m == simCore::Clock::MODE_REALTIME || m == simCore::Clock::MODE_FREEWHEEL);
}

void ButtonActions::clockStop_()
{
  if (clock_)
    clock_->stop();
  updateCheckedState_();
}

void ButtonActions::clockPlay_()
{
  if (clock_)
    clock_->playForward();
  updateCheckedState_();
}

void ButtonActions::clockStepBack_()
{
  if (clock_)
    clock_->stepBackward();
  updateCheckedState_();
}

void ButtonActions::clockStepForward_()
{
  if (clock_)
    clock_->stepForward();
  updateCheckedState_();
}

void ButtonActions::clockStepDecrease_()
{
  if (clock_)
    clock_->decreaseScale();
}

void ButtonActions::clockStepIncrease_()
{
  if (clock_)
    clock_->increaseScale();
}

void ButtonActions::clockPlayBackwards_()
{
  if (clock_)
    clock_->playReverse();
  updateCheckedState_();
}

void ButtonActions::clockRealTime_(bool pressed)
{
  if (clock_)
    clock_->setRealTime(pressed);
  updateCheckedState_();
}

void ButtonActions::clockToggleLoop_(bool pressed)
{
  if (clock_)
  {
    clock_->setCanLoop(pressed);
    // observer onCanLoopChange will updateCheckedState_()
  }
  else
    updateCheckedState_();
}

//////////////////////////////////////////////////////////////////

TimeButtons::TimeButtons(QWidget *parent)
: QWidget(parent)
{
  ui_ = new Ui_TimeButtons;
  ui_->setupUi(this);
}

TimeButtons::~TimeButtons()
{
  bindToActions(nullptr);
  delete ui_;
}

void TimeButtons::bindToActions(ButtonActions* actions)
{
  if (actions != nullptr)
  {
    ui_->button_StepDecrease->setDefaultAction(actions->stepDecreaseAction());
    ui_->button_StepBack->setDefaultAction(actions->stepBackAction());
    ui_->button_PlayBackwards->setDefaultAction(actions->playReverseAction());
    ui_->button_Stop->setDefaultAction(actions->stopAction());
    ui_->button_Play->setDefaultAction(actions->playAction());
    ui_->button_Step->setDefaultAction(actions->stepForwardAction());
    ui_->button_StepIncrease->setDefaultAction(actions->stepIncreaseAction());
    ui_->button_Realtime->setDefaultAction(actions->realTimeAction());
  }
  else
  {
    ui_->button_StepDecrease->setDefaultAction(nullptr);
    ui_->button_StepBack->setDefaultAction(nullptr);
    ui_->button_PlayBackwards->setDefaultAction(nullptr);
    ui_->button_Stop->setDefaultAction(nullptr);
    ui_->button_Play->setDefaultAction(nullptr);
    ui_->button_Step->setDefaultAction(nullptr);
    ui_->button_StepIncrease->setDefaultAction(nullptr);
    ui_->button_Realtime->setDefaultAction(nullptr);
  }
}

void TimeButtons::resizeButtons(int size)
{
  QSize newQSize(size, size);
  ui_->button_StepDecrease->resize(newQSize);
  ui_->button_StepDecrease->setIconSize(newQSize);
  ui_->button_StepBack->resize(newQSize);
  ui_->button_StepBack->setIconSize(newQSize);
  ui_->button_PlayBackwards->resize(newQSize);
  ui_->button_PlayBackwards->setIconSize(newQSize);
  ui_->button_Stop->resize(newQSize);
  ui_->button_Stop->setIconSize(newQSize);
  ui_->button_Play->resize(newQSize);
  ui_->button_Play->setIconSize(newQSize);
  ui_->button_Step->resize(newQSize);
  ui_->button_Step->setIconSize(newQSize);
  ui_->button_StepIncrease->resize(newQSize);
  ui_->button_StepIncrease->setIconSize(newQSize);
  ui_->button_Realtime->resize(newQSize);
  ui_->button_Realtime->setIconSize(newQSize);
}

}
