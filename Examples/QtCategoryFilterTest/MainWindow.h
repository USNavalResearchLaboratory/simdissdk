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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef EXAMPLES_CATEGORYFILTERTEST_MAINWINDOW_H
#define EXAMPLES_CATEGORYFILTERTEST_MAINWINDOW_H

#include <QDialog>
#include "simCore/Common/Common.h"
#include "simData/ObjectId.h"

namespace simData {
  class DataStore;
  class CategoryFilter;
}
class Ui_MainWindow;

class MainWindow : public QDialog
{
  Q_OBJECT

public:
  MainWindow(simData::DataStore* dataStore, QWidget *parent);
  virtual ~MainWindow();

private slots:
  void addSmallAmount_();
  void addMassiveAmount_();
  void toggleState_();
  void categoryFilterChanged_(const simData::CategoryFilter& filter);

protected:
  std::string mmsiString_(int mmsi) const;
  simData::ObjectId addPlatform_(const std::string& name);
  void addCategoryData_(double time, const std::string& key, const std::string& value);

  simData::DataStore* dataStore_;
  Ui_MainWindow* ui_;
  simData::ObjectId platformId_;
  bool state_;
};

#endif /* EXAMPLES_CATEGORYFILTERTEST_MAINWINDOW_H */
