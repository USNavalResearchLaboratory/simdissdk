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

#include "Gui.h"
#include "ui_QThreadExample.h"

namespace SdkQThreadExample
{

Gui::Gui(QWidget* parent)
  : QDialog(parent)
{
  // Configure the GUI
  ui_ = new Ui_ThreadExample;
  ui_->setupUi(this);

  // Make GUI fixed size
  layout()->setSizeConstraint(QLayout::SetFixedSize);
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

  // Keep the business logic out of the gui code, so echo out the signals
  connect(ui_->startButton, SIGNAL(clicked(bool)), this, SIGNAL(startClicked()));
  connect(ui_->stopButton, SIGNAL(clicked(bool)), this, SIGNAL(stopClicked()));
}

Gui::~Gui()
{
  delete ui_;
}

void Gui::updateNumberProcessed(unsigned int number)
{
  ui_->numberLabel->setText(QString("%1").arg(number));
}

}

