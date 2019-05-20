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
#include "simQt/QtFormatting.h"
#include "simQt/ColorGradientWidget.h"
#include "ui_ColorGradientWidget.h"

namespace simQt {

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
}

ColorGradientWidget::~ColorGradientWidget()
{
  delete ui_;
}

}
