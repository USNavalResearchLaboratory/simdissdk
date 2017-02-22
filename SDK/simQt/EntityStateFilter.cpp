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

    bool rv = true;

    double time = clock_.currentTime().secondsSinceRefYear(dataStore_.referenceYear());
    simData::DataStore::ObjectType type = dataStore_.objectType(id);
    switch (type)
    {
    case simData::DataStore::PLATFORM:
      rv = acceptPlatform_(id, time);
      break;
    case simData::DataStore::BEAM:
      rv = acceptBeam_(id, time);
      break;
    case simData::DataStore::GATE:
      rv = acceptGate_(id, time);
      break;
    case simData::DataStore::LASER:
      rv = acceptLaser_(id, time);
      break;
    case simData::DataStore::LOB_GROUP:
      rv = acceptLob_(id, time);
      break;
    case simData::DataStore::PROJECTOR:
      rv = true;
      break;
    case simData::DataStore::NONE:
      // Should never be called
      assert(false);
      return true;
    default:
      // The switch statement needs to be updated
      assert(false);
      return false;
    }

    return (state_ == ACTIVE) ? rv : !rv;
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
      // If no GUI, update internally, otherwise update GUI
      if (receivers(SIGNAL(entityStateChanged(int))) == 0)
        entityStateChanged_(it.value().toInt());
      else
        emit entityStateChanged(it.value().toInt());
    }
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

  bool EntityStateFilter::acceptPlatform_(simData::ObjectId id, double time) const
  {
    if (clock_.isLiveMode())
    {
      simData::DataStore::Transaction txn;
      const simData::CommonPrefs* prefs = dataStore_.commonPrefs(id, &txn);
      if (prefs != NULL)
      {
        return prefs->datadraw();
      }

      return true;
    }

    const simData::PlatformUpdateSlice* slice = dataStore_.platformUpdateSlice(id);
    if (slice == NULL)
      return false;

    // static platforms are always active
    if (slice->firstTime() == -1.0)
      return true;

    if ((slice->firstTime() > time) || (slice->lastTime() < time))
      return false;

    return true;
  }

  bool EntityStateFilter::acceptBeam_(simData::ObjectId id, double time) const
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::BeamProperties* beamProperty = dataStore_.beamProperties(id, &propertyTrans);
    if (!acceptPlatform_(beamProperty->hostid(), time))
      return false;

    const simData::BeamCommandSlice* slice = dataStore_.beamCommandSlice(id);
    if (slice == NULL)
      return false;

    // Check the draw state
    bool rv = false;
    simData::BeamCommandSlice::Iterator iter = slice->upper_bound(time);
    while (iter.hasPrevious())
    {
      const simData::BeamCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().commonprefs().has_datadraw())
      {
        rv = command->updateprefs().commonprefs().datadraw();
        break;
      }
    }

    // If false can return now
    if (!rv)
      return false;

    // Active depends on Beam Type
    if (beamProperty->type() != simData::BeamProperties::TARGET)
      return rv;

    // Verify that the target beam has a target and that the target is active
    iter = slice->upper_bound(time);
    while (iter.hasPrevious())
    {
      const simData::BeamCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().has_targetid())
      {
        // Verify that the target platform exists
        return acceptPlatform_(command->updateprefs().targetid(), time);
      }
    }

    // no previous target command exists
    return false;
  }

  bool EntityStateFilter::acceptGate_(simData::ObjectId id, double time) const
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::GateProperties* gateProperty = dataStore_.gateProperties(id, &propertyTrans);
    if (!acceptBeam_(gateProperty->hostid(), time))
      return false;

    const simData::GateCommandSlice* slice = dataStore_.gateCommandSlice(id);
    if (slice == NULL)
      return false;

    // Check the draw state
    simData::GateCommandSlice::Iterator iter = slice->upper_bound(time);
    while (iter.hasPrevious())
    {
      const simData::GateCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().commonprefs().has_datadraw())
      {
        return command->updateprefs().commonprefs().datadraw();
      }
    }

    // no previous data draw command exists
    return false;
  }

  bool EntityStateFilter::acceptLaser_(simData::ObjectId id, double time) const
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::LaserProperties* laserProperty = dataStore_.laserProperties(id, &propertyTrans);
    if (!acceptPlatform_(laserProperty->hostid(), time))
      return false;

    const simData::LaserCommandSlice* slice = dataStore_.laserCommandSlice(id);
    if (slice == NULL)
      return false;

    // Check the draw state
    simData::LaserCommandSlice::Iterator iter = slice->upper_bound(time);
    while (iter.hasPrevious())
    {
      const simData::LaserCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().commonprefs().has_datadraw())
      {
        return command->updateprefs().commonprefs().datadraw();
      }
    }

    // no previous data draw command exists
    return false;
  }

  bool EntityStateFilter::acceptLob_(simData::ObjectId id, double time) const
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::LobGroupProperties* lobProperty = dataStore_.lobGroupProperties(id, &propertyTrans);
    if (!acceptPlatform_(lobProperty->hostid(), time))
      return false;

    // LOB do NOT have datadraw command; LOBs are always on
    return true;
  }
}

