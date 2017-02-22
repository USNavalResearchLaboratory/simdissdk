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
#ifndef EXAMPLES_ENTITY_LINE_EDIT_MAINWINDOW_H
#define EXAMPLES_ENTITY_LINE_EDIT_MAINWINDOW_H

#include <QDialog>

#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include "simData/CategoryData/CategoryFilter.h"

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
  void categoryFilterChanged(const simData::CategoryFilter& filter);

protected:
  std::string mmsiString_(int mmsi) const;
  simData::ObjectId addPlatform_(const std::string& name);
  void addCategoryData_(double time, const std::string& key, const std::string& value);

  simData::DataStore* dataStore_;
  Ui_MainWindow* mainWindowGui_;
  simData::ObjectId platformId_;
  bool state_;
};

#endif
