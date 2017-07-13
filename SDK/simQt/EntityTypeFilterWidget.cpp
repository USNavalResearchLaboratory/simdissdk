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
#include <QMetaType>
#include "ui_EntityTypeFilter.h"
#include "simQt/QtConversion.h"
#include "simQt/EntityTypeFilterWidget.h"

namespace simQt {

EntityTypeFilterWidget::EntityTypeFilterWidget(QWidget* parent, unsigned int types)
  : QWidget(parent)
{
  setWindowTitle("Entity Type Filter:");
  ui_ = new Ui_EntityTypeFilter();
  ui_->setupUi(this);

  // activate check boxes based on types passed in
  setSelections(types);

  connect(ui_->allCheckable, SIGNAL(clicked(bool)), this, SLOT(toggleAllTypes_(bool)));
  connect(ui_->platformCheckable, SIGNAL(clicked(bool)), this, SLOT(entityTypeClicked_()));
  connect(ui_->beamCheckable, SIGNAL(clicked(bool)), this, SLOT(entityTypeClicked_()));
  connect(ui_->gateCheckable, SIGNAL(clicked(bool)), this, SLOT(entityTypeClicked_()));
  connect(ui_->laserCheckable, SIGNAL(clicked(bool)), this, SLOT(entityTypeClicked_()));
  connect(ui_->lobCheckable, SIGNAL(clicked(bool)), this, SLOT(entityTypeClicked_()));
  connect(ui_->projectorCheckable, SIGNAL(clicked(bool)), this, SLOT(entityTypeClicked_()));

  // Set tooltips
  ui_->allCheckable->setToolTip(simQt::formatTooltip(tr("All"),
    tr("Toggles the display of all entity types in the Entity List.")));
  ui_->platformCheckable->setToolTip(simQt::formatTooltip(tr("Platforms"),
    tr("Toggles the display of all platform entities in the Entity List.")));
  ui_->beamCheckable->setToolTip(simQt::formatTooltip(tr("Beams"),
    tr("Toggles the display of all beam entities in the Entity List.")));
  ui_->gateCheckable->setToolTip(simQt::formatTooltip(tr("Gates"),
    tr("Toggles the display of all gate entities in the Entity List.")));
  ui_->laserCheckable->setToolTip(simQt::formatTooltip(tr("Lasers"),
    tr("Toggles the display of all laser entities in the Entity List.")));
  ui_->lobCheckable->setToolTip(simQt::formatTooltip(tr("LOBs"),
    tr("Toggles the display of all LOB entities in the Entity List.")));
  ui_->projectorCheckable->setToolTip(simQt::formatTooltip(tr("Projectors"),
    tr("Toggles the display of all projector entities in the Entity List.")));
}

EntityTypeFilterWidget::~EntityTypeFilterWidget()
{
  delete ui_;
  ui_ = NULL;
}

unsigned int EntityTypeFilterWidget::getSelections() const
{
  // query all our Checkables to determine which ones are selected
  unsigned int rv = simData::DataStore::NONE;
  if (ui_->platformCheckable->isChecked())
    rv |= simData::DataStore::PLATFORM;
  if (ui_->beamCheckable->isChecked())
    rv |= simData::DataStore::BEAM;
  if (ui_->gateCheckable->isChecked())
    rv |= simData::DataStore::GATE;
  if (ui_->laserCheckable->isChecked())
    rv |= simData::DataStore::LASER;
  if (ui_->lobCheckable->isChecked())
    rv |= simData::DataStore::LOB_GROUP;
  if (ui_->projectorCheckable->isChecked())
    rv |= simData::DataStore::PROJECTOR;
  // update the all button state, based on components' state
  ui_->allCheckable->setChecked(rv == simData::DataStore::ALL);
  return rv;
}

std::set<simData::DataStore::ObjectType> EntityTypeFilterWidget::getSelectionsSet() const
{
  std::set<simData::DataStore::ObjectType> rv;
  if (ui_->platformCheckable->isChecked())
    rv.insert(simData::DataStore::PLATFORM);
  if (ui_->beamCheckable->isChecked())
    rv.insert(simData::DataStore::BEAM);
  if (ui_->gateCheckable->isChecked())
    rv.insert(simData::DataStore::GATE);
  if (ui_->laserCheckable->isChecked())
    rv.insert(simData::DataStore::LASER);
  if (ui_->lobCheckable->isChecked())
    rv.insert(simData::DataStore::LOB_GROUP);
  if (ui_->projectorCheckable->isChecked())
    rv.insert(simData::DataStore::PROJECTOR);
  return rv;
}


void EntityTypeFilterWidget::setSelections(unsigned int types)
{
  ui_->platformCheckable->setChecked(simData::DataStore::PLATFORM & types);
  ui_->beamCheckable->setChecked(simData::DataStore::BEAM & types);
  ui_->gateCheckable->setChecked(simData::DataStore::GATE & types);
  ui_->laserCheckable->setChecked(simData::DataStore::LASER & types);
  ui_->lobCheckable->setChecked(simData::DataStore::LOB_GROUP & types);
  ui_->projectorCheckable->setChecked(simData::DataStore::PROJECTOR & types);
  ui_->allCheckable->setChecked(types == simData::DataStore::ALL);
}

void EntityTypeFilterWidget::setSelections(const std::set<simData::DataStore::ObjectType>& types)
{
  unsigned int selections = simData::DataStore::NONE;
  for (std::set<simData::DataStore::ObjectType>::const_iterator typeIter = types.begin(); typeIter != types.end(); ++typeIter)
  {
    selections |= (*typeIter);
  }
  setSelections(selections);
}

void EntityTypeFilterWidget::entityTypeClicked_()
{
  // send out a signal, get selections from the selected check boxes
  emit(entityTypesChanged(getSelections()));
}

void EntityTypeFilterWidget::toggleAllTypes_(bool activateAllTypes)
{
  simData::DataStore::ObjectType type = activateAllTypes ? simData::DataStore::ALL : simData::DataStore::NONE;
  setSelections(type);
  emit(entityTypesChanged(type));
}

}

