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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SDK_EXAMPLES_TESTCOLORWIDGET_H
#define SDK_EXAMPLES_TESTCOLORWIDGET_H

#include <QString>
#include <QDialog>

class Ui_Dialog;

// Test code to verify that the Color Widget works.
class TestColorWidget : public QDialog
{
  Q_OBJECT

public:
  explicit TestColorWidget(QWidget *parent);
  virtual ~TestColorWidget();

protected:
  Ui_Dialog* gui_;
};

#endif /* SDK_EXAMPLES_TESTCOLORWIDGET_H */
