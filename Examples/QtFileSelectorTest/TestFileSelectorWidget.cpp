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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "TestFileSelectorWidget.h"
#include "ui_TestFileSelectorWidget.h"

// Test code to verify that the File Selector Widget works.
TestFileSelectorWidget::TestFileSelectorWidget(QWidget *parent) : QDialog(parent)
{
  gui_ = new Ui_Dialog();
  gui_->setupUi(this);
}

TestFileSelectorWidget::~TestFileSelectorWidget()
{
  delete gui_;
}

