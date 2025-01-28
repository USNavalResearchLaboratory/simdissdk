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
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QItemSelection>
#include <QStandardItemModel>
#include "osg/Sequence"
#include "osgSim/DOFTransform"
#include "osgSim/MultiSwitch"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "ui_ArticulationsEditorWidget.h"
#include "ArticulationsEditorWidget.h"

// Required to used ArticulationItem as a QVariant
Q_DECLARE_METATYPE(simQt::ArticulationItem);

namespace simQt {

/// Unnamed enumeration of auto-incrementing role flags for the item data model
enum
{
  NAME_ROLE = Qt::UserRole + 0,
  DATA_ROLE,
};

ArticulationsEditorWidget::ArticulationsEditorWidget(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_ArticulationsEditorWidget)
{
  ui_->setupUi(this);

  // Set up model
  itemModel_ = new QStandardItemModel;
  ui_->articulationTreeView->setModel(itemModel_);

  // Connect signals and slots
  connect(ui_->articulationTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ArticulationsEditorWidget::updateArticulationDetails_);
  connect(ui_->switchIndexCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ArticulationsEditorWidget::updateMultiSwitch_);
  connect(ui_->sequenceActiveCheck, &QCheckBox::stateChanged, this, &ArticulationsEditorWidget::updateSequence_);
  connect(ui_->headingSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setSpinBoxCurrentHeading_);
  connect(ui_->pitchSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setSpinBoxCurrentPitch_);
  connect(ui_->rollSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setSpinBoxCurrentRoll_);
  connect(ui_->headingSlider, &QSlider::valueChanged, this, &ArticulationsEditorWidget::setSliderCurrentHeading_);
  connect(ui_->pitchSlider, &QSlider::valueChanged, this, &ArticulationsEditorWidget::setSliderCurrentPitch_);
  connect(ui_->rollSlider, &QSlider::valueChanged, this, &ArticulationsEditorWidget::setSliderCurrentRoll_);
  connect(ui_->xOffsetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setCurrentTranslateX_);
  connect(ui_->yOffsetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setCurrentTranslateY_);
  connect(ui_->zOffsetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setCurrentTranslateZ_);
  connect(ui_->xScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setCurrentScaleX_);
  connect(ui_->yScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setCurrentScaleY_);
  connect(ui_->zScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ArticulationsEditorWidget::setCurrentScaleZ_);

  // Add header to model
  itemModel_->setColumnCount(2);
  QStandardItem* item = new QStandardItem(tr("Name"));
  itemModel_->setHorizontalHeaderItem(0, item);
  item = new QStandardItem(tr("Value"));
  itemModel_->setHorizontalHeaderItem(1, item);

  // Set stack widget invisible
  ui_->stackedWidget->setVisible(false);
}

ArticulationsEditorWidget::~ArticulationsEditorWidget()
{
  delete itemModel_;
  itemModel_ = nullptr;
}

QString ArticulationsEditorWidget::articulationTypeToQString_(ArticulationType type) const
{
  switch (type)
  {
  case ArticulationType::SEQUENCE:
    return tr("Sequence");
  case ArticulationType::DOF_TRANSFORM:
    return tr("DOF Transform");
  case ArticulationType::MULTI_SWITCH:
    return tr("MultiSwitch");
  }

  // Should not get here, but return empty string anyway
  assert(false);
  return QString();
}

void ArticulationsEditorWidget::displayArticulationInfo(const std::map<std::string, ArticulationItem>& articulationMap)
{
  // Display articulations in the tree view of the GUI. Start by clearing out model
  itemModel_->removeRows(0, itemModel_->rowCount());

  // If articulation map has articulation data, show them in the tree view
  if (!articulationMap.empty())
  {
    ui_->stackedWidget->setVisible(true);

    for (ArticulationMap::const_iterator it = articulationMap.begin(); it != articulationMap.end(); ++it)
    {
      // Set name for first column, type for second column
      QList<QStandardItem*> items;
      QString name = QString::fromStdString(it->first);
      QStandardItem* singleItem = new QStandardItem(name);
      singleItem->setData(name, NAME_ROLE);
      singleItem->setData(QVariant::fromValue(it->second), DATA_ROLE);
      singleItem->setFlags(singleItem->flags() & ~Qt::ItemIsEditable);
      items.append(singleItem);
      singleItem = new QStandardItem(articulationTypeToQString_(it->second.articulationType_));
      singleItem->setFlags(singleItem->flags() & ~Qt::ItemIsEditable);
      items.append(singleItem);
      itemModel_->appendRow(items);
    }

    ui_->articulationTreeView->setCurrentIndex(itemModel_->index(0, 0));
  }
  else
  {
    // Set stack widget invisible since this model has no articulations
    ui_->stackedWidget->setVisible(false);
    activeNode_ = nullptr;
  }
}

void ArticulationsEditorWidget::updateSpinBox_(QDoubleSpinBox* box, double minVal, double maxVal, double currentVal, DofType type) const
{
  // For rotations and translations, check if minimum and maximum values are the same.  If so, disable box because the
  // model wasn't set for a range.  For scales, I decided to check if minimum value is set to 0 and maximum value is
  // set to 1, which means the articulation wasn't bounded
  if ((minVal == maxVal) || (((type == DofType::SCALE_X) || (type == DofType::SCALE_Y) || (type == DofType::SCALE_Z)) &&
    simCore::areEqual(minVal, 0.0) && simCore::areEqual(maxVal, 1.0)))
  {
    box->setEnabled(false);
    box->setValue(0.0);
  }
  else
  {
    box->setEnabled(true);
    box->setMinimum(minVal);
    box->setMaximum(maxVal);
    box->setValue(currentVal);
    // If the DOF is not a rotation, set spin box increment to a tenth of the total range for now
    if ((type != DofType::HEADING) && (type != DofType::PITCH) && (type != DofType::ROLL))
    {
      box->setSingleStep((maxVal - minVal) / 10.0);
    }
  }
}

void ArticulationsEditorWidget::updateSlider_(QSlider* slider, double minVal, double maxVal, double currentVal)
{
  // For rotations, check if minimum and maximum values are the same.  If so, disable slider because the
  // model wasn't set for a range
  if (minVal == maxVal)
  {
    slider->setEnabled(false);
    slider->setMinimum(0.0);
    slider->setMaximum(0.0);
    slider->setValue(0.0);
  }
  else
  {
    slider->setEnabled(true);
    slider->setMinimum(minVal);
    slider->setMaximum(maxVal);
    slider->setValue(currentVal);
  }
}

void ArticulationsEditorWidget::updateArticulationDetails_(const QItemSelection& selectedItems)
{
  // Need at least one item
  if (selectedItems.empty())
    return;

  const QModelIndex idx = selectedItems.indexes().first();

  QVariant qData = itemModel_->data(idx, DATA_ROLE);
  if (!qData.canConvert<ArticulationItem>())
    return;

  ArticulationItem articulationItem = qData.value<ArticulationItem>();

  // If articulation is a sequence
  if (articulationItem.articulationType_ == ArticulationType::SEQUENCE)
  {
    // Set stackedWidget page to sequence
    ui_->stackedWidget->setVisible(true);
    ui_->stackedWidget->setCurrentIndex(2);

    activeNode_ = nullptr;

    const osg::Sequence* sequence = dynamic_cast<osg::Sequence*>(articulationItem.articulationNode_.get());
    if (sequence)
    {
      // Check sequence box if animation mode is "start" or "resume"; unchecked if "stop" or "pause"
      switch (sequence->getMode())
      {
      case osg::Sequence::STOP:
      case osg::Sequence::PAUSE:
        ui_->sequenceActiveCheck->setChecked(false);
	      break;
      case osg::Sequence::START:
      case osg::Sequence::RESUME:
        ui_->sequenceActiveCheck->setChecked(true);
	      break;
      }
    }

    activeNode_ = articulationItem.articulationNode_;
  }
  // Else if articulation is a degree-of-freedom transform
  else if (articulationItem.articulationType_ == ArticulationType::DOF_TRANSFORM)
  {
    // Set stackedWidget page to DOFTransform
    ui_->stackedWidget->setVisible(true);
    ui_->stackedWidget->setCurrentIndex(1);

    activeNode_ = nullptr;

    osgSim::DOFTransform* dof = dynamic_cast<osgSim::DOFTransform*>(articulationItem.articulationNode_.get());
    if (dof)
    {
      // Calculate minimum, maximum and current values for DOFTransform heading, pitch and roll
      osg::Vec3 minHPR = dof->getMinHPR();
      osg::Vec3 maxHPR = dof->getMaxHPR();
      osg::Vec3 currentHPR = dof->getCurrentHPR();
      double minHeading = minHPR.x() * simCore::RAD2DEG;
      double maxHeading = maxHPR.x() * simCore::RAD2DEG;
      double currentHeading = currentHPR.x() * simCore::RAD2DEG;
      double minPitch = minHPR.y() * simCore::RAD2DEG;
      double maxPitch = maxHPR.y() * simCore::RAD2DEG;
      double currentPitch = currentHPR.y() * simCore::RAD2DEG;
      double minRoll = minHPR.z() * simCore::RAD2DEG;
      double maxRoll = maxHPR.z() * simCore::RAD2DEG;
      double currentRoll = currentHPR.z() * simCore::RAD2DEG;
      // Update spin boxes.
      updateSpinBox_(ui_->headingSpin, minHeading, maxHeading, currentHeading, DofType::HEADING);
      updateSpinBox_(ui_->pitchSpin, minPitch, maxPitch, currentPitch, DofType::PITCH);
      updateSpinBox_(ui_->rollSpin, minRoll, maxRoll, currentRoll, DofType::ROLL);
      // Update sliders.
      updateSlider_(ui_->headingSlider, simCore::rint(minHeading), simCore::rint(maxHeading), simCore::rint(currentHeading));
      updateSlider_(ui_->pitchSlider, simCore::rint(minPitch), simCore::rint(maxPitch), simCore::rint(currentPitch));
      updateSlider_(ui_->rollSlider, simCore::rint(minRoll), simCore::rint(maxRoll), simCore::rint(currentRoll));

      // Set DOFTransform translation info for spin boxes.
      osg::Vec3 minTranslate = dof->getMinTranslate();
      osg::Vec3 maxTranslate = dof->getMaxTranslate();
      osg::Vec3 currentTranslate = dof->getCurrentTranslate();
      updateSpinBox_(ui_->xOffsetSpin, minTranslate.x(), maxTranslate.x(), currentTranslate.x(), DofType::TRANSLATE_X);
      updateSpinBox_(ui_->yOffsetSpin, minTranslate.y(), maxTranslate.y(), currentTranslate.y(), DofType::TRANSLATE_Y);
      updateSpinBox_(ui_->zOffsetSpin, minTranslate.z(), maxTranslate.z(), currentTranslate.z(), DofType::TRANSLATE_Z);

      // Set DOFTransform scaling for spin boxes.
      osg::Vec3 minScale = dof->getMinScale();
      osg::Vec3 maxScale = dof->getMaxScale();
      osg::Vec3 currentScale = dof->getCurrentScale();
      updateSpinBox_(ui_->xScaleSpin, minScale.x(), maxScale.x(), currentScale.x(), DofType::SCALE_X);
      updateSpinBox_(ui_->yScaleSpin, minScale.y(), maxScale.y(), currentScale.y(), DofType::SCALE_Y);
      updateSpinBox_(ui_->zScaleSpin, minScale.z(), maxScale.z(), currentScale.z(), DofType::SCALE_Z);
    }

    activeNode_ = articulationItem.articulationNode_;
  }
  // Else if articulation is a multiswitch
  else if (articulationItem.articulationType_ == ArticulationType::MULTI_SWITCH)
  {
    // Set stackedWidget page to multiswitch
    ui_->stackedWidget->setVisible(true);
    ui_->stackedWidget->setCurrentIndex(3);

    activeNode_ = nullptr;

    osgSim::MultiSwitch* ms = dynamic_cast<osgSim::MultiSwitch*>(articulationItem.articulationNode_.get());
    if (ms)
    {
      ui_->switchIndexCombo->clear();

      // Because current openflight models don't seem to have names for their multiswitch node,
      // just use index name for now.
      const unsigned int numSwitchStates = ms->getSwitchSetList().size();
      for (unsigned int i = 0; i < numSwitchStates; ++i)
        ui_->switchIndexCombo->insertItem(i, QString::number(i));

      // Set combo box based on current multiswitch set.
      unsigned int activeSet = ms->getActiveSwitchSet();
      ui_->switchIndexCombo->setCurrentIndex(activeSet);
    }

    activeNode_ = articulationItem.articulationNode_;
  }
  else
  {
    assert(false);
    ui_->stackedWidget->setVisible(false);
    activeNode_ = nullptr;
  }
}

void ArticulationsEditorWidget::updateSequence_(int state)
{
  osg::Sequence* s = dynamic_cast<osg::Sequence*>(activeNode_.get());
  if (s)
    s->setMode((static_cast<Qt::CheckState>(state) == Qt::Checked) ? osg::Sequence::START : osg::Sequence::STOP);
}

void ArticulationsEditorWidget::setSpinBoxCurrentHeading_(double val)
{
  ui_->headingSlider->setValue(static_cast<int>(val));
  // Make sure to set value back to radians
  setDofTransformValue_(DofType::HEADING, val * simCore::DEG2RAD);
}

void ArticulationsEditorWidget::setSliderCurrentHeading_(int val)
{
  ui_->headingSpin->setValue(static_cast<double>(val));
  // Make sure to set value back to radians
  setDofTransformValue_(DofType::HEADING, static_cast<double>(val * simCore::DEG2RAD));
}

void ArticulationsEditorWidget::setSpinBoxCurrentPitch_(double val)
{
  ui_->pitchSlider->setValue(static_cast<int>(val));
  // Make sure to set value back to radians
  setDofTransformValue_(DofType::PITCH, val * simCore::DEG2RAD);
}

void ArticulationsEditorWidget::setSliderCurrentPitch_(int val)
{
  ui_->pitchSpin->setValue(static_cast<double>(val));
  // Make sure to set value back to radians
  setDofTransformValue_(DofType::PITCH, static_cast<double>(val * simCore::DEG2RAD));
}

void ArticulationsEditorWidget::setSpinBoxCurrentRoll_(double val)
{
  ui_->rollSlider->setValue(static_cast<int>(val));
  // Make sure to set value back to radians
  setDofTransformValue_(DofType::ROLL, val * simCore::DEG2RAD);
}

void ArticulationsEditorWidget::setSliderCurrentRoll_(int val)
{
  ui_->rollSpin->setValue(static_cast<double>(val));
  // Make sure to set value back to radians
  setDofTransformValue_(DofType::ROLL, static_cast<double>(val * simCore::DEG2RAD));
}

void ArticulationsEditorWidget::setCurrentTranslateX_(double val)
{
  setDofTransformValue_(DofType::TRANSLATE_X , val);
}

void ArticulationsEditorWidget::setCurrentTranslateY_(double val)
{
  setDofTransformValue_(DofType::TRANSLATE_Y, val);
}

void ArticulationsEditorWidget::setCurrentTranslateZ_(double val)
{
  setDofTransformValue_(DofType::TRANSLATE_Z, val);
}

void ArticulationsEditorWidget::setCurrentScaleX_(double val)
{
  setDofTransformValue_(DofType::SCALE_X, val);
}

void ArticulationsEditorWidget::setCurrentScaleY_(double val)
{
  setDofTransformValue_(DofType::SCALE_Y, val);
}

void ArticulationsEditorWidget::setCurrentScaleZ_(double val)
{
  setDofTransformValue_(DofType::SCALE_Z, val);
}

void ArticulationsEditorWidget::setDofTransformValue_(DofType type, double val)
{
  osgSim::DOFTransform* dof = dynamic_cast<osgSim::DOFTransform*>(activeNode_.get());
  if (!dof)
    return;

  // Set value based on type
  switch (type)
  {
  case DofType::HEADING:
    dof->setCurrentHPR(osg::Vec3(val, dof->getCurrentHPR().y(), dof->getCurrentHPR().z()));
    break;
  case DofType::PITCH:
    dof->setCurrentHPR(osg::Vec3(dof->getCurrentHPR().x(), val, dof->getCurrentHPR().z()));
    break;
  case DofType::ROLL:
    dof->setCurrentHPR(osg::Vec3(dof->getCurrentHPR().x(), dof->getCurrentHPR().y(), val));
    break;
  case DofType::TRANSLATE_X:
    dof->setCurrentTranslate(osg::Vec3(val, dof->getCurrentTranslate().y(), dof->getCurrentTranslate().z()));
    break;
  case DofType::TRANSLATE_Y:
    dof->setCurrentTranslate(osg::Vec3(dof->getCurrentTranslate().x(), val, dof->getCurrentTranslate().z()));
    break;
  case DofType::TRANSLATE_Z:
    dof->setCurrentTranslate(osg::Vec3(dof->getCurrentTranslate().x(), dof->getCurrentTranslate().y(), val));
    break;
  case DofType::SCALE_X:
    dof->setCurrentScale(osg::Vec3(val, dof->getCurrentScale().y(), dof->getCurrentScale().z()));
    break;
  case DofType::SCALE_Y:
    dof->setCurrentScale(osg::Vec3(dof->getCurrentScale().x(), val, dof->getCurrentScale().z()));
    break;
  case DofType::SCALE_Z:
    dof->setCurrentScale(osg::Vec3(dof->getCurrentScale().x(), dof->getCurrentScale().y(), val));
    break;
  }
}

void ArticulationsEditorWidget::updateMultiSwitch_(int multiSwitchId)
{
  osgSim::MultiSwitch* ms = dynamic_cast<osgSim::MultiSwitch*>(activeNode_.get());
  if (ms)
  {
    // Update multiswitch based on switch ID.
    ms->setActiveSwitchSet(multiSwitchId);
  }
}

void ArticulationsEditorWidget::resetArticulationsInfo_()
{
  // Reset GUI back to empty state
  itemModel_->removeRows(0, itemModel_->rowCount());
  ui_->stackedWidget->setVisible(false);
  activeNode_ = nullptr;
}

}

