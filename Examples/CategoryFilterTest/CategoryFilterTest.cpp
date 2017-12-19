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
#include <sstream>
#include <iomanip>

#include "simData/MemoryDataStore.h"
#include "simQt/EntityTreeModel.h"
#include "simQt/ResourceInitializer.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(simData::DataStore* dataStore, QWidget *parent)
  : QDialog(parent),
    dataStore_(dataStore)
{
  platformId_ = addPlatform_("Test Platform");

  simQt::ResourceInitializer::initialize();

  mainWindowGui_ = new Ui_MainWindow();
  mainWindowGui_->setupUi(this);

  mainWindowGui_->categoryFilterWidget->setProviders(dataStore);
  connect(mainWindowGui_->smallButton, SIGNAL(clicked()), this, SLOT(addSmallAmount_()));
  connect(mainWindowGui_->massiveButton, SIGNAL(clicked()), this, SLOT(addMassiveAmount_()));
  connect(mainWindowGui_->togglePushButton, SIGNAL(clicked()), this, SLOT(toggleState_()));
  connect(mainWindowGui_->categoryFilterWidget, SIGNAL(categoryFilterChanged(const simData::CategoryFilter&)), this, SLOT(categoryFilterChanged(const simData::CategoryFilter&)));
}

MainWindow::~MainWindow()
{
  delete mainWindowGui_;
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
  simData::CategoryFilter*filter = new simData::CategoryFilter(dataStore_, true);
  filter->updateAll(state_);
  mainWindowGui_->categoryFilterWidget->setFilter(*filter);
  delete filter;
  state_ = !state_;
}

void MainWindow::categoryFilterChanged(const simData::CategoryFilter& filter)
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
  MainWindow* window = new MainWindow(dataStore, NULL);
  window->show();

  int rv = app.exec();
  delete window;
  delete dataStore;
  return rv;
}

