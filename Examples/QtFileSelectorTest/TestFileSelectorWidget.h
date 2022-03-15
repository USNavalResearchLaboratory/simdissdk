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
#ifndef SDKEXAMPLES_TESTFILESELECTORWIDGET_H
#define SDKEXAMPLES_TESTFILESELECTORWIDGET_H

#include <QString>
#include <QDialog>

class Ui_Dialog;

// Test code to verify that the File Selector Widget works.
class TestFileSelectorWidget : public QDialog
{
  Q_OBJECT

public:
  explicit TestFileSelectorWidget(QWidget *parent);
  virtual ~TestFileSelectorWidget();

protected:
  Ui_Dialog* gui_;
};

#endif /* SDKEXAMPLES_TESTFILESELECTORWIDGET_H */
