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
#include "simQt/TimeWidget.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent) : QDialog(parent)
{
  mainWindowGui_ = new Ui_MainWindow();
  mainWindowGui_->setupUi(this);

  timeWidget_ = mainWindowGui_->timeWidget;
  timeWidget_->setTimeRange(2013, simCore::TimeStamp(2013, simCore::ZERO_SECONDS), simCore::TimeStamp(2013, simCore::Seconds(24.0*simCore::SECPERHOUR, 0)));
  timeWidget_->setTimeStamp(simCore::TimeStamp(2013, simCore::Seconds(12.0*simCore::SECPERHOUR, 0)));

  connect(timeWidget_, SIGNAL(timeEdited(const simCore::TimeStamp&)), this, SLOT(timeChanged_(const simCore::TimeStamp&)));
  connect(mainWindowGui_->precisionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setPrecision_(int)));
  connect(mainWindowGui_->testButton, SIGNAL(clicked()), this, SLOT(test_()));
}

MainWindow::~MainWindow()
{
  delete mainWindowGui_;
}

void MainWindow::test_()
{
  // Implement any desired testing here
}

void MainWindow::timeChanged_(const simCore::TimeStamp& ts)
{
}

void MainWindow::setPrecision_(int prec)
{
  timeWidget_->setPrecision(prec);
}

