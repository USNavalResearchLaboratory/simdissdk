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

#include <QComboBox>
#include "simData/DataStoreHelpers.h"
#include "simQt/EntityStateFilter.h"

namespace simQt {

/** A time change may cause a re-filter */
class EntityStateFilter::TimeObserver : public simCore::Clock::TimeObserver
{
public:
  /** Constructor */
  explicit TimeObserver(EntityStateFilter& parent)
    : parent_(parent)
  {
  }

  virtual void onSetTime(const simCore::TimeStamp &t, bool isJump)
  {
    parent_.newTime_();
  }

  virtual void onTimeLoop()
  {
  }

  virtual void adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
  {
  }

private:
  EntityStateFilter& parent_;
};


//----------------------------------------------------------------------------------------------------


EntityStateFilter::EntityStateFilter(const simData::DataStore& dataStore, simCore::Clock& clock, bool showWidget)
  : EntityFilter(),
  dataStore_(dataStore),
  clock_(clock),
  showWidget_(showWidget),
  state_(BOTH)
{
  clockAdapter_.reset(new TimeObserver(*this));
  clock_.registerTimeCallback(clockAdapter_);
}

EntityStateFilter::~EntityStateFilter()
{
  clock_.removeTimeCallback(clockAdapter_);
}

bool EntityStateFilter::acceptEntity(simData::ObjectId id) const
{
  if (state_ == BOTH)
    return true;

  const double time = clock_.currentTime().secondsSinceRefYear(dataStore_.referenceYear());
  // Assertion failure means that isEntityActive may return invalid values
  assert(clock_.isLiveMode() == dataStore_.dataLimiting());
  const bool isActive = simData::DataStoreHelpers::isEntityActive(dataStore_, id, time);

  return (state_ == ACTIVE) ? isActive : !isActive;
}

QWidget* EntityStateFilter::widget(QWidget* newWidgetParent) const
{
  // only generate the widget if we are set to show a widget
  if (showWidget_)
  {
    QComboBox* rv = new QComboBox(newWidgetParent);
    rv->setWindowTitle("Entity State Filter:");
    rv->addItem(tr("Active"));
    rv->addItem(tr("Not Active"));
    rv->addItem(tr("Active and Not Active"));
    rv->setCurrentIndex(static_cast<int>(state_));
    // connect to the signal so we can update the filter based on GUI changes
    connect(rv, SIGNAL(currentIndexChanged(int)), this, SLOT(entityStateChanged_(int)));
    connect(this, SIGNAL(entityStateChanged(int)), rv, SLOT(setCurrentIndex(int)));
    return rv;
  }
  return NULL;
}

void EntityStateFilter::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  settings.insert("EntityStateFilter", static_cast<int>(state_));
}

void EntityStateFilter::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  QMap<QString, QVariant>::const_iterator it = settings.find("EntityStateFilter");
  if (it != settings.end())
  {
    // If GUI is displayed update GUI which will call entityStateChanged_, otherwise directly call entityStateChanged_
    const bool hasGui = (receivers(SIGNAL(entityStateChanged(int))) != 0);
    if (hasGui)
      emit entityStateChanged(it.value().toInt());
    else
      entityStateChanged_(it.value().toInt());
  }
}

void EntityStateFilter::setStateFilter(State state)
{
  // If GUI is displayed update GUI which will call entityStateChanged_, otherwise directly call entityStateChanged_
  const bool hasGui = (receivers(SIGNAL(entityStateChanged(int))) != 0);
  if (hasGui)
    emit entityStateChanged(state);
  else
    entityStateChanged_(state);
}

EntityStateFilter::State EntityStateFilter::stateFilter() const
{
  return state_;
}

void EntityStateFilter::entityStateChanged_(int state)
{
  if (state_ != static_cast<State>(state))
  {
    state_ = static_cast<State>(state);
    emit filterUpdated();
  }
}

void EntityStateFilter::newTime_()
{
  if (state_ != BOTH)
    emit filterUpdated();
}

}
