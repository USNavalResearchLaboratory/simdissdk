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
#ifndef TIMEWIDGETTEST_MAINWINDOW_H
#define TIMEWIDGETTEST_MAINWINDOW_H

#include <QString>
#include <QDialog>

namespace simQt { class TimeWidget; }
namespace simCore { class TimeStamp; }

class Ui_MainWindow;


class MainWindow : public QDialog
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent);
  virtual ~MainWindow();

private Q_SLOTS:
  void test_();
  void timeChanged_(const simCore::TimeStamp& ts);
  void setPrecision_(int prec);

private:
  Ui_MainWindow* mainWindowGui_;
  simQt::TimeWidget* timeWidget_;

};

#endif /* TIMEWIDGETTEST_MAINWINDOW_H */
