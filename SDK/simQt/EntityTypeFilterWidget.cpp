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
#include <QMetaType>
#include "ui_EntityTypeFilter.h"
#include "simQt/QtFormatting.h"
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
  connect(ui_->customRenderingCheckable, SIGNAL(clicked(bool)), this, SLOT(entityTypeClicked_()));

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
  ui_->customRenderingCheckable->setToolTip(simQt::formatTooltip(tr("Custom Rendering"),
    tr("Toggles the display of all custom rendering entities in the Entity List.")));
}

EntityTypeFilterWidget::~EntityTypeFilterWidget()
{
  delete ui_;
  ui_ = nullptr;
}

unsigned int EntityTypeFilterWidget::getSelections() const
{
  // query all our Checkables to determine which ones are selected
  unsigned int rv = simData::NONE;
  if (ui_->platformCheckable->isChecked())
    rv |= simData::PLATFORM;
  if (ui_->beamCheckable->isChecked())
    rv |= simData::BEAM;
  if (ui_->gateCheckable->isChecked())
    rv |= simData::GATE;
  if (ui_->laserCheckable->isChecked())
    rv |= simData::LASER;
  if (ui_->lobCheckable->isChecked())
    rv |= simData::LOB_GROUP;
  if (ui_->projectorCheckable->isChecked())
    rv |= simData::PROJECTOR;
  if (ui_->customRenderingCheckable->isChecked())
    rv |= simData::CUSTOM_RENDERING;
  // update the all button state, based on components' state
  ui_->allCheckable->setChecked(rv == simData::ALL);
  return rv;
}

std::set<simData::ObjectType> EntityTypeFilterWidget::getSelectionsSet() const
{
  std::set<simData::ObjectType> rv;
  if (ui_->platformCheckable->isChecked())
    rv.insert(simData::PLATFORM);
  if (ui_->beamCheckable->isChecked())
    rv.insert(simData::BEAM);
  if (ui_->gateCheckable->isChecked())
    rv.insert(simData::GATE);
  if (ui_->laserCheckable->isChecked())
    rv.insert(simData::LASER);
  if (ui_->lobCheckable->isChecked())
    rv.insert(simData::LOB_GROUP);
  if (ui_->projectorCheckable->isChecked())
    rv.insert(simData::PROJECTOR);
  if (ui_->customRenderingCheckable->isChecked())
    rv.insert(simData::CUSTOM_RENDERING);
  return rv;
}


void EntityTypeFilterWidget::setSelections(unsigned int types)
{
  if (getSelections() == types)
    return;

  // Note that because we tie into clicked(), calling setChecekd() will not emit
  // a signal.  We will emit a signal at the end.
  ui_->platformCheckable->setChecked(simData::PLATFORM & types);
  ui_->beamCheckable->setChecked(simData::BEAM & types);
  ui_->gateCheckable->setChecked(simData::GATE & types);
  ui_->laserCheckable->setChecked(simData::LASER & types);
  ui_->lobCheckable->setChecked(simData::LOB_GROUP & types);
  ui_->projectorCheckable->setChecked(simData::PROJECTOR & types);
  ui_->customRenderingCheckable->setChecked(simData::CUSTOM_RENDERING & types);
  ui_->allCheckable->setChecked(types == simData::ALL);

  // Emit a signal that the values have changed
  Q_EMIT entityTypesChanged(getSelections());
}

void EntityTypeFilterWidget::setSelections(const std::set<simData::ObjectType>& types)
{
  unsigned int selections = simData::NONE;
  for (std::set<simData::ObjectType>::const_iterator typeIter = types.begin(); typeIter != types.end(); ++typeIter)
  {
    selections |= (*typeIter);
  }
  setSelections(selections);
}

void EntityTypeFilterWidget::entityTypeClicked_()
{
  // send out a signal, get selections from the selected check boxes
  Q_EMIT(entityTypesChanged(getSelections()));
}

void EntityTypeFilterWidget::toggleAllTypes_(bool activateAllTypes)
{
  simData::ObjectType type = activateAllTypes ? simData::ALL : simData::NONE;
  setSelections(type);
  // emit is handled in setSelections()
}

}

