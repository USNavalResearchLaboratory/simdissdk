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
#include <sstream>
#include <iomanip>

#include "simData/CategoryData/CategoryFilter.h"
#include "simData/MemoryDataStore.h"
#include "simQt/EntityTreeModel.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/CategoryDataBreadcrumbs.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(simData::DataStore* dataStore, QWidget *parent)
  : QDialog(parent),
    dataStore_(dataStore)
{
  platformId_ = addPlatform_("Test Platform");

  simQt::ResourceInitializer::initialize();

  ui_ = new Ui_MainWindow();
  ui_->setupUi(this);

  connect(ui_->smallButton, SIGNAL(clicked()), this, SLOT(addSmallAmount_()));
  connect(ui_->massiveButton, SIGNAL(clicked()), this, SLOT(addMassiveAmount_()));
  connect(ui_->togglePushButton, SIGNAL(clicked()), this, SLOT(toggleState_()));

  // Configure the new Category Filter Widget
  ui_->categoryFilterWidget->setDataStore(dataStore);
  connect(ui_->categoryFilterWidget, SIGNAL(filterChanged(simData::CategoryFilter)),
    ui_->breadcrumbs, SLOT(setFilter(simData::CategoryFilter)));
  connect(ui_->breadcrumbs, SIGNAL(filterEdited(simData::CategoryFilter)),
    ui_->categoryFilterWidget, SLOT(setFilter(simData::CategoryFilter)));
  connect(ui_->categoryFilterWidget, SIGNAL(filterChanged(simData::CategoryFilter)),
    this, SLOT(categoryFilterChanged_(simData::CategoryFilter)));
  connect(ui_->breadcrumbs, SIGNAL(filterEdited(simData::CategoryFilter)),
    this, SLOT(categoryFilterChanged_(simData::CategoryFilter)));
}

MainWindow::~MainWindow()
{
  delete ui_;
}

simData::ObjectId MainWindow::addPlatform_(const std::string& name)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformProperties* props = dataStore_->addPlatform(&xaction);
  simData::ObjectId id = props->id();
  props->set_originalid(id);
  xaction.complete(&props);

  simData::DataStore::Transaction transaction;
  simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(id, &transaction);
  assert(prefs);
  prefs->mutable_commonprefs()->set_name(name);
  transaction.complete(&prefs);

  return id;
}

void MainWindow::addSmallAmount_()
{
  addCategoryData_(0.0, "Type", "Platform");
  addCategoryData_(0.0, "Type", "Beam");
  addCategoryData_(0.0, "Type", "Gate");
  addCategoryData_(0.0, "Type", "Laser");
  addCategoryData_(0.0, "Type", "LOB");

  addCategoryData_(0.0, "Affinity", "Friend");
  addCategoryData_(0.0, "Affinity", "Hostile");
  addCategoryData_(0.0, "Affinity", "Unknown");
}

void MainWindow::addMassiveAmount_()
{
// Test with smaller number when in DEBUG
#ifdef _DEBUG
  int size = 100;
#else
  int size = 20000;
#endif

  for (int ii = 0; ii < size; ++ii)
    addCategoryData_(0.0, "MMSI", mmsiString_(ii));
}

void MainWindow::toggleState_()
{
  simData::CategoryFilter* filter = new simData::CategoryFilter(dataStore_, true);
  filter->updateAll(state_);
  ui_->categoryFilterWidget->setFilter(*filter);
  delete filter;
  state_ = !state_;
}

void MainWindow::categoryFilterChanged_(const simData::CategoryFilter& filter)
{
}

std::string MainWindow::mmsiString_(int mmsi) const
{
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(9) << mmsi;
  return ss.str();
}

void MainWindow::addCategoryData_(double time, const std::string& key, const std::string& value)
{
  simData::DataStore::Transaction xaction;
  simData::CategoryData* cat = dataStore_->addCategoryData(platformId_, &xaction);
  cat->set_time(time);
  simData::CategoryData_Entry* entry = cat->add_entry();
  entry->set_key(key);
  entry->set_value(value);
  xaction.complete(&cat);
}

//////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  simData::MemoryDataStore* dataStore = new simData::MemoryDataStore();
  MainWindow* window = new MainWindow(dataStore, nullptr);
  window->show();

  int rv = app.exec();
  delete window;
  delete dataStore;
  return rv;
}
