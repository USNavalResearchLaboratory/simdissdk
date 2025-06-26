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
#include <iostream>
#include <QComboBox>
#include "simData/DataStore.h"
#include "simQt/QtFormatting.h"
#include "EntityDrawFilter.h"

namespace simQt {

/// Settings key for entity draw filter
inline const QString ENTITY_DRAW_SETTING = "EntityDrawFilter";


/////////////////////////////////////////////////////////////////////////
/**
* Class that listens for entity events in the DataStore, and
* informs the parent when they happen.
*/
class EntityDrawFilter::DataStoreListener : public simData::DataStore::DefaultListener
{
public:
  explicit DataStoreListener(EntityDrawFilter& parent)
    : parent_(parent)
  {};

  virtual void onAddEntity(simData::DataStore* source, simData::ObjectId newId, simData::ObjectType ot) override
  {
    parent_.checkDrawState_(newId);
  }

  virtual void onRemoveEntity(simData::DataStore* source, simData::ObjectId removedId, simData::ObjectType ot) override
  {
    parent_.entityDrawStates_.erase(removedId);
  }

  virtual void onPrefsChange(simData::DataStore* source, simData::ObjectId id) override
  {
    parent_.checkDrawState_(id);
  }

  virtual void onChange(simData::DataStore* source) override
  {
    parent_.checkDirty_();
  }

private:
  EntityDrawFilter& parent_;
};


//----------------------------------------------------------------------------------------------------

EntityDrawFilter::EntityDrawFilter(simData::DataStore& dataStore, bool showWidget)
  : EntityFilter(),
    dataStore_(dataStore),
    showWidget_(showWidget)
{
  // initialize draw states
  simData::DataStore::IdList ids;
  dataStore_.idList(&ids);
  for (const auto id : ids)
  {
    entityDrawStates_[id] = getDrawState_(id);
  }
  dsListener_.reset(new DataStoreListener(*this));
  dataStore_.addListener(dsListener_);
}

EntityDrawFilter::~EntityDrawFilter()
{
  dataStore_.removeListener(dsListener_);
}

bool EntityDrawFilter::acceptEntity(simData::ObjectId id) const
{
  if (draw_ == Draw::BOTH)
    return true;

  const bool drawState = getDrawState_(id);
  return (draw_ == Draw::DRAW_ON ? drawState : !drawState);
}

QWidget* EntityDrawFilter::widget(QWidget* newWidgetParent) const
{
  // only generate the widget if we are set to show a widget
  if (showWidget_)
  {
    QComboBox* rv = new QComboBox(newWidgetParent);
    rv->setWindowTitle(tr("Entity Draw Filter:"));
    rv->addItem(tr("Draw On"));
    rv->addItem(tr("Draw Off"));
    rv->addItem(tr("Draw On and Off"));
    rv->setCurrentIndex(static_cast<int>(draw_));
    rv->setToolTip(simQt::formatTooltip(tr("Entity Draw Filter"), tr("Display all entities with draw flag on, off, or both.")));
    // connect to the signal so we can update the filter based on GUI changes
    connect(rv, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EntityDrawFilter::setDrawFilterInternal_);
    connect(this, &EntityDrawFilter::entityDrawChanged, rv, &QComboBox::setCurrentIndex);
    return rv;
  }
  return nullptr;
}

void EntityDrawFilter::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  // this is for local transfer of the filter state, filter state is not maintained on restart
  settings.insert(ENTITY_DRAW_SETTING, static_cast<int>(draw_));
}

void EntityDrawFilter::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  QMap<QString, QVariant>::const_iterator it = settings.find(ENTITY_DRAW_SETTING);
  if (it != settings.end())
  {
    // If GUI is displayed update GUI which will call setDrawFilterInternal_, otherwise directly call setDrawFilterInternal_
    const bool hasGui = (receivers(SIGNAL(entityDrawChanged(int))) != 0);
    if (hasGui)
      Q_EMIT entityDrawChanged(it.value().toInt());
    else
      setDrawFilterInternal_(it.value().toInt());
  }
}

void EntityDrawFilter::setDrawFilter(Draw drawState)
{
  // If GUI is displayed update GUI which will call setDrawFilterInternal_, otherwise directly call setDrawFilterInternal_
  const bool hasGui = (receivers(SIGNAL(entityDrawChanged(int))) != 0);
  if (hasGui)
    Q_EMIT entityDrawChanged(static_cast<int>(drawState));
  else
    setDrawFilterInternal_(static_cast<int>(drawState));
}

EntityDrawFilter::Draw EntityDrawFilter::drawFilter() const
{
  return draw_;
}

void EntityDrawFilter::setDrawFilterInternal_(int draw)
{
  const Draw newDraw = static_cast<Draw>(draw);
  if (draw_ == newDraw)
    return;
  draw_ = newDraw;
  dirty_ = false;
  Q_EMIT filterUpdated();
}

void EntityDrawFilter::checkDirty_()
{
  if (!dirty_)
    return;
  dirty_ = false;
  Q_EMIT filterUpdated();
}

void EntityDrawFilter::checkDrawState_(simData::ObjectId entityId)
{
  const bool drawState = getDrawState_(entityId);
  auto iter = entityDrawStates_.find(entityId);

  if (iter != entityDrawStates_.end())
  {
    // if in our map and no change, nothing to do
    if (iter->second == drawState)
      return;
    // update the map if state changed
    iter->second = drawState;
  }
  else // add to map if new entry
    entityDrawStates_[entityId] = drawState;

  // set dirty flag if filtering and it's not already set
  if (draw_ != Draw::BOTH && !dirty_)
    dirty_ = true;
}

bool EntityDrawFilter::getDrawState_(simData::ObjectId entityId) const
{
  simData::DataStore::Transaction txn;
  const auto* prefs = dataStore_.commonPrefs(entityId, &txn);
  if (!prefs)
    return false;
  const bool rv = prefs->draw();
  txn.complete(&prefs);
  return rv;
}

}
