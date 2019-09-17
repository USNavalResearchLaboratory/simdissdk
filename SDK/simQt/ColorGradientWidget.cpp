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
#include <cassert>
#include "simQt/QtConversion.h"
#include "simQt/QtFormatting.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/ColorGradientWidget.h"
#include "ui_ColorGradientWidget.h"

namespace simQt {

/** String template to format a QLinearGradient background like our UTILS::ColorGradient */
static const QString GRADIENT_STR_TEMPLATE = "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, %1);";
/** Format for each stop.  %1 is the percentage (0-1), %2 is a RGBA color string. */
static const QString GRADIENT_STOP_TEMPLATE = "stop: %1 rgba(%2)";

ColorGradientWidget::ColorGradientWidget(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_ColorGradientWidget)
{
  ui_->setupUi(this);

  // Configure tooltips
  ui_->horizontalSlider->setToolTip(simQt::formatTooltip(tr("Position"),
    tr("Position of the selected gradient stop.")));
  ui_->positionSpin->setToolTip(simQt::formatTooltip(tr("Position"),
    tr("Position of the selected gradient stop.")));
  ui_->indexCombo->setToolTip(simQt::formatTooltip(tr("Selected"),
    tr("Select a different gradient stop.<p>Gradients are defined by a number of stops.  Each stop has a position and color.  The data value is determined by interpolating colors between two stops.")));
  ui_->colorWidget->setToolTip(simQt::formatTooltip(tr("Color"),
    tr("Color for the selected gradient stop.")));
  ui_->newColorButton->setToolTip(simQt::formatTooltip(tr("New Color"),
    tr("Add a new stop to the gradient.  The new stop can be repositioned and recolored to create an alteration to the gradient.")));
  ui_->deleteColorButton->setToolTip(simQt::formatTooltip(tr("Delete Selected"),
    tr("Removes the gradient stop.  Removing the stop will change the color interpolation between its adjacent stops.")));

  connect(ui_->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(storeGradientSliderPosition_(int)));
  connect(ui_->positionSpin, SIGNAL(valueChanged(double)), this, SLOT(storeGradientSpinnerPosition_(double)));
  connect(ui_->indexCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setSelectedGradientIndex_(int)));
  connect(ui_->colorWidget, SIGNAL(colorChanged(QColor)), this, SLOT(storeGradientColor_(QColor)));
  connect(ui_->newColorButton, SIGNAL(clicked()), this, SLOT(createColor_()));
  connect(ui_->deleteColorButton, SIGNAL(clicked()), this, SLOT(deleteColor_()));

  // Configure using a default gradient
  setColorGradient(ColorGradient::newDefaultGradient());
}

ColorGradientWidget::~ColorGradientWidget()
{
  delete ui_;
}

void ColorGradientWidget::setColorGradient(const ColorGradient& gradient)
{
  // No-op if the gradients match, to prevent the stops order from resetting
  if (gradient == getColorGradient())
    return;

  // Don't call clear() here, to prevent an unnecessary change signal and graphics update
  stops_.clear();
  ui_->indexCombo->clear();

  auto colors = gradient.colors();
  for (auto gradIter = colors.begin(); gradIter != colors.end(); ++gradIter)
  {
    stops_.push_back(*gradIter);
    ui_->indexCombo->addItem(QString::number(stops_.size()));
  }
  ui_->indexCombo->setCurrentIndex(0);

  // Update new enabled state, in case it changed
  updateEnables_();
  // Update the graphics
  applyGradient_();
}

ColorGradient ColorGradientWidget::getColorGradient() const
{
  ColorGradient gradient;
  gradient.clearColors();
  for (auto stopIter = stops_.begin(); stopIter != stops_.end(); ++stopIter)
    gradient.setColor(stopIter->first, stopIter->second);

  return gradient;
}

void ColorGradientWidget::clear()
{
  stops_.clear();
  ui_->indexCombo->clear();

  updateEnables_();
  applyGradient_();
}

void ColorGradientWidget::applyGradient_()
{
  QString stopsString; // Holds the color values

  // Create black bar, matching underlying ColorGradient behavior
  if (stops_.empty())
  {
    const QString rgba = simQt::getQStringFromQColor(Qt::black);
    stopsString.append(GRADIENT_STOP_TEMPLATE.arg(0.).arg(rgba));
  }
  else
  {
    // Build the stylesheet for the gradient:
    int pos = 0;
    for (auto i = stops_.begin(); i != stops_.end(); ++i, ++pos)
    {
      if (pos != 0)
      {
        stopsString += ", ";
      }
      const QString rgba = simQt::getQStringFromQColor(i->second);
      stopsString.append(GRADIENT_STOP_TEMPLATE.arg(i->first).arg(rgba));
    }
  }

  // Set the style on the widget
  ui_->gradientWidget->setStyleSheet(GRADIENT_STR_TEMPLATE.arg(stopsString));
  emit gradientChanged(getColorGradient());
}

void ColorGradientWidget::setSelectedGradientIndex_(int index)
{
  // We can get a signal when last item is deleted from combo box, where index is -1
  if (index < 0)
    return;

  // Should not be able to provide an index outside the range of stops_
  assert(index < static_cast<int>(stops_.size()));
  if (index >= static_cast<int>(stops_.size()))
    return;
  auto stop = stops_[index];

  // Since we want to update GUI elements, but the underlying data is not changing, block signals to prevent updates
  simQt::ScopedSignalBlocker blockerA(*ui_->horizontalSlider);
  simQt::ScopedSignalBlocker blockerB(*ui_->positionSpin);

  // Slider maps [0,1] to [0,SLIDER_SIZE]
  ui_->horizontalSlider->setValue(static_cast<int>(stop.first * ui_->horizontalSlider->maximum()));
  // Spinner maps [0,1] to [0,100]
  ui_->positionSpin->setValue(stop.first * ui_->positionSpin->maximum());
  ui_->colorWidget->setColor(stop.second);
}

void ColorGradientWidget::storeGradientSliderPosition_(int sliderPos)
{
  const int index = ui_->indexCombo->currentIndex();
  // Should not be able to provide an index outside the range of stops_
  assert(index >= 0 && index < static_cast<int>(stops_.size()));
  if (index < 0 || index >= static_cast<int>(stops_.size()))
    return;
  stops_[index].first = static_cast<double>(sliderPos) / ui_->horizontalSlider->maximum();
  // Map down to [0,100] for gradient position spin
  simQt::ScopedSignalBlocker blocker(*ui_->positionSpin);
  ui_->positionSpin->setValue(stops_[index].first * ui_->positionSpin->maximum());

  applyGradient_();
}

void ColorGradientWidget::storeGradientSpinnerPosition_(double spinPos)
{
  const int index = ui_->indexCombo->currentIndex();
  // Should not be able to provide an index outside the range of stops_
  assert(index >= 0 && index < static_cast<int>(stops_.size()));
  if (index < 0 || index >= static_cast<int>(stops_.size()))
    return;
  stops_[index].first = spinPos * 0.01;
  // Map down to [0,SLIDER_SIZE] for gradient position slider
  simQt::ScopedSignalBlocker blocker(*ui_->horizontalSlider);
  ui_->horizontalSlider->setValue(stops_[index].first * ui_->horizontalSlider->maximum());

  applyGradient_();
}

void ColorGradientWidget::storeGradientColor_(const QColor& color)
{
  const int index = ui_->indexCombo->currentIndex();
  // Should not be able to provide an index outside the range of stops_
  assert(index >= 0 && index < static_cast<int>(stops_.size()));
  if (index < 0 || index >= static_cast<int>(stops_.size()))
    return;
  stops_[index].second = color;

  applyGradient_();
}

void ColorGradientWidget::createColor_()
{
  // Find the value and color of our new stop
  double percentage = 0.;
  QColor newColor = Qt::black;
  // If we've only got one stop, special case to generate new one
  if (stops_.size() == 1)
  {
    auto stopIter = stops_.begin();
    // Place our new stop symmetrically across the middle if possible
    percentage = (stopIter->first == 0.5 ? 0. : (1. - stopIter->first));
    if (percentage < stopIter->first)
      newColor = ColorGradient::interpolate(Qt::black, stopIter->second, 0., percentage, stopIter->first);
    else
      newColor = ColorGradient::interpolate(stopIter->second, Qt::black, stopIter->first, percentage, 1.);
  }
  else if (!stops_.empty())
  {
    auto lowerIter = stops_.end();
    auto higherIter = stops_.end();
    double range = 0.;
    // Find the largest gap between stops, and start our new stop between those two
    for (auto stopIter = stops_.begin(); stopIter != stops_.end(); ++stopIter)
    {
      auto nextIter = stopIter;
      ++nextIter;
      if (nextIter == stops_.end())
        break;
      if ((nextIter->first - stopIter->first) > range)
      {
        lowerIter = stopIter;
        higherIter = nextIter;
        range = (nextIter->first - stopIter->first);
      }
    }

    percentage = lowerIter->first + (range / 2.);
    newColor = ColorGradient::interpolate(lowerIter->second, higherIter->second, lowerIter->first, percentage, higherIter->first);
  }

  const unsigned int newPos = stops_.size();
  stops_.push_back(std::make_pair(percentage, newColor));

  // Set new combo text to the next number after the current last combo text
  const QString indexText = (newPos == 0 ? QString::number(1) : QString::number(ui_->indexCombo->itemText(newPos - 1).toInt() + 1));
  ui_->indexCombo->addItem(indexText);
  ui_->indexCombo->setCurrentIndex(newPos);

  // Need to fix enable/disable flag because we might go from 0 to 1 item
  if (newPos == 0)
    updateEnables_();

  applyGradient_();
}

void ColorGradientWidget::deleteColor_()
{
  const int index = ui_->indexCombo->currentIndex();
  // Should not be able to provide an index outside the range of stops_
  assert(index >= 0 && index < static_cast<int>(stops_.size()));
  if (index < 0 || index >= static_cast<int>(stops_.size()))
    return;
  stops_.erase(stops_.begin() + index);
  ui_->indexCombo->removeItem(index);
  if (stops_.empty())
    updateEnables_();

  applyGradient_();
}

void ColorGradientWidget::updateEnables_()
{
  // Update gradient editing enabled flag; note "new" button is always available
  const bool canEdit = !stops_.empty();
  ui_->indexCombo->setEnabled(canEdit);
  ui_->horizontalSlider->setEnabled(canEdit);
  ui_->positionSpin->setEnabled(canEdit);
  ui_->deleteColorButton->setEnabled(canEdit);
  ui_->colorWidget->setEnabled(canEdit);
  // Repeat the setIndex call, in case it was last called while our GUI was disabled
  if (ui_->indexCombo->count())
    setSelectedGradientIndex_(ui_->indexCombo->currentIndex());
}

}
